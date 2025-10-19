# JoystickRaspberry-HandsOn-DevTitans

# Joystick Raspberry

Bem-vindo ao repositório da Equipe 02 do HandsON do DevTITANS! Este projeto contém um firmware para o ESP32 escrito em formato Arduino `.ino`, bem como um driver do kernel Linux escrito em C. O objetivo é demonstrar como criar uma solução completa de hardware e software que integra um dispositivo ESP32 com um sistema Linux.

## Tabela de Conteúdos

- [Conceito](#introdução)
- [Contribuidores](#contribuidores)
- [Recursos](#recursos)
- [Requisitos](#requisitos)
- [Configuração de Hardware](#configuração-de-hardware)
- [Instalação](#instalação)
- [Uso](#uso)
- [Contato](#contato)

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

## Contribuidores

<img width="180" alt="Image" src="https://github.com/user-attachments/assets/cd796644-124d-4f5d-86b6-0a07714f4eb5" />
<img width="180" alt="Image" src="https://github.com/user-attachments/assets/2f1e9279-9ed3-4f34-8fe9-330a9388f79f" />
<img width="180" alt="Image" src="https://github.com/user-attachments/assets/312858b6-3417-4fa8-949f-00bbd2e49d95" />
<img width="180" alt="Image" src="https://github.com/user-attachments/assets/0ace7fa6-2203-4dea-a51b-4a35c326c3aa" />
<img width="180" alt="Image" src="https://github.com/user-attachments/assets/59bc0193-08d4-4aec-914f-1428d8985642" />
<img width="180" alt="Image" src="https://github.com/user-attachments/assets/98943483-134f-4db8-89c8-ebe8bf0a6b99" />
<img src="https://github.com/DevTITANS05/Hands-On-Linux-fork-/assets/21023906/85e61f3e-476c-47a4-82d5-4054e856c67b" width="180" >

- **Nome do(a) Aluno(a) 01:** Desenvolvedor do Firmware e Mantenedor do Projeto
- **Nome do(a) Aluno(a) 02:** Desenvolvedor do Firmware
- **Nome do(a) Aluno(a) 03:** Desenvolvedor do Driver Linux
- **Nome do(a) Aluno(a) 04:** Desenvolvedor do Driver Linux
- **Nome do(a) Aluno(a) 05:** Desenvolvedor do Firmware e Escritor da Documentação

## Introdução

Este projeto serve como um exemplo para desenvolvedores interessados em construir e integrar soluções de hardware personalizadas com sistemas Linux. Inclui os seguintes componentes:
- Firmware para o microcontrolador ESP32 para lidar com operações específicas do dispositivo.
- Um driver do kernel Linux que se comunica com o dispositivo ESP32, permitindo troca de dados e controle.

## Recursos

- **Firmware ESP32:**
  - Aquisição básica de dados de sensores.
  - Comunicação via Serial com o driver Linux.
  
- **Driver do Kernel Linux:**
  - Rotinas de inicialização e limpeza.
  - Operações de arquivo de dispositivo (`GET_LED`, `SET_LED`, `GET_LDR`).
  - Comunicação com o ESP32 via Serial.

## Requisitos

- **Hardware:**
  - Placa de Desenvolvimento ESP32
  - Máquina Linux
  - Protoboard e Cabos Jumper
  - Sensor LDR
  
- **Software:**
  - Arduino IDE
  - Kernel Linux 4.0 ou superior
  - GCC 4.8 ou superior
  - Make 3.81 ou superior

## Configuração de Hardware

1. **Conecte o ESP32 à sua Máquina Linux:**
    - Use um cabo USB.
    - Conecte os sensores ao ESP32 conforme especificado no firmware.

2. **Garanta a alimentação e conexões adequadas:**
    - Use um protoboard e cabos jumper para montar o circuito.
    - Consulte o diagrama esquemático fornecido no diretório `esp32` para conexões detalhadas.

## Instalação

### Firmware ESP32

1. **Abra o Arduino IDE e carregue o firmware:**
    ```sh
    Arquivo -> Abrir -> Selecione `smartlamp.ino`
    ```

2. **Configure a Placa e a Porta:**
    ```sh
    Ferramentas -> Placa -> Node32s
    Ferramentas -> Porta -> Selecione a porta apropriada
    ```

3. **Carregue o Firmware:**
    ```sh
    Sketch -> Upload (Ctrl+U)
    ```

### Driver Linux

1. **Clone o Repositório:**
    ```sh
    git clone https://github.com/seuusuario/Hands-On-Linux.git
    cd Hands-On-Linux
    ```

2. **Compile o Driver:**
    ```sh
    cd smartlamp-kernel-module
    make
    ```

3. **Carregue o Driver:**
    ```sh
    sudo insmod smartlamp.ko
    ```

4. **Verifique o Driver:**
    ```sh
    dmesg | tail
    ```

## Uso

Depois que o driver e o firmware estiverem configurados, você poderá interagir com o dispositivo ESP32 através do sistema Linux.

- **Escrever para o Dispositivo:**
    ```sh
    echo "100" > /sys/kernel/smartlamp/led
    ```

- **Ler do Dispositivo:**
    ```sh
    cat /sys/kernel/smartlamp/led
    ```

- **Verificar Mensagens do Driver:**
    ```sh
    dmesg | tail
    ```

- **Remover o Driver:**
    ```sh
    sudo rmmod smartlamp
    ```
    
## Contato

Para perguntas, sugestões ou feedback, entre em contato com o mantenedor do projeto em [maintainer@example.com](mailto:maintainer@example.com).
