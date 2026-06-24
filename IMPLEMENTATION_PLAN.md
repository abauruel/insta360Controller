# Plano de Implementação (MVP)

Este plano segue o PRD em [PRD-sample.md](PRD-sample.md) e usa uma abordagem estilo skill-driven (especificação -> implementação -> validação).

## 1. Objetivo do MVP

Entregar firmware inicial para TTGO T-Display ESP32 com:

- navegação por botões físicos,
- fluxo de conexão com câmera,
- execução dos comandos principais (record/photo/mode/sleep/wake),
- exibição de estado na tela,
- telemetria (bateria/tempo/fotos) com fallback `--` quando indisponível.

## 2. Arquitetura

- `insta360_x5_ttgo.ino`: bootstrap e loop principal.
- `model.*`: estados globais e tipos do domínio.
- `buttons.*`: leitura com debounce dos botões.
- `camera_protocol.*`: camada de comando/telemetria (com modo simulado para validação local).
- `controller.*`: estado de menu e roteamento de ações.
- `ui.*`: renderização (TFT quando disponível, Serial como fallback).

## 3. Escopo implementado agora

- Estrutura completa do firmware.
- Máquina de estado de conexão.
- Ações centrais do PRD.
- Telemetria com disponibilidade condicional.
- Documentação de build e configuração.

## 4. Próxima evolução

- Trocar modo simulado por integração BLE real da Insta360 X5.
- Mapear UUIDs/características finais e confirmações por ACK.
- Ajustar layout final de UI para uso outdoor.

## 5. Referências de engenharia reversa

- PRD: [PRD-sample.md](PRD-sample.md)
- Artigo: https://medium.com/@patrickchwalek/ble-control-of-insta360-cameras-7bf6894648a4
- Código de referência: https://github.com/pchwalek/insta360_ble_esp32/tree/main

## 6. Próximos passos práticos

- Verificar se a X5 replica o comportamento descrito no artigo: comandos saem pelo remoto e metadados retornam da câmera depois de cada ação.
- Instrumentar logs para distinguir melhor duas fontes de dados:
	- notificações/leitura relacionadas ao fluxo `FE EF FE`
	- eventuais writes da câmera em `CE81` com strings de modo, frames restantes ou outros estados.
- Manter a implementação atual de wake, que já está funcional, e focar agora em telemetria real.
- Só reativar leitura de bateria após correlacionar logs de dois níveis reais de carga com um byte/campo estável.