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

static String batteryText(const Telemetry &telemetry)
{
  if (!telemetry.batteryAvailable)
  {
    return "BAT --";
  }
  return String("BAT ") + telemetry.batteryPercent + "%";
}

static String videoText(const Telemetry &telemetry)
{
  if (!telemetry.videoRemainingAvailable)
  {
    return "REC --";
  }
  return String("REC ") + formatSeconds(telemetry.videoRemainingSec);
}

static String photoText(const Telemetry &telemetry)
{
  if (!telemetry.photosRemainingAvailable)
  {
    return "PHOTO --";
  }
  return String("PHOTO ") + telemetry.photosRemaining;
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

  const String line1 = String("CONN ") + connectionToText(model.connectionState) +
                       (model.isSleeping ? " | SLP" : " | AWAKE");
  const String line2 = String("MODE ") + modeDisplayText(model) +
                       (model.isRecording ? " | REC ON" : " | REC OFF");
  const String line3 = String("SEL ") + actionToText(selectedAction);
  const String line4 = batteryText(model.telemetry);
  const String line5 = videoText(model.telemetry);
  const String line6 = photoText(model.telemetry);
  const String line7 = String("MSG ") + model.feedback;

  const String frameKey =
      String(static_cast<int>(model.connectionState)) + "|" +
      String(static_cast<int>(model.cameraMode)) + "|" +
      model.cameraModeLabel + "|" +
      String(static_cast<int>(selectedAction)) + "|" +
      String(model.isRecording ? 1 : 0) + "|" +
      String(model.isSleeping ? 1 : 0) + "|" +
      String(model.telemetry.batteryAvailable ? 1 : 0) + "|" +
      String(model.telemetry.batteryPercent) + "|" +
      String(model.telemetry.videoRemainingAvailable ? 1 : 0) + "|" +
      String(model.telemetry.videoRemainingSec) + "|" +
      String(model.telemetry.photosRemainingAvailable ? 1 : 0) + "|" +
      String(model.telemetry.photosRemaining) + "|" +
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
    // ── Paleta Insta360 ──────────────────────────────
    static const uint16_t C_ORANGE = 0xFAA0; // #FF5500
    static const uint16_t C_WHITE = TFT_WHITE;
    static const uint16_t C_GREY = 0x8410;
    static const uint16_t C_RED = TFT_RED;
    static const uint16_t C_BG = TFT_BLACK;

    tft.startWrite();

    // Limpa apenas faixas usadas pela UI para evitar flicker de tela inteira.
    tft.fillRect(0, 0, 240, 18, C_BG);  // top status
    tft.fillRect(0, 18, 240, 40, C_BG); // tabs
    tft.fillRect(0, 58, 240, 30, C_BG); // estado/rec
    tft.fillRect(0, 88, 240, 47, C_BG); // acao + feedback
    tft.setTextSize(1);

    // ── TOP STATUS BAR (y=0..16) ─────────────────────

    // Bateria — canto superior esquerdo
    {
      String bat = model.telemetry.batteryAvailable
                       ? String(model.telemetry.batteryPercent) + "%"
                       : "--%";
      tft.setTextColor(C_WHITE, C_BG);
      tft.setCursor(4, 5);
      tft.print("BAT " + bat);
    }

    // Conexao — centro do topo
    {
      const char *conn = connectionToText(model.connectionState);
      int tw = strlen(conn) * 6;
      tft.setCursor((240 - tw) / 2, 5);
      tft.setTextColor(
          model.connectionState == ConnectionState::CONNECTED ? C_ORANGE : C_GREY,
          C_BG);
      tft.print(conn);
    }

    // Armazenamento restante — canto superior direito
    {
      String rem = remainingText(model);
      int tw = rem.length() * 6;
      tft.setCursor(240 - tw - 4, 5);
      tft.setTextColor(C_WHITE, C_BG);
      tft.print(rem);
    }

    // Divisor horizontal
    tft.drawFastHLine(0, 17, 240, C_GREY);

    // ── MODO / TABS (y=22..55) ───────────────────────
    if (model.cameraModeLabel.length() > 0)
    {
      String modeLabel = modeDisplayText(model);
      int chipY = 24;
      int chipH = 22;
      int textW = modeLabel.length() * 6;
      int chipW = textW + 18;
      if (chipW > 210)
      {
        chipW = 210;
      }
      int chipX = (240 - chipW) / 2;
      tft.drawRoundRect(chipX, chipY, chipW, chipH, 5, C_ORANGE);
      tft.setTextColor(C_ORANGE, C_BG);
      tft.setCursor(chipX + (chipW - textW) / 2, chipY + 7);
      tft.print(modeLabel);
    }
    else
    {
      const char *tabLabels[3] = {"VIDEO", "PHOTO", "TIME"};
      CameraMode tabModes[3] = {CameraMode::VIDEO, CameraMode::PHOTO, CameraMode::TIMELAPSE};
      const int tabW = 62, tabH = 26, tabY = 22;
      const int tabGap = 8;
      const int totalW = 3 * tabW + 2 * tabGap;
      const int tx0 = (240 - totalW) / 2;

      for (int i = 0; i < 3; i++)
      {
        int tx = tx0 + i * (tabW + tabGap);
        bool active = (tabModes[i] == model.cameraMode);
        if (active)
        {
          tft.fillRoundRect(tx, tabY, tabW, tabH, 5, C_ORANGE);
          tft.setTextColor(C_BG, C_ORANGE);
        }
        else
        {
          tft.drawRoundRect(tx, tabY, tabW, tabH, 5, C_GREY);
          tft.setTextColor(C_GREY, C_BG);
        }
        int lw = strlen(tabLabels[i]) * 6;
        tft.setCursor(tx + (tabW - lw) / 2, tabY + (tabH - 8) / 2);
        tft.print(tabLabels[i]);
      }
    }

    // ── INDICADOR DE GRAVACAO / ESTADO (y=60..82) ────
    if (model.isRecording)
    {
      tft.fillCircle(72, 71, 5, C_RED);
      tft.setTextColor(C_RED, C_BG);
      tft.setCursor(82, 67);
      tft.print("REC");
      if (model.telemetry.videoRemainingAvailable)
      {
        tft.setTextColor(C_WHITE, C_BG);
        tft.setCursor(112, 67);
        tft.print(formatSeconds(model.telemetry.videoRemainingSec));
      }
    }
    else if (model.isSleeping)
    {
      const char *s = "Z Z Z   SLEEPING";
      int sw = strlen(s) * 6;
      tft.setTextColor(C_GREY, C_BG);
      tft.setCursor((240 - sw) / 2, 67);
      tft.print(s);
    }

    // Divisor horizontal
    tft.drawFastHLine(0, 87, 240, C_GREY);

    // ── BARRA DE ACAO (y=91..134) ─────────────────────
    {
      tft.setTextSize(2);
      tft.setTextColor(C_ORANGE, C_BG);
      String act = String("> ") + actionToText(selectedAction);
      int aw = act.length() * 12;
      int ax = (240 - aw) / 2;
      tft.setCursor(ax < 4 ? 4 : ax, 95);
      tft.print(act);
    }

    // Feedback — rodape
    tft.setTextSize(1);
    tft.setTextColor(C_GREY, C_BG);
    tft.setCursor(4, 122);
    tft.print(model.feedback);

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
  Serial.println(line7);
}
