#include "camera_protocol.h"

#include "config.h"

#if CAMERA_PROTOCOL_BACKEND == 1
#include <BLE2902.h>
#include <BLEAdvertising.h>
#include <BLECharacteristic.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEService.h>
#include <BLEUtils.h>
#endif

#if CAMERA_PROTOCOL_BACKEND == 1
namespace
{
  BLEServer *g_bleServer = nullptr;
  BLEService *g_bleService = nullptr;
  BLEService *g_bleSecondaryService = nullptr;
  BLECharacteristic *g_bleWriteCharacteristic = nullptr;
  BLECharacteristic *g_bleNotifyCharacteristic = nullptr;
  BLECharacteristic *g_bleInfoCharacteristic = nullptr;

  bool g_bleDeviceConnected = false;
  bool g_bleAckReceived = false;
  uint8_t g_bleAckOpcode = 0;
  uint8_t g_bleExpectedAckOpcode = 0;
  bool g_bleWaitingAck = false;
  unsigned long g_bleLastRxMs = 0;
  SystemModel *g_model = nullptr;
  uint8_t g_lastFrame02[32] = {0};
  size_t g_lastFrame02Len = 0;
  uint8_t g_lastFrame10[32] = {0};
  size_t g_lastFrame10Len = 0;

  const uint8_t SHUTTER_CMD[] = {0xFC, 0xEF, 0xFE, 0x86, 0x00, 0x03, 0x01, 0x02, 0x00};
  const uint8_t MODE_CMD[] = {0xFC, 0xEF, 0xFE, 0x86, 0x00, 0x03, 0x01, 0x01, 0x00};
  const uint8_t TOGGLE_SCREEN_CMD[] = {0xFC, 0xEF, 0xFE, 0x86, 0x00, 0x03, 0x01, 0x00, 0x00};
  const uint8_t POWER_OFF_CMD[] = {0xFC, 0xEF, 0xFE, 0x86, 0x00, 0x03, 0x01, 0x00, 0x03};

  static const char *frameTypeLabel(uint8_t type)
  {
    switch (type)
    {
    case 0x02:
      return "ack/status";
    case 0x10:
      return "telemetry/ascii";
    default:
      return "unknown";
    }
  }

  static bool isAckMatch(uint8_t expectedOpcode, uint8_t receivedOpcode)
  {
    if (!CAMERA_BLE_ACK_STRICT)
    {
      return true;
    }
    return receivedOpcode == expectedOpcode || receivedOpcode == static_cast<uint8_t>(expectedOpcode | 0x80);
  }

  static void logHexPrefix(const char *prefix, const uint8_t *data, size_t length)
  {
    if (!CAMERA_BLE_LOG_NOTIFY_HEX)
    {
      return;
    }
    Serial.print(prefix);
    Serial.print(" len=");
    Serial.print(length);
    Serial.print(" data=");
    for (size_t i = 0; i < length; i++)
    {
      if (data[i] < 0x10)
      {
        Serial.print('0');
      }
      Serial.print(data[i], HEX);
      if (i + 1 < length)
      {
        Serial.print(' ');
      }
    }
    Serial.println();
  }

  static bool isInstaFrame(const uint8_t *data, size_t length)
  {
    return length >= 5 && data[0] == 0xFE && data[1] == 0xEF && data[2] == 0xFE;
  }

  static uint8_t frameType(const uint8_t *data)
  {
    return data[3];
  }

  static bool isAsciiMetadataByte(uint8_t value)
  {
    return (value >= 32 && value <= 126) || value == '\t';
  }

  static String sanitizeAsciiMetadata(const uint8_t *data, size_t length)
  {
    String bestRun;
    String currentRun;

    for (size_t i = 0; i < length; i++)
    {
      const uint8_t value = data[i];
      if (isAsciiMetadataByte(value))
      {
        currentRun += (value == '\t') ? ' ' : static_cast<char>(value);
      }
      else
      {
        if (currentRun.length() > bestRun.length())
        {
          bestRun = currentRun;
        }
        currentRun = "";
      }
    }

    if (currentRun.length() > bestRun.length())
    {
      bestRun = currentRun;
    }

    bestRun.trim();
    while (bestRun.indexOf("  ") >= 0)
    {
      bestRun.replace("  ", " ");
    }

    if (bestRun.length() > 22)
    {
      bestRun.remove(22);
      bestRun.trim();
    }

    return bestRun;
  }

  static bool parseDurationToken(const char *text, unsigned long &secondsOut);

  static bool inferModeFromText(const String &text, CameraMode &modeOut)
  {
    String upper = text;
    upper.toUpperCase();

    if (upper.indexOf("K|") >= 0 || upper.indexOf("FPS") >= 0)
    {
      modeOut = CameraMode::VIDEO;
      return true;
    }

    if (upper.endsWith("+"))
    {
      modeOut = CameraMode::PHOTO;
      return true;
    }

    if (upper.indexOf("TIME") >= 0 || upper.indexOf("LAPSE") >= 0 ||
        upper.indexOf("SHIFT") >= 0 || upper.indexOf("STAR") >= 0)
    {
      modeOut = CameraMode::TIMELAPSE;
      return true;
    }

    if (upper.indexOf("PHOTO") >= 0 || upper.indexOf("BURST") >= 0 ||
        upper.indexOf("INTERVAL") >= 0)
    {
      modeOut = CameraMode::PHOTO;
      return true;
    }

    if (upper.indexOf("VIDEO") >= 0 || upper.indexOf("LOOP") >= 0 ||
        upper.indexOf("REC") >= 0)
    {
      modeOut = CameraMode::VIDEO;
      return true;
    }

    return false;
  }

  static bool parseDurationString(const String &text, unsigned long &secondsOut)
  {
    if (text.length() == 0)
    {
      return false;
    }

    char buffer[24] = {0};
    const size_t copyLen = text.length() < (sizeof(buffer) - 1) ? text.length() : (sizeof(buffer) - 1);
    memcpy(buffer, text.c_str(), copyLen);
    buffer[copyLen] = '\0';
    return parseDurationToken(buffer, secondsOut);
  }

  static bool isDigitsOnly(const String &value)
  {
    if (value.length() == 0)
    {
      return false;
    }

    for (size_t i = 0; i < value.length(); i++)
    {
      const char ch = value[i];
      if (ch < '0' || ch > '9')
      {
        return false;
      }
    }

    return true;
  }

  static bool parseClockHms(const String &text, unsigned long &secondsOut)
  {
    int firstColon = text.indexOf(':');
    if (firstColon <= 0)
    {
      return false;
    }

    int secondColon = text.indexOf(':', firstColon + 1);
    if (secondColon <= (firstColon + 1))
    {
      return false;
    }

    const int start = firstColon >= 2 ? (firstColon - 2) : 0;
    String hPart = text.substring(start, firstColon);
    String mPart = text.substring(firstColon + 1, secondColon);
    String sPart = text.substring(secondColon + 1);

    hPart.trim();
    mPart.trim();
    sPart.trim();

    if (!isDigitsOnly(hPart) || !isDigitsOnly(mPart) || !isDigitsOnly(sPart))
    {
      return false;
    }

    const unsigned long h = static_cast<unsigned long>(hPart.toInt());
    const unsigned long m = static_cast<unsigned long>(mPart.toInt());
    const unsigned long s = static_cast<unsigned long>(sPart.toInt());
    secondsOut = h * 3600UL + m * 60UL + s;
    return true;
  }

  static void updateTelemetryFromMetadataText(const String &text)
  {
    if (!g_model)
    {
      return;
    }

    String trimmed = text;
    trimmed.trim();
    if (trimmed.length() < 2)
    {
      return;
    }

    g_model->telemetrySupported = true;

    unsigned long recordingSec = 0;
    if (parseClockHms(trimmed, recordingSec))
    {
      g_model->cameraMode = CameraMode::VIDEO;
      g_model->isRecording = true;
      return;
    }

    const int lastSeparator = trimmed.lastIndexOf('|');
    String tailToken = lastSeparator >= 0 ? trimmed.substring(lastSeparator + 1) : trimmed;
    tailToken.trim();

    unsigned long durationSec = 0;
    if (parseDurationString(tailToken, durationSec))
    {
      g_model->telemetry.videoRemainingAvailable = true;
      g_model->telemetry.videoRemainingSec = durationSec;
      g_model->cameraMode = CameraMode::VIDEO;
      g_model->isRecording = false;
      return;
    }

    if (trimmed.endsWith("+"))
    {
      String countPart = trimmed;
      countPart.remove(countPart.length() - 1);
      countPart.trim();
      if (isDigitsOnly(countPart))
      {
        g_model->telemetry.photosRemainingAvailable = true;
        g_model->telemetry.photosRemaining = countPart.toInt();
        g_model->cameraMode = CameraMode::PHOTO;
        g_model->isRecording = false;
      }
    }
  }

  static void updateModeMetadataFromText(const char *sourceLabel, const String &text)
  {
    if (!g_model || text.length() < 4)
    {
      return;
    }

    g_model->cameraModeLabel = text;

    CameraMode parsedMode = CameraMode::UNKNOWN;
    if (inferModeFromText(text, parsedMode))
    {
      g_model->cameraMode = parsedMode;
    }

    updateTelemetryFromMetadataText(text);

    if (CAMERA_BLE_LOG_GATT_ACTIVITY)
    {
      Serial.print("[BLE] ");
      Serial.print(sourceLabel ? sourceLabel : "META");
      Serial.print(" text=");
      Serial.println(text);
    }
  }

  static bool parseDurationToken(const char *text, unsigned long &secondsOut)
  {
    unsigned long totalSeconds = 0;
    unsigned long currentValue = 0;
    bool sawUnit = false;

    for (size_t i = 0; text[i] != '\0'; i++)
    {
      const char ch = text[i];
      if (ch >= '0' && ch <= '9')
      {
        currentValue = currentValue * 10 + static_cast<unsigned long>(ch - '0');
        continue;
      }

      if (ch == 'h')
      {
        totalSeconds += currentValue * 3600UL;
        currentValue = 0;
        sawUnit = true;
        continue;
      }

      if (ch == 'm')
      {
        totalSeconds += currentValue * 60UL;
        currentValue = 0;
        sawUnit = true;
        continue;
      }

      if (ch == 's')
      {
        totalSeconds += currentValue;
        currentValue = 0;
        sawUnit = true;
        continue;
      }

      return false;
    }

    if (!sawUnit)
    {
      return false;
    }

    secondsOut = totalSeconds;
    return true;
  }

  static bool frameChanged(const uint8_t *data, size_t length, uint8_t *lastFrame, size_t &lastLen)
  {
    if (lastLen != length)
    {
      memcpy(lastFrame, data, length);
      lastLen = length;
      return true;
    }

    if (memcmp(lastFrame, data, length) != 0)
    {
      memcpy(lastFrame, data, length);
      return true;
    }

    return false;
  }

  static void logBatteryCandidates(const uint8_t *data, size_t length, uint8_t type)
  {
    if (!CAMERA_BLE_LOG_BATTERY_CANDIDATES)
    {
      return;
    }

    // Rastreamento de mudanças por tipo de frame
    uint8_t *lastFrame = nullptr;
    size_t *lastLen = nullptr;
    if (type == 0x02)
    {
      lastFrame = g_lastFrame02;
      lastLen = &g_lastFrame02Len;
    }
    else if (type == 0x10)
    {
      lastFrame = g_lastFrame10;
      lastLen = &g_lastFrame10Len;
    }
    else
    {
      // Para frames secundários (0x05, 0x07, 0x0F, FFD1, etc.), sempre loga
      // pois queremos descobrir onde está a bateria
    }

    if (lastFrame && !frameChanged(data, length, lastFrame, *lastLen))
    {
      return;
    }

    Serial.print("[BLE] frame 0x");
    if (type < 0x10)
    {
      Serial.print('0');
    }
    Serial.print(type, HEX);
    Serial.print(" candidates");

    for (size_t i = 4; i < length; i++)
    {
      Serial.print(" b");
      Serial.print(i);
      Serial.print('=');
      Serial.print(static_cast<int>(data[i]));
    }
    Serial.println();
  }

  static void logFrameSummary(const char *sourceLabel, const uint8_t *data, size_t length, uint8_t type)
  {
    if (!CAMERA_BLE_LOG_FRAME_SUMMARY)
    {
      return;
    }

    Serial.print("[BLE] ");
    Serial.print(sourceLabel ? sourceLabel : "RX");
    Serial.print(" frame=0x");
    if (type < 0x10)
    {
      Serial.print('0');
    }
    Serial.print(type, HEX);
    Serial.print(" kind=");
    Serial.print(frameTypeLabel(type));
    Serial.print(" len=");
    Serial.print(length);

    if (type == 0x10 && length > 10)
    {
      Serial.print(" tail=");
      for (size_t i = 10; i < length; i++)
      {
        const char ch = static_cast<char>(data[i]);
        if (ch >= 32 && ch <= 126)
        {
          Serial.print(ch);
        }
        else
        {
          Serial.print('.');
        }
      }
    }

    Serial.println();
  }

  static void parseIncomingFrame(const char *sourceLabel, const uint8_t *data, size_t length)
  {
    if (!isInstaFrame(data, length))
    {
      return;
    }

    const uint8_t type = frameType(data);
    g_bleAckOpcode = type;
    g_bleLastRxMs = millis();
    logFrameSummary(sourceLabel, data, length, type);
    logBatteryCandidates(data, length, type);

    if (!g_model)
    {
      return;
    }

    if (type == 0x10 && length >= 15)
    {
      char timeText[8] = {0};
      size_t textLen = length - 10;
      if (textLen > 7)
      {
        textLen = 7;
      }

      bool asciiOk = true;
      for (size_t i = 0; i < textLen; i++)
      {
        const char ch = static_cast<char>(data[10 + i]);
        if (!((ch >= '0' && ch <= '9') || ch == 'h' || ch == 'm' || ch == 's'))
        {
          asciiOk = false;
          break;
        }
        timeText[i] = ch;
      }

      if (asciiOk)
      {
        unsigned long seconds = 0;
        if (parseDurationToken(timeText, seconds))
        {
          g_model->telemetrySupported = true;
          g_model->telemetry.videoRemainingAvailable = true;
          g_model->telemetry.videoRemainingSec = seconds;
        }
      }
    }

    // Frame 0x02 (ack/status) contém dados de estado incluindo bateria (b9).
    // Formula experimental: battery% = (b9 * SCALE) + OFFSET
    // Baseado em correlação de b9 variando de ~21 (100%) para ~8 (37%)
    if (CAMERA_BLE_ENABLE_EXPERIMENTAL_BATTERY && type == 0x02 &&
        length > CAMERA_BLE_BATTERY_BYTE_INDEX)
    {
      int battery = (static_cast<int>(data[CAMERA_BLE_BATTERY_BYTE_INDEX]) *
                     CAMERA_BLE_BATTERY_SCALE_FACTOR) +
                    CAMERA_BLE_BATTERY_OFFSET_PERCENT;
      if (battery < 0)
      {
        battery = 0;
      }
      if (battery > 100)
      {
        battery = 100;
      }
      if (battery >= 0 && battery <= 100)
      {
        g_model->telemetrySupported = true;
        g_model->telemetry.batteryAvailable = true;
        g_model->telemetry.batteryPercent = battery;

        if (CAMERA_BLE_LOG_BATTERY_CANDIDATES)
        {
          Serial.print("[BLE] Battery estimate: b");
          Serial.print(CAMERA_BLE_BATTERY_BYTE_INDEX);
          Serial.print("=");
          Serial.print(data[CAMERA_BLE_BATTERY_BYTE_INDEX]);
          Serial.print(" → ");
          Serial.print(battery);
          Serial.println("%");
        }
      }
    }
  }

  class RemoteServerCallbacks : public BLEServerCallbacks
  {
    void onConnect(BLEServer *) override
    {
      g_bleDeviceConnected = true;
      g_bleLastRxMs = millis();
      if (CAMERA_BLE_LOG_CONNECTION)
      {
        Serial.println("[BLE] Camera conectada no remoto.");
      }
    }

    void onDisconnect(BLEServer *server) override
    {
      g_bleDeviceConnected = false;
      g_bleWaitingAck = false;
      g_bleLastRxMs = 0;

      if (CAMERA_BLE_LOG_CONNECTION)
      {
        Serial.println("[BLE] Camera desconectada. Reiniciando advertising.");
      }

      BLEAdvertising *adv = BLEDevice::getAdvertising();
      if (adv)
      {
        adv->start();
      }
      else if (server)
      {
        server->startAdvertising();
      }
    }
  };

  class RemoteGattWriteCallbacks : public BLECharacteristicCallbacks
  {
  public:
    RemoteGattWriteCallbacks(const char *label, bool parseFrames, bool ackEligible, bool inspectMetadata)
        : label_(label), parseFrames_(parseFrames), ackEligible_(ackEligible), inspectMetadata_(inspectMetadata)
    {
    }

  private:
    void onWrite(BLECharacteristic *characteristic) override
    {
      String rxValue = characteristic->getValue();
      if (rxValue.length() == 0)
      {
        return;
      }

      const uint8_t *data = reinterpret_cast<const uint8_t *>(rxValue.c_str());
      const size_t length = static_cast<size_t>(rxValue.length());
      if (CAMERA_BLE_LOG_GATT_ACTIVITY)
      {
        String prefix = String("[BLE] ") + label_;
        logHexPrefix(prefix.c_str(), data, length);
      }

      if (inspectMetadata_)
      {
        const String metadataText = sanitizeAsciiMetadata(data, length);
        if (metadataText.length() >= 4)
        {
          updateModeMetadataFromText(label_, metadataText);
        }
      }

      if (parseFrames_)
      {
        parseIncomingFrame(label_, data, length);
      }

      if (!ackEligible_)
      {
        return;
      }

      const uint8_t rxType = isInstaFrame(data, length) ? frameType(data) : data[0];
      g_bleAckOpcode = rxType;
      if (!g_bleWaitingAck)
      {
        return;
      }

      if (isAckMatch(g_bleExpectedAckOpcode, rxType))
      {
        g_bleAckReceived = true;
        g_bleWaitingAck = false;
      }
    }

    const char *label_;
    bool parseFrames_;
    bool ackEligible_;
    bool inspectMetadata_;
  };

  static void startNormalAdvertising(const char *remoteIdentifier)
  {
    BLEAdvertising *adv = BLEDevice::getAdvertising();
    if (!adv)
    {
      return;
    }

    BLEDevice::stopAdvertising();
    delay(80);

    String deviceName = String(CAMERA_BLE_DEVICE_NAME_PREFIX) + remoteIdentifier;
    adv->addServiceUUID(BLEUUID(CAMERA_BLE_SERVICE_UUID));
    adv->addServiceUUID(BLEUUID(CAMERA_BLE_SECONDARY_SERVICE_UUID));
    adv->setScanResponse(true);
    adv->setMinPreferred(0x06);
    adv->setMinPreferred(0x12);
    adv->start();

    if (CAMERA_BLE_LOG_CONNECTION)
    {
      Serial.print("[BLE] Advertising: ");
      Serial.println(deviceName);
    }
  }
} // namespace
#endif

#if CAMERA_PROTOCOL_BACKEND != 1
static bool g_bleDeviceConnected = false;
#endif

void CameraProtocol::begin(SystemModel *model, const char *remoteIdentifier)
{
  model_ = model;
  remoteIdentifier_ = remoteIdentifier;

#if CAMERA_PROTOCOL_BACKEND == 1
  g_model = model_;
#endif

#if CAMERA_PROTOCOL_BACKEND == 1
  backend_ = Backend::BLE_STUB;
#else
  backend_ = Backend::SIMULATED;
#endif

  if (!model_)
  {
    return;
  }

  model_->connectionState = ConnectionState::DISCONNECTED;
  model_->cameraMode = backend_ == Backend::SIMULATED ? CameraMode::VIDEO : CameraMode::UNKNOWN;
  model_->cameraModeLabel = backend_ == Backend::SIMULATED ? "VIDEO" : "";
  model_->feedback = "Pronto para conectar";

  // No MVP, a telemetria inicia desabilitada e e ativada somente apos conexao.
  model_->telemetrySupported = false;
  model_->telemetry = Telemetry();

  if (backend_ == Backend::BLE_STUB)
  {
    model_->feedback = "Pronto para conectar";
  }
}

void CameraProtocol::tick()
{
  if (!model_)
  {
    return;
  }

  if (connectInProgress_)
  {
    if (backend_ == Backend::SIMULATED)
    {
      const unsigned long elapsed = millis() - connectStartMs_;
      if (elapsed < 700)
      {
        model_->connectionState = ConnectionState::SCANNING;
        model_->feedback = "Buscando camera";
      }
      else if (elapsed < 1500)
      {
        model_->connectionState = ConnectionState::PAIRING;
        model_->feedback = "Pareando";
      }
      else
      {
        connectInProgress_ = false;
        model_->connectionState = ConnectionState::CONNECTED;
        model_->cameraMode = CameraMode::VIDEO;
        model_->cameraModeLabel = "VIDEO";
        model_->isSleeping = false;
        model_->feedback = "Conectado";

        // Em modo simulado, assumimos que os dados de telemetria estao acessiveis.
        model_->telemetrySupported = true;
        model_->telemetry.batteryAvailable = true;
        model_->telemetry.batteryPercent = 78;
        model_->telemetry.videoRemainingAvailable = true;
        model_->telemetry.videoRemainingSec = 3600;
        model_->telemetry.photosRemainingAvailable = true;
        model_->telemetry.photosRemaining = 120;
      }
    }
    else
    {
      const bool bleReady = bleEnsureConnected();
      if (!bleReady)
      {
        connectInProgress_ = false;
        model_->connectionState = ConnectionState::ERROR;
        model_->feedback = "Falha na conexao";
      }
      else if (g_bleDeviceConnected)
      {
        connectInProgress_ = false;
        model_->connectionState = ConnectionState::CONNECTED;
        model_->cameraMode = CameraMode::UNKNOWN;
        model_->cameraModeLabel = "";
        model_->isSleeping = false;
        model_->telemetrySupported = false;
        model_->telemetry = Telemetry();
        model_->feedback = "Conectado";
      }
      else
      {
        const unsigned long elapsed = millis() - connectStartMs_;
        if (elapsed < 1200)
        {
          model_->connectionState = ConnectionState::SCANNING;
          model_->feedback = "Buscando camera";
        }
        else if (elapsed < CAMERA_BLE_PAIR_TIMEOUT_MS)
        {
          model_->connectionState = ConnectionState::PAIRING;
          model_->feedback = "Aguardando pareamento";
        }
        else
        {
          connectInProgress_ = false;
          model_->connectionState = ConnectionState::ERROR;
          model_->feedback = "Tempo de pareamento esgotado";
        }
      }
    }
  }

#if CAMERA_PROTOCOL_BACKEND == 1
  if (backend_ == Backend::BLE_STUB && g_bleDeviceConnected &&
      model_->connectionState != ConnectionState::CONNECTED)
  {
    model_->connectionState = ConnectionState::CONNECTED;
  }
  else if (backend_ == Backend::BLE_STUB && !g_bleDeviceConnected &&
           model_->connectionState == ConnectionState::CONNECTED)
  {
    model_->connectionState = ConnectionState::DISCONNECTED;
  }

  if (backend_ == Backend::BLE_STUB && g_bleDeviceConnected && g_bleLastRxMs > 0)
  {
    if ((millis() - g_bleLastRxMs) > 6000)
    {
      model_->connectionState = ConnectionState::ERROR;
      model_->feedback = "Sem resposta da camera";
    }
  }
#endif

  updateMockTelemetry();
}

bool CameraProtocol::execute(MenuAction action)
{
  if (!model_)
  {
    return false;
  }

  switch (action)
  {
  case MenuAction::CONNECT:
    if (model_->connectionState == ConnectionState::CONNECTED)
    {
      model_->feedback = "Camera ja conectada";
      return true;
    }
    startConnectFlow();
    return true;

  case MenuAction::RECORD_TOGGLE:
    if (model_->connectionState != ConnectionState::CONNECTED || model_->isSleeping)
    {
      model_->feedback = "Conecte a camera";
      return false;
    }
    return dispatchCommand(CommandId::RECORD_TOGGLE);

  case MenuAction::COUNT:
    return false;
  }

  return false;
}

void CameraProtocol::startConnectFlow()
{
  if (!model_)
  {
    return;
  }
  connectInProgress_ = true;
  connectStartMs_ = millis();
  model_->connectionState = ConnectionState::SCANNING;
  model_->feedback = "Conectando";

  if (backend_ == Backend::BLE_STUB)
  {
    bleEnsureConnected();
  }
}

void CameraProtocol::updateMockTelemetry()
{
  if (!model_ || backend_ != Backend::SIMULATED)
  {
    return;
  }
  if (model_->connectionState != ConnectionState::CONNECTED || model_->isSleeping)
  {
    return;
  }

  const unsigned long now = millis();
  if ((now - lastTelemetryTickMs_) < 1000)
  {
    return;
  }
  lastTelemetryTickMs_ = now;

  if (model_->telemetry.batteryAvailable && model_->telemetry.batteryPercent > 0)
  {
    model_->telemetry.batteryPercent -= 1;
  }

  if (model_->isRecording && model_->telemetry.videoRemainingAvailable && model_->telemetry.videoRemainingSec > 0)
  {
    model_->telemetry.videoRemainingSec -= 1;
  }
}

bool CameraProtocol::dispatchCommand(CommandId command)
{
  if (!model_)
  {
    return false;
  }

  if (backend_ == Backend::BLE_STUB)
  {
    return dispatchBleStub(command);
  }
  return dispatchSimulated(command);
}

bool CameraProtocol::dispatchSimulated(CommandId command)
{
  if (!model_)
  {
    return false;
  }

  switch (command)
  {
  case CommandId::RECORD_TOGGLE:
    model_->isRecording = !model_->isRecording;
    model_->feedback = model_->isRecording ? "Gravacao iniciada" : "Gravacao parada";
    return true;
  }

  return false;
}

bool CameraProtocol::dispatchBleStub(CommandId command)
{
#if CAMERA_PROTOCOL_BACKEND != 1
  (void)command;
  model_->feedback = "Conexao BLE indisponivel";
  return false;
#else
  if (!bleEnsureConnected())
  {
    model_->feedback = "Conexao BLE indisponivel";
    return false;
  }
  if (!g_bleDeviceConnected)
  {
    model_->feedback = "Conecte a camera primeiro";
    return false;
  }

  const uint8_t *payload = nullptr;
  size_t payloadLen = 0;
  uint8_t ackOpcode = 0x02;
  const char *label = "CMD";

  switch (command)
  {
  case CommandId::RECORD_TOGGLE:
    payload = SHUTTER_CMD;
    payloadLen = sizeof(SHUTTER_CMD);
    label = "SHUTTER";
    break;
  }

  model_->connectionState = ConnectionState::PAIRING;
  const bool ack = sendBleCommandWithAck(payload, payloadLen, ackOpcode, label);
  model_->connectionState = ConnectionState::CONNECTED;
  if (!ack)
  {
    model_->feedback = "Falha ao enviar comando";
    return false;
  }

  if (command == CommandId::RECORD_TOGGLE)
  {
    model_->isRecording = !model_->isRecording;
  }

  model_->feedback = model_->isRecording ? "Gravacao iniciada" : "Gravacao parada";
  return true;
#endif
}

bool CameraProtocol::sendBleCommandWithAck(const uint8_t *payload, size_t payloadLen, uint8_t ackOpcode, const char *label)
{
#if CAMERA_PROTOCOL_BACKEND != 1
  (void)payload;
  (void)payloadLen;
  (void)ackOpcode;
  (void)label;
  return false;
#else
  if (!g_bleNotifyCharacteristic || !payload || payloadLen == 0)
  {
    return false;
  }

  logHexPrefix("[BLE] TX", payload, payloadLen);
  Serial.print("[BLE] CMD ");
  Serial.println(label);

  g_bleAckReceived = false;
  g_bleAckOpcode = 0;
  g_bleExpectedAckOpcode = ackOpcode;
  g_bleWaitingAck = CAMERA_BLE_REQUIRE_ACK;

  g_bleNotifyCharacteristic->setValue(const_cast<uint8_t *>(payload), payloadLen);
  g_bleNotifyCharacteristic->notify();

  if (!CAMERA_BLE_REQUIRE_ACK)
  {
    return true;
  }

  const unsigned long start = millis();
  while ((millis() - start) < CAMERA_BLE_ACK_TIMEOUT_MS)
  {
    if (g_bleAckReceived)
    {
      return true;
    }
    delay(5);
  }

  g_bleWaitingAck = false;
  Serial.print("[BLE] ACK timeout lastOpcode=0x");
  Serial.println(g_bleAckOpcode, HEX);
  return false;
#endif
}

bool CameraProtocol::bleHasConfiguredUuids() const
{
  return String(CAMERA_BLE_SERVICE_UUID).length() > 0 &&
         String(CAMERA_BLE_COMMAND_CHAR_UUID).length() > 0 &&
         String(CAMERA_BLE_NOTIFY_CHAR_UUID).length() > 0 &&
         String(CAMERA_BLE_INFO_CHAR_UUID).length() > 0 &&
         String(CAMERA_BLE_SECONDARY_SERVICE_UUID).length() > 0;
}

void CameraProtocol::bleDisconnect()
{
#if CAMERA_PROTOCOL_BACKEND == 1
  g_bleDeviceConnected = false;
  g_bleWaitingAck = false;
#endif
}

bool CameraProtocol::bleDiscoverCharacteristics()
{
#if CAMERA_PROTOCOL_BACKEND != 1
  return false;
#else
  return g_bleNotifyCharacteristic != nullptr && g_bleWriteCharacteristic != nullptr;
#endif
}

bool CameraProtocol::bleEnsureConnected()
{
#if CAMERA_PROTOCOL_BACKEND != 1
  return false;
#else
  if (!bleHasConfiguredUuids())
  {
    return false;
  }

  if (bleInitialized_)
  {
    return true;
  }

  String deviceName = String(CAMERA_BLE_DEVICE_NAME_PREFIX) + remoteIdentifier_;
  BLEDevice::init(deviceName.c_str());

  g_bleServer = BLEDevice::createServer();
  g_bleServer->setCallbacks(new RemoteServerCallbacks());

  g_bleService = g_bleServer->createService(BLEUUID(CAMERA_BLE_SERVICE_UUID));
  g_bleWriteCharacteristic = g_bleService->createCharacteristic(
      BLEUUID(CAMERA_BLE_COMMAND_CHAR_UUID),
      BLECharacteristic::PROPERTY_WRITE);
  g_bleWriteCharacteristic->setCallbacks(new RemoteGattWriteCallbacks("CE81 WRITE", true, true, true));

  g_bleNotifyCharacteristic = g_bleService->createCharacteristic(
      BLEUUID(CAMERA_BLE_NOTIFY_CHAR_UUID),
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLE2902 *notifyDescriptor = new BLE2902();
  notifyDescriptor->setNotifications(true);
  notifyDescriptor->setIndications(true);
  g_bleNotifyCharacteristic->addDescriptor(notifyDescriptor);
  uint8_t emptyValue = 0;
  g_bleNotifyCharacteristic->setValue(&emptyValue, 1);

  g_bleInfoCharacteristic = g_bleService->createCharacteristic(
      BLEUUID(CAMERA_BLE_INFO_CHAR_UUID),
      BLECharacteristic::PROPERTY_READ);
  g_bleInfoCharacteristic->setValue(CAMERA_BLE_INFO_CHAR_VALUE);

  g_bleSecondaryService = g_bleServer->createService(BLEUUID(CAMERA_BLE_SECONDARY_SERVICE_UUID));
  BLECharacteristic *secondaryWrite1 = g_bleSecondaryService->createCharacteristic(
      BLEUUID(CAMERA_BLE_SECONDARY_CHAR_UUID1),
      BLECharacteristic::PROPERTY_WRITE);
  secondaryWrite1->setCallbacks(new RemoteGattWriteCallbacks("FFD1 WRITE", false, false, false));
  g_bleSecondaryService->createCharacteristic(
      BLEUUID(CAMERA_BLE_SECONDARY_CHAR_UUID2),
      BLECharacteristic::PROPERTY_READ);

  BLECharacteristic *secondaryRead3 = g_bleSecondaryService->createCharacteristic(
      BLEUUID(CAMERA_BLE_SECONDARY_CHAR_UUID3),
      BLECharacteristic::PROPERTY_READ);
  secondaryRead3->setValue(CAMERA_BLE_SECONDARY_CHAR3_VALUE);

  BLECharacteristic *secondaryRead4 = g_bleSecondaryService->createCharacteristic(
      BLEUUID(CAMERA_BLE_SECONDARY_CHAR_UUID4),
      BLECharacteristic::PROPERTY_READ);
  secondaryRead4->setValue(CAMERA_BLE_SECONDARY_CHAR4_VALUE);

  g_bleSecondaryService->createCharacteristic(
      BLEUUID(CAMERA_BLE_SECONDARY_CHAR_UUID5),
      BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *secondaryWrite8 = g_bleSecondaryService->createCharacteristic(
      BLEUUID(CAMERA_BLE_SECONDARY_CHAR_UUID8),
      BLECharacteristic::PROPERTY_WRITE);
  secondaryWrite8->setCallbacks(new RemoteGattWriteCallbacks("FFD8 WRITE", false, false, false));
  g_bleSecondaryService->createCharacteristic(
      BLEUUID(CAMERA_BLE_SECONDARY_CHAR_UUID9),
      BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *secondaryWrite10 = g_bleSecondaryService->createCharacteristic(
      BLEUUID(CAMERA_BLE_SECONDARY_CHAR_UUID10),
      BLECharacteristic::PROPERTY_WRITE);
  secondaryWrite10->setCallbacks(new RemoteGattWriteCallbacks("FFF2 WRITE", false, false, false));
  g_bleSecondaryService->createCharacteristic(
      BLEUUID(CAMERA_BLE_SECONDARY_CHAR_UUID11),
      BLECharacteristic::PROPERTY_READ);

  g_bleService->start();
  g_bleSecondaryService->start();
  startNormalAdvertising(remoteIdentifier_);

  bleInitialized_ = true;
  return true;
#endif
}
