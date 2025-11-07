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

- **Alex Monteiro:** Desenvolvedor do Hardware
- **Jefferson Lima:** Desenvolvedor do Firmware
- **Waldomiro Seabra:** Desenvolvedor do Driver Linux
- **Bruno Solimôes:** Desenvolvedor do Aosp Product
- **Stella Silva:** Desenvolvedor do Aosp e Escritora da Documentação
- **Theed Wilk:** Desenvolvedor e Escritor da Documentação

## Introdução

O projeto **Joystick Raspberry — HandsOn DevTitans** propõe a integração direta entre um **joystick físico controlado por um ESP32** e o **Android Open Source Project (AOSP)** executando em um **Raspberry Pi 4B**.  

O sistema foi desenvolvido do zero, abrangendo desde o **firmware do ESP32**, responsável pela leitura dos botões e eixos, até o **driver de kernel Linux**, que traduz os sinais elétricos em **eventos de input reconhecidos pelo Android**.

A comunicação entre os dispositivos é realizada via **GPIO** utilizando um **protocolo síncrono de 16 bits** com três sinais principais (`TX`, `CLK`, `SYNC`).

> O resultado final é um joystick funcional detectado automaticamente pelo Android, sem necessidade de aplicativos intermediários.

---

## Recursos

###  Firmware (ESP32)
- Leitura de **7 botões digitais** e **2 eixos analógicos (X/Y)**.  
- Aplicação de **debounce por software** para eliminar ruído elétrico.  
- Geração de **D-Pad digital** a partir dos eixos analógicos.  
- Envio de **pacote de 16 bits via GPIO (TX/CLK/SYNC)**.  
- Compatível com a arquitetura ESP32 Node32s.  

###  Driver de Kernel (Linux)
- Implementado em C como **módulo de kernel (`joy_driver_module.c`)**.  
- Leitura dos bits via **interrupção no pino de clock (IRQ)**.  
- Conversão automática em eventos `EV_KEY` e `EV_SYN`.  
- Registro no **Linux Input Subsystem**, reconhecido pelo Android InputFlinger.  

###  Integração com o AOSP
- Compilação completa do **Android 16.0** (AOSP) para **Raspberry Pi 4B**.  
- Inclusão do driver `.ko` no kernel (`common-modules/virtual-device`).  
- Reconhecimento automático do joystick na camada Android.  

---

## Requisitos

###  Hardware
| Componente | Função |
|-------------|--------|
| Raspberry Pi 4B | Executa o AOSP e o driver de kernel |
| ESP32 Node32s | Controla os botões e envia os dados via GPIO |
| Joystick Shield / Botões físicos | Interface do usuário |
| Protoboard e jumpers | Conexões elétricas |
| Cartão microSD (mín. 32 GB) | Armazenamento da imagem Android |
| Fonte de 5V / 3A | Alimentação estável |

###  Software
| Software | Versão Recomendada |
|-----------|--------------------|
| Ubuntu 22.04 LTS | Ambiente de compilação |
| Arduino IDE | 2.0+ |
| Toolchain AOSP | Android 16.0 / RPi4 |
| Make / GCC | 4.8+ |
| Git e Repo Tools | Última versão |

---

## Configuração de Hardware

Conexões entre **ESP32** e **Raspberry Pi 4B**:

| Sinal (ESP32) | Pino (GPIO) no Raspberry | Direção | Função |
|----------------|--------------------------|----------|--------|
| TX_PIN (5) | DATA_GPIO (229) | ESP32 → RPi | Transmissão de dados bit a bit |
| CLK_PIN (4) | CLK_GPIO (230) | ESP32 → RPi | Pulso de clock |
| SYNC_PIN (2) | SYNC_GPIO (228) | RPi → ESP32 | Sinal de sincronização |

> Os botões e eixos analógicos são conectados diretamente aos pinos de entrada do ESP32 (36–39, 32–35, 25–27).  
> Consulte o arquivo `firmware/firmware.ino` para o mapeamento completo.

---

## Instalação

### Firmware no ESP32

1. Abra o **Arduino IDE** e carregue `firmware/firmware.ino`.  
2. Configure a placa e porta:

Ferramentas → Placa → Node32s
Ferramentas → Porta → /dev/ttyUSB0 

3. Compile e envie:
Sketch → Upload (Ctrl + U)

O ESP32 exibirá no Serial Monitor:
Joystick ESP32 iniciado.
UP,RIGHT,DOWN,LEFT,START,SELECT,ANALOG,DPAD_UP,DPAD_DOWN,DPAD_LEFT,DPAD_RIGHT


---

### Compilação do Driver no Kernel

1. Copie o arquivo `driver/joy_driver_module.c` para o diretório de módulos do AOSP:
```
~/kernel-aosprasp/common-modules/virtual-device/nesjoy/
```
---

2. Edite `BUILD.bazel` e adicione:

```
ddk_module(
    name = "aarch64/nesjoy",
    srcs = [":nesjoy"],
    out = "nesjoy.ko",
    kernel_build = ":virtual_device_aarch64",
    deps = [":common_headers"],
)
```

---

3. Compile o kernel:
```
cd ~/kernel-aosprasp
time tools/bazel run //common-modules/virtual-device:virtual_device_aarch64_dist
```
---

4. Copie o .ko para o AOSP:
```
cp out/virtual-device_aarch64/dist/nesjoy.ko ~/aosp-raspberry/kernel/nesjoy/
```
---

### Recompilando o AOSP com o Driver

1. Configure o ambienteTeste os eventos:Teste os eventos:
2. Compile o sistema
3. Grave a imagem gerada no cartão SD usando o Raspberry Pi Imager.

## Uso

Após o boot do Android no Raspberry Pi:

```
insmod /vendor/lib/modules/nesjoy.ko
```

Verifique o registro do dispositivo:

```
dmesg | grep nesjoy
```

O joystick será reconhecido automaticamente pelo Android:

```
/dev/input/js0
```

Teste os eventos:

```
getevent -lt /dev/input/event*
```
