#include <Arduino.h>

// ------------ Mapeamento de pinos ------------
const int PIN_UP      = 36; 
const int PIN_RIGHT   = 39;
const int PIN_DOWN    = 34;
const int PIN_LEFT    = 35;
const int PIN_START   = 32;
const int PIN_SELECT  = 33;
const int PIN_ANALOGB = 25;

const int PIN_X_AXIS  = 26;  
const int PIN_Y_AXIS  = 27;  

// ------------ Configuração de debounce ------------
const uint16_t DEBOUNCE_MS = 25;

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

// Eixos
int lastX = -1, lastY = -1;
const int AXIS_EPS = 30;         
bool xMoving = false;
bool yMoving = false;

void setupPinModes() {
  for (size_t i = 0; i < N_BUTTONS; ++i) {
    if (buttons[i].hasInternalPullup) {
      pinMode(buttons[i].pin, INPUT_PULLUP);
    } else {
      pinMode(buttons[i].pin, INPUT);
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(200);
  setupPinModes();
  Serial.println("Joystick ESP32 iniciado.");
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

    // ativo em 0 → pressionado = 1, solto = 0
    states[i] = (b.lastStable == LOW) ? 1 : 0;
  }
}

void readAxes(int &xState, int &yState) {
  int x = analogRead(PIN_X_AXIS);
  int y = analogRead(PIN_Y_AXIS);

  // X
  if (lastX >= 0) {
    if (abs(x - lastX) > AXIS_EPS) {
      xMoving = true;
    } else {
      xMoving = false;
    }
  }
  lastX = x;
  xState = xMoving ? 1 : 0;

  // Y
  if (lastY >= 0) {
    if (abs(y - lastY) > AXIS_EPS) {
      yMoving = true;
    } else {
      yMoving = false;
    }
  }
  lastY = y;
  yState = yMoving ? 1 : 0;
}

void loop() {
  int states[N_BUTTONS];
  int xAxis, yAxis;

  readButtons(states);
  readAxes(xAxis, yAxis);

  // imprime todos os estados separados por vírgula
  //UP,RIGHT,DOWN,LEFT,START,SELECT,ANALOG,X-AXIS,Y-AXIS
  for (size_t i = 0; i < N_BUTTONS; ++i) {
    Serial.print(states[i]);
    Serial.print(",");
  }
  Serial.print(xAxis);
  Serial.print(",");
  Serial.println(yAxis);

  delay(50); // ajusta a taxa de atualização
}
