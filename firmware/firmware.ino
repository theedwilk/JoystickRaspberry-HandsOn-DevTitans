
#define B_BUTTON_MASK 1
#define A_BUTTON_MASK 1 << 1
#define Y_BUTTON_MASK 1 << 2
#define X_BUTTON_MASK 1 << 3
#define SEL_BUTTON_MASK 1 << 4
#define ST_BUTTON_MASK 1 << 5
#define DOWN_BUTTON_MASK 1 << 6
#define RIGHT_BUTTON_MASK 1 << 7
#define UP_BUTTON_MASK 1 << 8
#define LEFT_BUTTON_MASK 1 << 9


#define TX_PIN      5
#define CLK_PIN     4
#define SYNC_PIN    2


uint16_t dataToWrite = 0;
uint16_t ButonStatus = 0;

const int PIN_BUTTON_B   = 26;
const int PIN_BUTTON_A    = 25;
const int PIN_BUTTON_Y      = 27;
const int PIN_BUTTON_X    = 33;
const int PIN_BUTTON_SE  = 35; //SELECT
const int PIN_BUTTON_ST   = 32; //START

const int PIN_AXIS_X = 34;
const int PIN_AXIS_Y = 39 ;

const uint16_t DEBOUNCE_MS = 25;
const int AXIS_CENTER_VAL_X = 1755;
const int AXIS_CENTER_VAL_Y = 1749;
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
  {"A",      PIN_BUTTON_A,      false, HIGH, HIGH, 0},
  {"B",    PIN_BUTTON_B,    false, HIGH, HIGH, 0},
  {"X",    PIN_BUTTON_X,    false, HIGH, HIGH, 0},
  {"Y",   PIN_BUTTON_Y,   false, HIGH, HIGH, 0},
  {"SE",   PIN_BUTTON_SE,   true,  HIGH, HIGH, 0},
  {"ST",  PIN_BUTTON_ST,  true,  HIGH, HIGH, 0},
};
const size_t N_BUTTONS = sizeof(buttons)/sizeof(buttons[0]);

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
}

void readButtons() {
  const uint32_t now = millis();

  for (size_t i = 0; i < N_BUTTONS; i++) {
    Button &b = buttons[i];
    int raw = digitalRead(b.pin);

    if (raw != b.lastRead) {
      b.lastRead = raw;
      b.lastChangeMs = now;
    }

    if ((now - b.lastChangeMs) >= DEBOUNCE_MS) {
      b.lastStable = raw;
    }

    if (b.lastStable == HIGH) {
      dataToWrite |= 1 << i;
    }
  }
}

void readDPadFromAnalog() {
  int x = analogRead(PIN_AXIS_X);
  int y = analogRead(PIN_AXIS_Y);
  int xDelta = x - AXIS_CENTER_VAL_X;
  int yDelta = y - AXIS_CENTER_VAL_Y;

  if (yDelta < -AXIS_THRESHOLD) {
    dataToWrite |= UP_BUTTON_MASK;
  } else if (yDelta > AXIS_THRESHOLD) {
    dataToWrite |= DOWN_BUTTON_MASK;
  }

  if (xDelta < -AXIS_THRESHOLD) {
    dataToWrite |= RIGHT_BUTTON_MASK;
  } else if (xDelta > AXIS_THRESHOLD) {
    dataToWrite |= LEFT_BUTTON_MASK;
  }
}

void readAllStates() {
  dataToWrite = 0;
  // Lê os estados dos botões digitais
  readButtons(); 
  
  // Lê os estados do D-Pad
  readDPadFromAnalog();
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

void sendJoystickData() {
  static uint16_t lastSent = 0;
  if (lastSent != dataToWrite) {
    lastSent = dataToWrite;
    write_2_bytes(dataToWrite);
  }
}

void loop() {
  // Lê todos os estados (botões + D-Pad)
  readAllStates(); 

  // Envia os dados através da serial
  sendJoystickData(); 
   
  delay(50); 
}
