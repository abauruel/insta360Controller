# PRD - Controle remoto para Insta360 X5 com TTGO T-Display ESP32

## 1. Visão geral

Este projeto tem como objetivo criar um controle remoto IoT para a câmera Insta360 X5 utilizando o módulo TTGO T-Display ESP32 WiFi com LCD de 1,14". A solução deve permitir operar funções principais da câmera sem depender do app oficial no celular, com uma interface local simples e confiável no próprio dispositivo.

O ponto de partida técnico é a referência de uso do projeto open source da Serial Hobbyism e o repositório de controle remoto baseado em ESP32/M5StickC.

Referências:

- https://serialhobbyism.com/open-source-diy-remote-for-insta360-cameras
- https://github.com/theserialhobbyist/insta360_m5StickC_remote/tree/main

## 2. Problema

Em muitos cenários de uso da Insta360 X5, o usuário precisa iniciar gravações, alternar modos ou colocar a câmera em repouso sem acesso rápido ao aplicativo móvel. Isso é especialmente relevante em atividades como bicicleta, moto, esportes ao ar livre ou montagens em que o celular não é prático.

Hoje não existe, para este contexto, um controle físico compacto, dedicado e com feedback local suficiente para operar a câmera com segurança.

## 3. Objetivo do produto

Construir um controle remoto dedicado que permita comandar a Insta360 X5 por Bluetooth, com interface em tela e botões físicos no TTGO T-Display ESP32.

## 4. Objetivos de sucesso

- Permitir conexão estável com a Insta360 X5.
- Executar ações essenciais da câmera com poucos toques.
- Exibir feedback claro do estado atual da conexão e da ação executada.
- Funcionar de forma autônoma, sem depender de smartphone durante o uso normal.
- Ser simples de instalar, configurar e manter.

## 5. Público-alvo

- Usuários da Insta360 X5 que gravam em movimento.
- Criadores de conteúdo que usam suporte, capacete, bicicleta, moto ou tripé.
- Entusiastas de IoT e eletrônica que desejam adaptar o projeto open source para um hardware diferente.

## 6. Escopo inicial

### 6.1 Dentro do escopo

- Pareamento e reconexão Bluetooth com a Insta360 X5.
- Iniciar e parar gravação.
- Capturar foto.
- Alternar modos de câmera, quando suportado pelo protocolo.
- Colocar a câmera em sleep.
- Despertar a câmera.
- Exibir status da conexão na tela.
- Exibir o comando atualmente selecionado.
- Aceitar controle por botões físicos do TTGO T-Display ESP32.

### 6.2 Fora do escopo inicial

- Streaming de vídeo.
- Transferência de arquivos por Bluetooth.
- Edição de mídia.
- Controle total de configurações avançadas da câmera.
- Sincronização com nuvem ou app móvel.
- Suporte nativo a múltiplas câmeras no primeiro release.

## 7. Premissas e restrições

- A solução será baseada em comunicação Bluetooth Low Energy, conforme a referência do projeto original.
- O TTGO T-Display ESP32 possui recursos suficientes para tela, botões e comunicação BLE.
- O comportamento exato da Insta360 X5 pode exigir ajustes de protocolo, identificação e emparelhamento.
- Deve existir uma forma simples de configurar um identificador único do controle para evitar conflito entre múltiplos dispositivos.
- O produto deve priorizar confiabilidade e uso prático em campo, acima de customizações complexas.

## 8. Requisitos funcionais

### 8.1 Conexão e pareamento

- O dispositivo deve iniciar em estado desacoplado ou reconectar automaticamente ao último pareamento válido.
- O sistema deve permitir iniciar o processo de pareamento com a câmera a partir da interface local.
- O dispositivo deve indicar claramente quando a câmera foi encontrada, quando está pareando e quando a conexão foi concluída.
- Caso a conexão falhe, o usuário deve ver uma mensagem de erro ou status de falha.

### 8.2 Controle da câmera

- O usuário deve conseguir iniciar e parar gravação.
- O usuário deve conseguir capturar uma foto.
- O usuário deve conseguir alternar entre os modos suportados pela câmera.
- O usuário deve conseguir colocar a câmera em sleep.
- O usuário deve conseguir acordar a câmera.

### 8.3 Interface local

- A tela deve exibir o estado da conexão.
- A tela deve exibir o comando selecionado no menu.
- A navegação deve ser possível por botões físicos.
- A interface deve ser legível em ambientes externos e sob luz natural.

### 8.4 Configuração

- O projeto deve permitir configurar um identificador único do controle remoto.
- O identificador deve ser editável sem necessidade de alterar várias partes do código.
- O firmware deve ser fácil de compilar e gravar via Arduino IDE ou fluxo equivalente.

### 8.5 Feedback do sistema

- O dispositivo deve informar sucesso ou falha das ações principais.
- O sistema deve sinalizar reconexão automática quando o vínculo Bluetooth for restabelecido.
- O usuário deve perceber quando a câmera está pronta para receber comandos.

### 8.6 Telemetria e disponibilidade de mídia

- O dispositivo deve exibir, quando a câmera disponibilizar a informação, o nível de bateria restante.
- O dispositivo deve exibir, quando disponível, o tempo restante estimado de gravação de vídeo.
- O dispositivo deve exibir, quando disponível, a quantidade estimada de fotos restantes.
- Caso a Insta360 X5 não exponha esses dados pelo protocolo acessado pelo projeto, o PRD deve tratar isso como limitação conhecida da primeira versão, com possibilidade de evolução futura.

### 8.7 Exemplos de exibição na tela

- Bateria: `BAT 78%`
- Vídeo: `REC 00:41:20`
- Fotos: `PHOTO 124`
- Sem telemetria disponível: `BAT -- | REC -- | PHOTO --`

## 9. Requisitos não funcionais

- Baixa latência entre comando do usuário e ação na câmera.
- Consumo de energia compatível com uso portátil.
- Interface estável, sem travamentos durante reconexão.
- Código legível e organizado para facilitar manutenção e adaptação de hardware.
- Compatibilidade com um fluxo de desenvolvimento simples para makers.

## 10. Experiência do usuário

### Fluxo principal

1. O usuário liga o TTGO T-Display ESP32.
2. O dispositivo mostra o status inicial.
3. O usuário inicia o pareamento com a Insta360 X5.
4. A câmera confirma a conexão.
5. O usuário navega pelos comandos com os botões físicos.
6. O dispositivo executa o comando e mostra o retorno na tela.

### Fluxo de uso em campo

1. A câmera já está montada no equipamento.
2. O usuário pega o controle sem acessar o celular.
3. Ele inicia a gravação rapidamente.
4. Ao final, interrompe a gravação e coloca a câmera em repouso, se necessário.

## 11. Critérios de aceitação

O projeto será considerado bem-sucedido quando:

- O TTGO T-Display ESP32 conseguir parear com a Insta360 X5.
- Os comandos de gravação, foto, sleep e wake funcionarem de forma consistente.
- A tela mostrar estados úteis para o usuário durante operação e conexão.
- O dispositivo puder ser usado sem smartphone no fluxo principal.
- A configuração do identificador do controle for simples e documentada.

## 12. Riscos e dependências

- A Insta360 X5 pode exigir ajustes específicos de protocolo Bluetooth.
- Mudanças de firmware da câmera podem alterar o comportamento esperado.
- A limitação de tela e botões do TTGO pode restringir a interface inicial.
- O pareamento pode ser sensível a identificadores, UUIDs ou nomes do dispositivo.
- A implementação pode depender de bibliotecas específicas do ecossistema ESP32 e Arduino.
- A exposição de bateria, tempo restante de gravação e fotos restantes depende de esses dados estarem acessíveis pelo protocolo da Insta360 X5 ou pelas bibliotecas utilizadas.

## 13. Métricas de sucesso

- Taxa de sucesso de pareamento com a X5.
- Tempo médio para conectar após ligar o dispositivo.
- Percentual de comandos executados sem falha.
- Número de passos necessários para iniciar gravação.
- Percepção de usabilidade em cenário real de campo.

## 14. Roadmap sugerido

### Fase 1 - Prova de conceito

- Validar comunicação Bluetooth com a Insta360 X5.
- Reproduzir os comandos principais do projeto de referência.
- Confirmar funcionamento básico no TTGO T-Display ESP32.

### Fase 2 - Produto mínimo

- Organizar menu local.
- Implementar feedback visual mais claro.
- Adicionar reconexão automática.
- Documentar instalação e configuração.

### Fase 3 - Evolução

- Melhorar usabilidade da interface.
- Adicionar mais estados da câmera, se suportado.
- Avaliar suporte a perfis ou múltiplos dispositivos.

## 15. Observações técnicas

O projeto original foi desenvolvido com foco em dispositivos M5StickC e similares. Para o TTGO T-Display ESP32, a adaptação deverá considerar diferenças de hardware, mapeamento de botões, display e eventuais bibliotecas da placa.

Se necessário, a documentação final deve incluir instruções de compilação, gravação, alteração do identificador remoto e processo de pareamento com a Insta360 X5.

