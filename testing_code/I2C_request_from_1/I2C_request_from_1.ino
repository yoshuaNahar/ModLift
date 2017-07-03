#include<Wire.h>

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
    if(Serial.available()){
      while(Serial.available()){
        Serial.read();
      }
      Serial.println("Requesting");
      Wire.requestFrom(1, 1);
    }
    while(Wire.available()){
      int i = Wire.read();
      Serial.println(i);
  }
}

