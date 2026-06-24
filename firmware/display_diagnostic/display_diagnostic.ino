#include <Arduino.h>

// ======================================================================
// Configuracao local do TFT_eSPI para ST7789V na LilyGo T-Display
// Este arquivo de diagnostico testa a comunicacao e exibe no display
// ======================================================================

#define USER_SETUP_LOADED 1
#define ST7789_DRIVER

// Resolucao do display (135x240 com rotacao aplicada)
#define TFT_WIDTH 135
#define TFT_HEIGHT 240

// Pinos SPI 4-wire ESP32
#define TFT_MOSI 19     // GPIO19 - Serial Data In
#define TFT_SCLK 18     // GPIO18 - Serial Clock
#define TFT_CS 5        // GPIO5  - Chip Select
#define TFT_DC 16       // GPIO16 - Data/Command
#define TFT_RST 23      // GPIO23 - Reset (IMPORTANTE: nao usar -1!)

// Backlight control
#define TFT_BL 4        // GPIO4 - Backlight PWM
#define TFT_BACKLIGHT_ON HIGH

// Fontes
#define LOAD_GLCD   1
#define LOAD_FONT2  1
#define LOAD_FONT4  1
#define LOAD_FONT6  1
#define LOAD_FONT7  1
#define LOAD_FONT8  1
#define LOAD_GFXFF  1
#define SMOOTH_FONT 1

// Performance - velocidade otimizada
#define SPI_FREQUENCY 40000000      // 40 MHz - otimizado para ESP32
#define SPI_READ_FREQUENCY 6000000  // 6 MHz para leitura

// Buffer otimizado
#define TFT_BUFFER_SIZE 512

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void showScreen(const char *title, uint16_t bg, uint16_t fg, int blLevel)
{
  digitalWrite(TFT_BL, blLevel);
  tft.fillScreen(bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.println("TTGO T-Display diag");
  tft.print("BL level: ");
  tft.println(blLevel == HIGH ? "HIGH" : "LOW");
  tft.println(title);
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("[DIAG] Boot");

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  Serial.println("[DIAG] Backlight ON");

  Serial.println("[DIAG] Antes de tft.init()");
  tft.init();
  Serial.println("[DIAG] Depois de tft.init()");
  tft.setRotation(1);
  tft.invertDisplay(false);

  Serial.println("[DIAG] Iniciado");
  Serial.println("[DIAG] Fase 1: BL HIGH por 3s");
  showScreen("FASE 1", TFT_WHITE, TFT_BLACK, HIGH);
  delay(3000);

  Serial.println("[DIAG] Fase 2: BL LOW por 3s");
  showScreen("FASE 2", TFT_WHITE, TFT_BLACK, LOW);
  delay(3000);

  // Mantem ligado no nivel padrao e mostra tela estatica de diagnostico.
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  Serial.println("[DIAG] Tela estatica de diagnostico");
  showScreen("ST7789V INIT OK", TFT_BLACK, TFT_WHITE, TFT_BACKLIGHT_ON);
}

void loop()
{
  delay(1000);
}
