#pragma once

// Identificador exibido no remote/advertising para evitar conflito entre dispositivos.
static const char REMOTE_IDENTIFIER[] = "A01";

// Mapeamento padrão de botões para TTGO T-Display ESP32.
// Ajuste conforme revisão da sua placa, se necessário.
static const int BTN_SELECT_PIN = 0;
static const int BTN_NEXT_PIN = 35;

// TTGO T-Display (ST7789): normalmente o backlight e controlado pelo GPIO4.
// Se sua revisao usar outro pino, ajuste aqui.
static const int TFT_BACKLIGHT_PIN = 4;
static const int TFT_BACKLIGHT_ON_LEVEL = HIGH;

// Habilita uso do TFT_eSPI. Se quiser compilar sem a lib, altere para 0.
#define USE_TFT_ESPI 1

// Setup local do display TTGO T-Display (ST7789 240x135).
// Esses defines sao usados para configurar o TFT_eSPI sem editar arquivos na pasta da biblioteca.
#define DISPLAY_ST7789 1
#define DISPLAY_TFT_WIDTH 135              // Altura fisica (rotacionado)
#define DISPLAY_TFT_HEIGHT 240             // Largura fisica (rotacionado)
#define DISPLAY_PIN_MOSI 19                // GPIO19 - Data
#define DISPLAY_PIN_SCLK 18                // GPIO18 - Clock
#define DISPLAY_PIN_CS 5                   // GPIO5  - Chip Select
#define DISPLAY_PIN_DC 16                  // GPIO16 - Data/Command
#define DISPLAY_PIN_RST 23                 // GPIO23 - Reset
#define DISPLAY_SPI_FREQUENCY 40000000     // 40MHz - Otimizado para ESP32 com ST7789V
#define DISPLAY_SPI_READ_FREQUENCY 6000000 // 6MHz para leitura

static const unsigned long BUTTON_DEBOUNCE_MS = 40;
static const unsigned long UI_REFRESH_MS = 250;

// Backend do protocolo da camera:
// 0 = Simulado (padrao MVP), 1 = BLE remoto (peripheral/advertising, padrao Insta360 GPS Remote).
#define CAMERA_PROTOCOL_BACKEND 1

// Nome anunciado para a camera detectar este dispositivo como remoto.
static const char CAMERA_BLE_DEVICE_NAME_PREFIX[] = "Insta360 GPS Remote ";

// UUIDs do perfil de remoto (referencia Serial Hobbyism / Insta360 GPS Remote).
static const char CAMERA_BLE_SERVICE_UUID[] = "0000ce80-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_COMMAND_CHAR_UUID[] = "0000ce81-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_NOTIFY_CHAR_UUID[] = "0000ce82-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_INFO_CHAR_UUID[] = "0000ce83-0000-1000-8000-00805f9b34fb";
static const uint16_t CAMERA_BLE_INFO_CHAR_VALUE = 0x0201;

static const char CAMERA_BLE_SECONDARY_SERVICE_UUID[] = "0000d0ff-3c17-d293-8e48-14fe2e4da212";
static const char CAMERA_BLE_SECONDARY_CHAR_UUID1[] = "0000ffd1-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_SECONDARY_CHAR_UUID2[] = "0000ffd2-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_SECONDARY_CHAR_UUID3[] = "0000ffd3-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_SECONDARY_CHAR_UUID4[] = "0000ffd4-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_SECONDARY_CHAR_UUID5[] = "0000ffd5-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_SECONDARY_CHAR_UUID8[] = "0000ffd8-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_SECONDARY_CHAR_UUID9[] = "0000fff1-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_SECONDARY_CHAR_UUID10[] = "0000fff2-0000-1000-8000-00805f9b34fb";
static const char CAMERA_BLE_SECONDARY_CHAR_UUID11[] = "0000ffe0-0000-1000-8000-00805f9b34fb";
static const uint32_t CAMERA_BLE_SECONDARY_CHAR3_VALUE = 0x301E9001UL;
static const uint32_t CAMERA_BLE_SECONDARY_CHAR4_VALUE = 0x18002001UL;

static const unsigned long CAMERA_BLE_ACK_TIMEOUT_MS = 800;
static const unsigned long CAMERA_BLE_PAIR_TIMEOUT_MS = 30000;
static const bool CAMERA_BLE_REQUIRE_ACK = false;

// Diagnostico BLE para engenharia reversa do protocolo.
// Se true, imprime todos os notifications em HEX no Serial.
static const bool CAMERA_BLE_LOG_NOTIFY_HEX = true;

// Log resumido por frame recebido da camera, para destacar novos tipos alem de
// 0x02 e 0x10 durante a engenharia reversa da X5.
static const bool CAMERA_BLE_LOG_FRAME_SUMMARY = true;

// Log de escritas GATT por characteristic, separando CE81 das demais.
static const bool CAMERA_BLE_LOG_GATT_ACTIVITY = true;

// Estrategia de ACK:
// false -> qualquer notify conta como ACK.
// true  -> exige opcode do notify igual ao comando (ou comando|0x80).
static const bool CAMERA_BLE_ACK_STRICT = false;

// Logs de advertising/conexao no modo remoto BLE.
static const bool CAMERA_BLE_LOG_CONNECTION = true;

// Parser experimental de bateria BLE.
// Habilitado para mapeamento do frame.
static const bool CAMERA_BLE_ENABLE_EXPERIMENTAL_BATTERY = true;
// Hipótese: b9 em frame 0x02 correlaciona com bateria
// Formula calibrada com dois pontos medidos:
//   b9=8  @ 31% real  → 8*5 - 9  = 31% ✓
//   b9=21 @ ~100% real → 21*5 - 9 = 96% (~ok)
// Historico de medicoes:
//   36-39%: b9=8 → 35% (com offset -5, erro +4%)
//   31%:    b9=8 → 31% (com offset -9, calibrado)
static const uint8_t CAMERA_BLE_BATTERY_BYTE_INDEX = 9;
static const int CAMERA_BLE_BATTERY_OFFSET_PERCENT = -9;
static const int CAMERA_BLE_BATTERY_SCALE_FACTOR = 5;

// Diagnostico para descobrir qual byte representa bateria nos frames BLE.
// Imprime resumo dos frames 0x02 e 0x10 com TODOS os bytes para comparacao.
static const bool CAMERA_BLE_LOG_BATTERY_CANDIDATES = true;
// Extra: imprimir todos os bytes de cada frame 0x02/0x10 para analise raw
static const bool CAMERA_BLE_LOG_FRAME_HEX_ALL_BYTES = true;
