#include "shieldInitConfig.h"

void readAnalogs(){
  
  //analogicos variam de 0 a 67
  current_x = analogRead(X)/ 10;
  current_y = analogRead(Y)/ 10;
  
}

void readButtons(){
  
  current_btn_a = !digitalRead(A);
  current_btn_b = !digitalRead(B);
  current_btn_c = !digitalRead(C);
  current_btn_d = !digitalRead(D);
  current_btn_e = !digitalRead(E);
  current_btn_f = !digitalRead(F);
  current_btn_k = !digitalRead(K);
    
  }

void setup() 
{
  pinMode(X, INPUT);
  pinMode(Y, INPUT);
  
  for(int i = 2; i <= 8; i++) {
    pinMode(i, INPUT_PULLUP);
 
  //logs
  Serial.begin(115200);
    }
}

void loop() {

  
  readAnalogs();
  readButtons();

  //no liguem pros IFs, depois a eu otimizo o código ksksksksó
  
  // O "abs(diferença)" é pra evitar ruído do analógico
  if (abs(current_x - last_x) > 4) {
    Serial.print("X:");
    Serial.println(current_x);
    last_x = current_x; 
  }

  if (abs(current_y - last_y) > 4) {
    Serial.print("Y:");
    Serial.println(current_y);
    last_y = current_y;
  }
  
// Envia 1 para pressionado, 0 para solto
  if (current_btn_a != last_btn_a) {
    Serial.print("A:");
    Serial.println(current_btn_a); 
    last_btn_a = current_btn_a;
  }
  
  if (current_btn_b != last_btn_b) {
    Serial.print("B:");
    Serial.println(current_btn_b);
    last_btn_b = current_btn_b;
  }

  if (current_btn_c != last_btn_c) {
    Serial.print("C:");
    Serial.println(current_btn_c);
    last_btn_c = current_btn_c;
  }

  if (current_btn_d != last_btn_d) {
    Serial.print("D:");
    Serial.println(current_btn_d);
    last_btn_d = current_btn_d;
  }
  
  if (current_btn_e != last_btn_e) {
    Serial.print("E:");
    Serial.println(current_btn_e);
    last_btn_e = current_btn_e;
  }

  if (current_btn_f != last_btn_f) {
    Serial.print("F:");
    Serial.println(current_btn_f);
    last_btn_f = current_btn_f;
  }

  if (current_btn_k != last_btn_k) {
    Serial.print("K:");
    Serial.println(current_btn_k);
    last_btn_k = current_btn_k;
  }
  delay(10); 
}
