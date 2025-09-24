# JoystickRaspberry-HandsOn-DevTitans

# oystick Raspberry

**Nível de Dificuldade:** 8/10

### Conceito

Desenvolver um driver de Kernel do zero para integrar um protótipo de joystick físico ao Android (AOSP) rodando em um Raspberry Pi 4B. O projeto foca no desenvolvimento de baixo nível, transformando sinais elétricos de pinos GPIO em eventos de input padrão do Android, fazendo com que o sistema reconheça o protótipo como um gamepad nativo.

### Funcionalidade Principal

- **Hardware do Joystick:** Um protótipo de joystick será construído utilizando um ESP32 para ler o estado de botões físicos.
- **Comunicação via GPIO:** O ESP32 se comunicará com o Raspberry Pi 4B diretamente através das portas GPIO. Cada botão pressionado no joystick resultará na alteração do estado de um pino GPIO correspondente.
- **Driver de Kernel:** O núcleo do projeto é a implementação de um driver de dispositivo de entrada para o Kernel Linux (AOSP). O próprio driver será responsável por todo o trabalho: ele irá monitorar os pinos GPIO, detectar as mudanças de estado e gerar os eventos de botão (ex: `BTN_A`, `BTN_B`) no formato padrão que o Android entende nativamente.
- **Integração Nativa:** Como o driver cria os eventos de forma padronizada, o Android InputFlinger reconhecerá o dispositivo automaticamente assim que o driver for carregado, permitindo que o joystick seja usado em qualquer aplicativo ou jogo compatível.

### Equipamentos Necessários

- Raspberry Pi 4B (com AOSP instalado)
- Protótipo de Joystick (ESP32, botões, protoboard, fios)

### Camadas do AOSP Envolvidas

- **Kernel:** Esta é a única camada de software a ser modificada. Todo o esforço será concentrado em escrever um módulo de Kernel completo que lida com a interação com o hardware (GPIO) e com a comunicação com o restante do sistema (subsistema de input), servindo como a única ponte entre o circuito físico e o sistema operacional Android.

### Divisão de Tarefas (Equipe de 5)

1. **Engenheiro de Hardware (Projeto de Circuito):**
    - **Responsabilidade:** Projetar e montar o circuito físico do joystick.
    - **Tarefa:** Desenhar o esquemático do joystick, selecionar os botões e outros componentes eletrônicos, e montar o protótipo físico, garantindo que as conexões entre os botões e o ESP32 sejam robustas e funcionais.
2. **Engenheiro de Firmware (ESP32):**
    - **Responsabilidade:** Programar o microcontrolador para ser a "mente" do joystick.
    - **Tarefa:** Desenvolver o firmware para o ESP32. O código deverá continuamente escanear o estado de todos os botões e, ao detectar uma mudança (pressionar/soltar), alterar o estado do pino de saída correspondente que se conecta ao Raspberry Pi.
3. **Desenvolvedor de Driver de Kernel (Estrutura e GPIO):**
    - **Responsabilidade:** Criar a base do driver e a lógica de interação com o hardware no Raspberry Pi.
    - **Tarefa:** Escrever a estrutura inicial de um módulo de Kernel para Linux. Implementar a lógica para requisitar, configurar e ler o estado dos pinos GPIO do Raspberry Pi que estão conectados ao joystick, detectando as mudanças de sinal enviadas pelo ESP32.
4. **Desenvolvedor de Driver de Kernel (Subsistema de Input):**
    - **Responsabilidade:** Traduzir os sinais de hardware em comandos de joystick para o Android.
    - **Tarefa:** Implementar a parte crucial do driver. Este desenvolvedor irá registrar um novo dispositivo de entrada (`input_device`) no Kernel. A lógica implementada aqui irá pegar os sinais brutos dos GPIOs e convertê-los em eventos de input padronizados do Linux (ex: mapear o pino GPIO 17 para `BTN_SOUTH`), utilizando a API do subsistema de input.
5. **Engenheiro de Integração e Build Master:**
    - **Responsabilidade:** Gerenciar os ambientes de desenvolvimento, compilar o sistema e integrar o driver.
    - **Tarefa:** Manter dois ambientes de trabalho: um **Linux (ex: Ubuntu)** para facilitar e agilizar o desenvolvimento e compilação inicial do driver, e o **ambiente completo do AOSP**. Inicialmente, irá compilar e instalar o AOSP no Raspberry Pi. Durante o desenvolvimento, será responsável por pegar o driver compilado no ambiente Linux e fazer a instalação manual no AOSP para testes. Na versão final da solução, sua tarefa será integrar o código-fonte do driver à árvore de fontes do AOSP, garantindo que ele seja compilado junto com o Kernel do sistema para a entrega final.
