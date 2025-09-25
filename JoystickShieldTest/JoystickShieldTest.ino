
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
