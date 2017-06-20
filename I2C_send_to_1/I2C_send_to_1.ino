#include<Wire.h>

void setup() {
 // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.available()){
    String s = Serial.readString();
    int i1 = s.substring(0,1).toInt();
    int i2 = s.substring(1).toInt();
    Serial.print("Sending ");
    Serial.print(i1);
    Serial.print(" and ");
    Serial.println(i2);
    Wire.beginTransmission(1);
    Wire.write(i1);
    Wire.write(i2);
    Wire.endTransmission();
  }
}
