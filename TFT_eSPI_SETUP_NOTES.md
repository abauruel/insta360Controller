# Configuração TFT_eSPI - LilyGo T-Display

## 📋 Resumo da Implementação

A configuração do TFT_eSPI para a placa **LilyGo T-Display** com display **ST7789V** foi implementada em 3 arquivos:

### 1. **config.h** - Definições de Pinos e Frequência
Arquivo de configuração centralizado com:
- Pinos SPI (MOSI, SCLK, CS, DC, RST)
- Frequência SPI otimizada: **40 MHz** (melhor performance para ESP32)
- Controle de backlight (GPIO4)

### 2. **ui.cpp** - Configuração Local do TFT_eSPI
Configuração inline dentro do código (sem depender de `User_Setup.h` global):
- Define `USER_SETUP_LOADED` antes de incluir `TFT_eSPI.h`
- Referencia valores de `config.h`
- Inclui otimizações: `TFT_BUFFER_SIZE`, fontes carregadas, suporte smooth font

### 3. **User_Setup.h** - Referência para Configuração Global (Opcional)
Arquivo de backup/referência que pode ser copiado para a pasta da biblioteca TFT_eSPI se preferir:
- Pode servir como fallback
- Documentado com instruções detalhadas
- Útil se mudar para setup global no futuro

## 🔌 Pinagem Implementada

| Função | GPIO ESP32 | Descrição |
|--------|----------|-----------|
| **MOSI (DIN)** | GPIO 19 | Dados → Display |
| **SCLK (CLK)** | GPIO 18 | Clock SPI |
| **CS** | GPIO 5 | Chip Select |
| **DC (RS)** | GPIO 16 | Data/Command |
| **RST** | GPIO 23 | Reset |
| **BL** | GPIO 4 | Backlight PWM |

## ⚡ Configurações de Performance

```cpp
// Velocidade SPI otimizada para ST7789V + ESP32
#define SPI_FREQUENCY      40000000   // 40 MHz (escrita)
#define SPI_READ_FREQUENCY 6000000    // 6 MHz (leitura)

// Buffer otimizado
#define TFT_BUFFER_SIZE 512

// Fontes ativadas
#define LOAD_GLCD   1    // GLCD
#define LOAD_FONT2  1    // 16px
#define LOAD_FONT4  1    // 26px
#define LOAD_FONT6  1    // 48px
#define LOAD_FONT7  1    // 48px
#define LOAD_FONT8  1    // 60px
#define LOAD_GFXFF  1    // TrueType fonts
#define SMOOTH_FONT 1    // Anti-alias
```

## ✅ Verificação de Funcionamento

### Testar com o Exemplo de Diagnóstico
```bash
cd firmware/display_diagnostic
arduino-cli compile -b esp32:esp32:esp32 .
arduino-cli upload -b esp32:esp32:esp32 -p /dev/cu.usbserial-0206DBE3 .
```

### Monitorar Output
```bash
arduino-cli monitor -p /dev/cu.usbserial-0206DBE3 -c baudrate=115200
```

## 🔧 Troubleshooting

### Display não aparece?
1. **Verificar frequência SPI**: Testar com 20-27 MHz ao invés de 40 MHz
2. **Revisar conexões**: Confirmar pinos MOSI, SCLK, CS, DC, RST
3. **Reset Manual**: Adicionar delay após init → `tft.init(); delay(100);`
4. **Backlight**: Verificar se GPIO4 está em HIGH no setup

### Display com cores erradas?
1. **Invertir display**: `tft.invertDisplay(true);`
2. **Rotacao**: Ajustar `tft.setRotation(0-3);`

### Performance lenta?
1. **Aumentar SPI_FREQUENCY**: Tentar 50-60 MHz
2. **Habilitar buffer**: `#define TFT_BUFFER_SIZE 1024`
3. **Desabilitar PSRAM**: Comentar `#define SPRITE_USE_PSRAM`

## 📚 Referências

- [TFT_eSPI GitHub](https://github.com/Bodmer/TFT_eSPI)
- [LilyGo T-Display Wiki](https://wiki.lilygo.cc/products/t-display-series/t-display/)
- [ST7789V Datasheet](https://wiki.lilygo.cc/products/t-display-series/t-display/#schematic)

## 🎯 Próximas Etapas

1. Compilar e fazer upload do firmware
2. Testar display com `display_diagnostic/display_diagnostic.ino`
3. Validar rotacao, cores e fonts
4. Ajustar frequência SPI se necessário para estabilidade
5. Implementar UI completa em `insta360_x5_ttgo.ino`
