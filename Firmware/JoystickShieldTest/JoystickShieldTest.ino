//ESP PINS


//#define TX_PIN      5
//#define CLK_PIN     4
//#define SYNC_PIN    2
//
//
//uint16_t dataToWrite = 0;
//uint16_t ButonStatus = 0;
//
//const int PIN_UP      = 36;
//const int PIN_RIGHT   = 39;
//const int PIN_DOWN    = 34;
//const int PIN_LEFT    = 35;
//const int PIN_START   = 32;
//const int PIN_SELECT  = 33;
//const int PIN_ANALOGB = 25;
//
//const int PIN_AXIS_X = 26;
//const int PIN_AXIS_Y = 27;

//ARDUINO PINS

#define   X    A0  //Analogico Horizontal
#define   Y    A1  //Analogico Vertical
#define   K    8
#define   F    7
#define   E    6
#define   D    5
#define   C    4
#define   B    3
#define   A    2


void setup() 
{
  for(int i=2; i<9; i++) pinMode(i, INPUT_PULLUP);
  
  pinMode(X, INPUT);
  pinMode(Y, INPUT);

  Serial.begin(115200);
}

void loop() {
  int x_read = analogRead(X);
  int y_read = analogRead(Y);

  Serial.print    ("X: ");
  Serial.println  ("Y: ");
  Serial.print    (x_read);
  Serial.print    (" ");
  Serial.println  (y_read);

if         (!digitalRead(K)) Serial.println("K press");
else if    (!digitalRead(F)) Serial.println("F press");
else if    (!digitalRead(E)) Serial.println("E press");  
else if    (!digitalRead(D)) Serial.println("D press");
else if    (!digitalRead(C)) Serial.println("C press");
else if    (!digitalRead(B)) Serial.println("B press");
else if    (!digitalRead(A)) Serial.println("A press");
else                         Serial.println("NOTHING press");


delay(741);
}
