# Firmware MVP - TTGO T-Display ESP32 para Insta360 X5

Implementacao inicial alinhada ao PRD em [../../PRD-sample.md](../../PRD-sample.md).

## O que este MVP entrega

- menu por botao `NEXT` e confirmacao por `SELECT`,
- fluxo de conexao (scanning -> pairing -> connected),
- comandos: connect, record toggle, capture, next mode, sleep, wake,
- telemetria na tela:
  - bateria (`BAT xx%`),
  - tempo restante (`REC HH:MM:SS`),
  - fotos restantes (`PHOTO n`),
- fallback para `--` quando telemetria indisponivel,
- fallback para `Serial` quando `TFT_eSPI` nao estiver instalada.

## Estrutura

- [insta360_x5_ttgo.ino](insta360_x5_ttgo.ino)
- [config.h](config.h)
- [model.h](model.h)
- [buttons.h](buttons.h), [buttons.cpp](buttons.cpp)
- [camera_protocol.h](camera_protocol.h), [camera_protocol.cpp](camera_protocol.cpp)
- [controller.h](controller.h), [controller.cpp](controller.cpp)
- [ui.h](ui.h), [ui.cpp](ui.cpp)

## Configuracao

1. Ajuste identificador remoto em [config.h](config.h):
   - `REMOTE_IDENTIFIER`
2. Ajuste pinos dos botoes se necessario:
   - `BTN_SELECT_PIN`
   - `BTN_NEXT_PIN`
3. Escolha backend do protocolo em [config.h](config.h):
   - `CAMERA_PROTOCOL_BACKEND 0` -> simulado (padrao)
   - `CAMERA_PROTOCOL_BACKEND 1` -> BLE remoto (advertising para pareamento pela camera)
4. Parametros BLE remoto em [config.h](config.h):
   - `CAMERA_BLE_DEVICE_NAME_PREFIX`
   - `CAMERA_BLE_SERVICE_UUID`
   - `CAMERA_BLE_COMMAND_CHAR_UUID`
   - `CAMERA_BLE_NOTIFY_CHAR_UUID`
   - `CAMERA_BLE_INFO_CHAR_UUID`
   - `CAMERA_BLE_SECONDARY_SERVICE_UUID`
5. Ajustes de diagnostico/ACK em [config.h](config.h):
   - `CAMERA_BLE_LOG_NOTIFY_HEX` (log de notify em HEX)
   - `CAMERA_BLE_LOG_FRAME_SUMMARY` (resume tipo de frame e tail ASCII quando existir)
   - `CAMERA_BLE_LOG_GATT_ACTIVITY` (separa escritas em `CE81`, `FFD1`, `FFD8` e `FFF2`)
   - `CAMERA_BLE_ACK_STRICT` (ACK estrito por opcode)
   - `CAMERA_BLE_REQUIRE_ACK` (exigir ACK para considerar comando como sucesso)
   - `CAMERA_BLE_PAIR_TIMEOUT_MS` (tempo maximo para pareamento)
   - `CAMERA_BLE_LOG_CONNECTION` (log de conexao/desconexao/advertising)

## Compilacao (Arduino IDE)

1. Instale suporte ESP32 na Arduino IDE.
2. (Opcional) Instale `TFT_eSPI` para renderizacao no display.
3. Abra [insta360_x5_ttgo.ino](insta360_x5_ttgo.ino).
4. Selecione a placa ESP32 correspondente ao seu TTGO.
5. Compile e grave.

## Troubleshooting - Tela sem imagem

Se compilou e nao apareceu nada no display:

1. Verifique se a biblioteca `TFT_eSPI` esta instalada.
2. Este projeto aplica setup local da `TFT_eSPI` no arquivo [ui.cpp](ui.cpp), sem editar `User_Setup` global da biblioteca.
3. Se sua placa for revisao diferente, ajuste os pinos de display em [config.h](config.h) (`DISPLAY_PIN_*`).
4. Abra o Serial Monitor em `115200` e verifique a mensagem:
   - `[UI] TFT_eSPI detectado e inicializado.`
   - Se aparecer `[UI] TFT_eSPI nao encontrada...`, verifique [config.h](config.h) e mantenha `USE_TFT_ESPI 1`.
5. Confirme o pino de backlight em [config.h](config.h):
   - `TFT_BACKLIGHT_PIN` (padrao `4`)
   - `TFT_BACKLIGHT_ON_LEVEL` (padrao `HIGH`)
6. Se sua revisao da placa tiver pinagem diferente, ajuste esses valores.

## Limites atuais

- A camada [camera_protocol.cpp](camera_protocol.cpp) agora suporta dois backends:
   - simulado (funcional)
   - BLE remoto (advertising + pareamento pela camera + perfil GATT ampliado + envio de comandos)
- O wake implementado no firmware atual ja esta funcional.
- A telemetria de bateria/tempo/fotos ainda esta em modo simulado.
- Proximo passo: parser de notificacoes da camera para telemetria real.

## Referencias

- PRD base: [../../PRD-sample.md](../../PRD-sample.md)
- Artigo de engenharia reversa BLE: https://medium.com/@patrickchwalek/ble-control-of-insta360-cameras-7bf6894648a4
- Exemplo Arduino/ESP32 do autor: https://github.com/pchwalek/insta360_ble_esp32/tree/main

## Proximos passos BLE

- Confirmar se a X5 envia metadados adicionais por escrita em `CE81`, como descrito no artigo, alem dos frames `FE EF FE ...` ja observados.
- Manter o parser de bateria desabilitado ate surgir um campo validado nos logs reais da X5.
- Separar telemetria por origem:
   - frames `0x10` com ASCII para tempo restante
   - possiveis writes/notifies adicionais para bateria, modo e fotos restantes

### O que observar no monitor serial

- linhas `CE81 WRITE` para identificar metadados empurrados pela camera
- linhas `frame=0x.. kind=...` para detectar novos tipos de frame
- qualquer atividade em `FFD1 WRITE`, `FFD8 WRITE` ou `FFF2 WRITE`, que pode indicar canais auxiliares usados pela X5
