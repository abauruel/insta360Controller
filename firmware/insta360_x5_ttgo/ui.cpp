#include "ui.h"

#include "config.h"

#if USE_TFT_ESPI
// ======================================================================
// Configuracao local do TFT_eSPI para ST7789V na LilyGo T-Display
// Evita dependencia de User_Setup global da biblioteca
// ======================================================================

// Marcar setup como carregado para evitar conflitos com User_Setup.h
#define USER_SETUP_LOADED 1

// Driver display
#define ST7789_DRIVER

// Dimensoes do display (240x135, mas rotacionado para 135x240 na exibicao)
#define TFT_WIDTH DISPLAY_TFT_WIDTH
#define TFT_HEIGHT DISPLAY_TFT_HEIGHT

// Pinos SPI 4-wire para ESP32
#define TFT_MOSI DISPLAY_PIN_MOSI // GPIO19 - Master Out Slave In
#define TFT_SCLK DISPLAY_PIN_SCLK // GPIO18 - Serial Clock
#define TFT_CS DISPLAY_PIN_CS     // GPIO5  - Chip Select
#define TFT_DC DISPLAY_PIN_DC     // GPIO16 - Data/Command (RS)
#define TFT_RST DISPLAY_PIN_RST   // GPIO23 - Reset

// Backlight PWM control
#define TFT_BL TFT_BACKLIGHT_PIN // GPIO4
#define TFT_BACKLIGHT_ON TFT_BACKLIGHT_ON_LEVEL

// Velocidade SPI otimizada para ESP32
// 40MHz recomendado para ST7789V com ESP32
#define SPI_FREQUENCY DISPLAY_SPI_FREQUENCY
#define SPI_READ_FREQUENCY DISPLAY_SPI_READ_FREQUENCY

// Fonte e graficos - selecionar quais ativar para economizar FLASH
#define LOAD_GLCD 1   // Fonte GLCD
#define LOAD_FONT2 1  // Pequena
#define LOAD_FONT4 1  // Media
#define LOAD_FONT6 1  // Maior
#define LOAD_FONT7 1  // Maior
#define LOAD_FONT8 1  // Maior
#define LOAD_GFXFF 1  // Fontes TrueType
#define SMOOTH_FONT 1 // Anti-alias em fontes

// Performance - aumentar buffer para melhor velocidade
#define TFT_BUFFER_SIZE 512

// Desabilitar SDA_READ (para I2C, nao SPI)
// #define TFT_SDA_READ

// Incluir a biblioteca com configuracoes definidas acima
#include <TFT_eSPI.h>
static TFT_eSPI tft = TFT_eSPI();
static bool tftAvailable = true;
#else
static bool tftAvailable = false;
#endif

static String formatSeconds(unsigned long sec)
{
  const unsigned long h = sec / 3600;
  const unsigned long m = (sec % 3600) / 60;
  const unsigned long s = sec % 60;

  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", h, m, s);
  return String(buffer);
}

static String remainingText(const SystemModel &model)
{
  if (model.cameraMode == CameraMode::VIDEO)
  {
    return model.telemetry.videoRemainingAvailable
               ? formatSeconds(model.telemetry.videoRemainingSec)
               : "--:--";
  }

  if (model.cameraMode == CameraMode::PHOTO || model.cameraMode == CameraMode::TIMELAPSE)
  {
    return model.telemetry.photosRemainingAvailable
               ? String(model.telemetry.photosRemaining) + "p"
               : "--p";
  }

  if (model.telemetry.videoRemainingAvailable)
  {
    return formatSeconds(model.telemetry.videoRemainingSec);
  }

  if (model.telemetry.photosRemainingAvailable)
  {
    return String(model.telemetry.photosRemaining) + "p";
  }

  return "SYNC";
}

static String modeDisplayText(const SystemModel &model)
{
  if (model.cameraModeLabel.length() > 0)
  {
    return model.cameraModeLabel;
  }
  return modeToText(model.cameraMode);
}

void Ui::begin()
{
  pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
  digitalWrite(TFT_BACKLIGHT_PIN, TFT_BACKLIGHT_ON_LEVEL);

  if (tftAvailable)
  {
#if USE_TFT_ESPI
    tft.init();
    tft.setRotation(1);
    tft.invertDisplay(true);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.println("Insta360 X5 Remote");
    tft.println("Display OK");
    delay(500);
#endif
    Serial.println("[UI] TFT_eSPI detectado e inicializado.");
  }
  else
  {
    Serial.println("[UI] TFT_eSPI nao encontrada. Usando fallback Serial.");
  }
}

void Ui::render(const SystemModel &model, MenuAction selectedAction)
{
  const unsigned long now = millis();
  if ((now - lastDrawMs_) < UI_REFRESH_MS)
  {
    return;
  }
  lastDrawMs_ = now;

  String actionLabel;
  if (selectedAction == MenuAction::CONNECT)
  {
    actionLabel = "CONECTAR";
  }
  else
  {
    actionLabel = model.isRecording ? "PARAR GRAV" : "INICIAR GRAV";
  }

  String remainingLabel;
  if (model.cameraMode == CameraMode::PHOTO || model.cameraMode == CameraMode::TIMELAPSE)
  {
    remainingLabel = "SHOTS";
  }
  else
  {
    remainingLabel = "REMAIN";
  }

  const String line1 = String("LINK ") + connectionToText(model.connectionState);
  const String line2 = modeDisplayText(model);
  const String line3 = remainingLabel;
  const String line4 = remainingText(model);
  const String line5 = String("> ") + actionLabel;
  const String line6 = model.feedback;

  const String frameKey =
      String(static_cast<int>(model.connectionState)) + "|" +
      String(static_cast<int>(model.cameraMode)) + "|" +
      model.cameraModeLabel + "|" +
      String(static_cast<int>(selectedAction)) + "|" +
      String(model.isRecording ? 1 : 0) + "|" +
      String(model.isSleeping ? 1 : 0) + "|" +
      String(model.telemetry.videoRemainingAvailable ? 1 : 0) + "|" +
      String(model.telemetry.videoRemainingSec) + "|" +
      String(model.telemetry.photosRemainingAvailable ? 1 : 0) + "|" +
      String(model.telemetry.photosRemaining) + "|" +
      actionLabel + "|" +
      model.feedback;
  static String lastFrameKey;
  if (frameKey == lastFrameKey)
  {
    return;
  }
  lastFrameKey = frameKey;

  if (tftAvailable)
  {
#if USE_TFT_ESPI
    static const int SCREEN_W = 240;
    static const int MARGIN_X = 12;
    static const int CONTENT_X = MARGIN_X;
    static const int CONTENT_W = SCREEN_W - (MARGIN_X * 2);

    static const uint16_t C_WHITE = TFT_WHITE;
    static const uint16_t C_GREY = 0x8410;
    static const uint16_t C_RED = TFT_RED;
    static const uint16_t C_BG = TFT_BLACK;

    tft.startWrite();

    tft.fillRect(0, 0, 240, 135, C_BG);
    tft.setTextSize(1);

    // Header minimalista: status de conexao e rec
    tft.setTextColor(C_GREY, C_BG);
    tft.setCursor(CONTENT_X, 6);
    tft.print(line1);

    if (model.isRecording)
    {
      const int recDotX = CONTENT_X + CONTENT_W - 36;
      tft.fillCircle(recDotX, 10, 4, C_RED);
      tft.setTextColor(C_RED, C_BG);
      tft.setCursor(recDotX + 10, 6);
      tft.print("REC");
    }

    // Linha divisoria superior
    tft.drawFastHLine(CONTENT_X, 18, CONTENT_W, C_GREY);

    // Modo (centro superior)
    tft.setTextColor(C_WHITE, C_BG);
    tft.setTextSize(2);
    int modeW = line2.length() * 12;
    int modeX = CONTENT_X + (CONTENT_W - modeW) / 2;
    tft.setCursor(modeX < CONTENT_X ? CONTENT_X : modeX, 28);
    tft.print(line2);

    // Rotulo e valor principal (grande, estilo visor)
    tft.setTextSize(1);
    tft.setTextColor(C_GREY, C_BG);
    int labelW = line3.length() * 6;
    int labelX = CONTENT_X + (CONTENT_W - labelW) / 2;
    tft.setCursor(labelX < CONTENT_X ? CONTENT_X : labelX, 52);
    tft.print(line3);

    tft.setTextSize(2);
    tft.setTextColor(C_WHITE, C_BG);
    int valueW = line4.length() * 12;
    int valueX = CONTENT_X + (CONTENT_W - valueW) / 2;
    tft.setCursor(valueX < CONTENT_X ? CONTENT_X : valueX, 62);
    tft.print(line4);

    // Barra inferior com acao selecionada
    tft.fillRect(CONTENT_X, 108, CONTENT_W, 27, C_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(C_BG, C_WHITE);
    int actionW = line5.length() * 6;
    int actionX = CONTENT_X + (CONTENT_W - actionW) / 2;
    tft.setCursor(actionX < CONTENT_X ? CONTENT_X : actionX, 112);
    tft.print(line5);

    // Mensagem curta de estado
    tft.setTextColor(C_GREY, C_BG);
    tft.setCursor(CONTENT_X, 97);
    if (line6.length() > 36)
    {
      String shortMsg = line6;
      shortMsg.remove(36);
      tft.print(shortMsg);
    }
    else
    {
      tft.print(line6);
    }

    tft.endWrite();
#endif
    return;
  }

  // Fallback serial para debug quando TFT nao estiver disponivel.
  Serial.println("---------------------------");
  Serial.println(line1);
  Serial.println(line2);
  Serial.println(line3);
  Serial.println(line4);
  Serial.println(line5);
  Serial.println(line6);
}
