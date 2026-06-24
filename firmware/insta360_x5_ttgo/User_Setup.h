// ======================================================================
// User_Setup.h - Configuracao para TFT_eSPI com LilyGo T-Display
// ======================================================================
// Este arquivo pode ser copiado para a pasta da biblioteca TFT_eSPI se
// preferir usar configuracao global ao inves de config local em ui.cpp
// ======================================================================

#ifndef USER_SETUP_LOADED
#define USER_SETUP_LOADED

// ======================================================================
// DRIVER ST7789V - 240x135 pixels (LilyGo T-Display)
// ======================================================================
#define ST7789_DRIVER

// ======================================================================
// RESOLUCAO DO DISPLAY
// ======================================================================
#define TFT_WIDTH  135   // Altura (com rotacao aplicada)
#define TFT_HEIGHT 240   // Largura (com rotacao aplicada)

// ======================================================================
// PINOS SPI 4-WIRE - ESP32 PINS
// ======================================================================
#define TFT_MOSI   19    // GPIO19 - Serial Data Input (DIN/MOSI)
#define TFT_SCLK   18    // GPIO18 - Serial Clock (CLK/SCLK)
#define TFT_CS     5     // GPIO5  - Chip Select (CS)
#define TFT_DC     16    // GPIO16 - Data/Command (RS/DC)
#define TFT_RST    23    // GPIO23 - Reset (RST)

// ======================================================================
// BACKLIGHT CONTROL
// ======================================================================
#define TFT_BL     4     // GPIO4 - Backlight control (PWM capable)
#define TFT_BACKLIGHT_ON HIGH

// ======================================================================
// VELOCIDADE SPI
// ======================================================================
// ESP32 com ST7789V pode usar ate 80MHz, mas 40MHz e recomendado
// para estabilidade com cabos longos ou shields
#define SPI_FREQUENCY      40000000   // 40 MHz - escrita
#define SPI_READ_FREQUENCY 6000000    // 6 MHz - leitura

// ======================================================================
// FONTES - Selecionar quais carregar para economizar FLASH
// ======================================================================
#define LOAD_GLCD      1   // Fonte pequena (GLCD)
#define LOAD_FONT2     1   // 16 pixels altura
#define LOAD_FONT4     1   // 26 pixels altura
#define LOAD_FONT6     1   // 48 pixels altura
#define LOAD_FONT7     1   // 48 pixels altura
#define LOAD_FONT8     1   // 60 pixels altura
#define LOAD_GFXFF     1   // Suporte para TrueType fonts
#define SMOOTH_FONT    1   // Anti-alias em fontes suaves

// ======================================================================
// SPRITE E BUFFER
// ======================================================================
// Aumentar buffer size para melhor performance
#define TFT_BUFFER_SIZE 512

// Habilitar sprite (frame buffer na RAM)
// Comentar se RAM for limitada
// #define SPRITE_USE_PSRAM

// ======================================================================
// ANIMACOES
// ======================================================================
// Desabilitar efeitos de transicao se necessario economizar ciclos CPU

// ======================================================================
// SUPORTE A LEITURA E BUSCA
// ======================================================================
// Descomentar se quiser ler pixels do display (consome mais RAM)
// #define TFT_SDA_READ  // Para interfaces I2C apenas
// #define TFT_SPI_SHARED  // Se SPI e compartilhado com SD card

// ======================================================================
// NOTAS DE CONFIGURACAO
// ======================================================================
// 1. Orientacao padrão (rotation 0) = retrato (135x240)
//    Use tft.setRotation(1) no codigo para paisagem (240x135)
//
// 2. Se houver problemas de display nao aparecer:
//    - Verificar conexao dos pinos
//    - Testar com frequencia SPI menor (10-20MHz)
//    - Usar o exemplo display_diagnostic.ino para debug
//
// 3. Para estabilizar comunicacao SPI:
//    - Manter fios curtos entre ESP32 e display
//    - Adicionar capacitor 100nF entre CS e GND
//    - Considerar resistor 10k entre RST e VCC
//
// 4. Performance:
//    - 40MHz oferece bom balance entre velocidade e estabilidade
//    - Para aplicacoes criticas, testar com 27MHz
//    - DMA nao suportado com ST7789V neste setup

#endif // USER_SETUP_LOADED
