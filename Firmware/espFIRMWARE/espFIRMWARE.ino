
#define TX_PIN      5
#define CLK_PIN     4
#define SYNC_PIN    2


uint16_t dataToWrite = 0;
uint16_t ButonStatus = 0;

const int PIN_UP      = 36;
const int PIN_RIGHT   = 39;
const int PIN_DOWN    = 34;
const int PIN_LEFT    = 35;
const int PIN_START   = 32;
const int PIN_SELECT  = 33;
const int PIN_ANALOGB = 25;

const int PIN_AXIS_X = 26;
const int PIN_AXIS_Y = 27;

const uint16_t DEBOUNCE_MS = 25;
const int AXIS_CENTER_VAL_X = 333;
const int AXIS_CENTER_VAL_Y = 330;
const int AXIS_THRESHOLD  = 300;

struct Button {
  const char* name;
  uint8_t pin;
  bool hasInternalPullup;
  int lastStable;
  int lastRead;
  uint32_t lastChangeMs;
};

Button buttons[] = {
  {"UP",      PIN_UP,      false, HIGH, HIGH, 0},
  {"RIGHT",   PIN_RIGHT,   false, HIGH, HIGH, 0},
  {"DOWN",    PIN_DOWN,    false, HIGH, HIGH, 0},
  {"LEFT",    PIN_LEFT,    false, HIGH, HIGH, 0},
  {"START",   PIN_START,   true,  HIGH, HIGH, 0},
  {"SELECT",  PIN_SELECT,  true,  HIGH, HIGH, 0},
  {"ANALOG",  PIN_ANALOGB, true,  HIGH, HIGH, 0},
};
const size_t N_BUTTONS = sizeof(buttons)/sizeof(buttons[0]);
int lastStates[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setupPinModes() {
  // Configura os pinos digitais dos botões
  for (size_t i = 0; i < N_BUTTONS; ++i) {
    if (buttons[i].hasInternalPullup) {
      pinMode(buttons[i].pin, INPUT_PULLUP);
    } else {
      pinMode(buttons[i].pin, INPUT);
    }
  }
  pinMode(CLK_PIN, OUTPUT);
  pinMode(TX_PIN,OUTPUT);
  pinMode(SYNC_PIN,INPUT);
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(TX_PIN, LOW);
}

void setup() {
  Serial.begin(9600);
  delay(200);
  setupPinModes();
  Serial.println("Joystick ESP32 iniciado.");
  // Adicionar cabeçalho para facilitar a leitura dos dados seriais
  Serial.println("UP,RIGHT,DOWN,LEFT,START,SELECT,ANALOG,DPAD_UP,DPAD_DOWN,DPAD_LEFT,DPAD_RIGHT");
}

void readButtons(int states[]) {
  const uint32_t now = millis();

  for (size_t i = 0; i < N_BUTTONS; ++i) {
    Button &b = buttons[i];
    int raw = digitalRead(b.pin);

    if (raw != b.lastRead) {
      b.lastRead = raw;
      b.lastChangeMs = now;
    }

    if ((now - b.lastChangeMs) >= DEBOUNCE_MS) {
      b.lastStable = raw;
    }

    states[i] = (b.lastStable == LOW) ? 1 : 0;
  }
}

void readDPadFromAnalog(int dpadStates[]) {
  int x = analogRead(PIN_AXIS_X);
  int y = analogRead(PIN_AXIS_Y);

  int xDelta = x - AXIS_CENTER_VAL_X;
  int yDelta = y - AXIS_CENTER_VAL_Y;

  bool up_active = false;
  bool down_active = false;
  bool left_active = false;
  bool right_active = false;

  if (yDelta < -AXIS_THRESHOLD) {
    up_active = true;
  } else if (yDelta > AXIS_THRESHOLD) {
    down_active = true;
  }

  if (xDelta < -AXIS_THRESHOLD) {
    left_active = true;
  } else if (xDelta > AXIS_THRESHOLD) {
    right_active = true;
  }

  dpadStates[0] = up_active ? 1 : 0;
  dpadStates[1] = down_active ? 1 : 0;
  dpadStates[2] = left_active ? 1 : 0;
  dpadStates[3] = right_active ? 1 : 0;
}

void readAllStates(int allStates[]) {
  // Lê os estados dos botões digitais
  readButtons(allStates); 
  
  // Lê os estados do D-Pad
  int dpadStates[4]; // [UP, DOWN, LEFT, RIGHT]
  readDPadFromAnalog(dpadStates);

  // Consolida os estados do D-Pad no array
  for (int i = 0; i < 4; ++i) {
    allStates[7 + i] = dpadStates[i];  // Posições 7, 8, 9, 10 no array
  }
}

#define BAUD_RATE   9600 // Exemplo de taxa. Escolha a menor possível para maior estabilidade.

// Calcula o tempo de um bit em microsegundos
const int BIT_TIME_US = (1000000 / BAUD_RATE);

// Máscara de Registrador para operação atômica
const uint32_t TX_MASK = (1UL << TX_PIN);

// Função auxiliar para atraso preciso (melhor que delayMicroseconds())
// ets_delay_us é uma função de IRAM do ESP-IDF/Arduino-ESP32
#define bit_delay() ets_delay_us(BIT_TIME_US)

// Garante que a função rode na RAM de Instruções para latência mínima
void IRAM_ATTR write_2_bytes(uint16_t data) {
    static int lastSync = 0;
    int syncBit = digitalRead(SYNC_PIN);
    for (int i = 0; i < 16; i++) {
        while(syncBit == lastSync) {
          bit_delay();
          syncBit = digitalRead(SYNC_PIN);
        }
        lastSync = syncBit;
        bit_delay();
        digitalWrite(CLK_PIN,LOW);
        if (data & (1 << i)) {
            // Bit é 1: Set (HIGH)
            digitalWrite(TX_PIN,HIGH);
            Serial.print("1");
        } else {
            // Bit é 0: Clear (LOW)
            digitalWrite(TX_PIN,LOW);
            Serial.print("0");
        }
        bit_delay();
        digitalWrite(CLK_PIN,HIGH);
    }
    Serial.println();
}

void sendJoystickData(int allStates[]) {
  dataToWrite = 0;
  for (int i = 0; i < 11; i++) {
    if (allStates[i]) {
      dataToWrite |= (1 << i);
    }
  }
  write_2_bytes(dataToWrite);
}

void loop() {
  int allStates[11]; // 7 botões + 4 D-Pad
  
  // 1. Lê todos os estados atuais
  readAllStates(allStates); 

  // 2. Verifica se algo mudou
  bool stateChanged = false;
  for (int i = 0; i < 11; i++) {
    if (allStates[i] != lastStates[i]) {
      stateChanged = true;
      break; // Encontrou uma mudança, não precisa checar o resto
    }
  }

  // 3. Só envia se o estado mudou
  if (stateChanged) {
    // Envia os novos dados
    sendJoystickData(allStates); 
    
    // Atualiza o "último estado" com os dados que acabamos de enviar
    for (int i = 0; i < 11; i++) {
      lastStates[i] = allStates[i];
    }
  } 
  delay(50); 
}
