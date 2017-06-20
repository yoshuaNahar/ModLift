#include<Wire.h>

#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

// Variables Stepper
int steps = 0;
boolean clockwise = true;

int turnMotor = 0;

// Variables for I2C
const int CONNECTED_SLAVES = 1;
int currentSlave = 1;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  // Setup Stepper
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  // in de oude code was er hier een for loop, om voor elke slave een check te doen
//  readUsbAndSend(currentSlave);
//  requestButtonState(currentSlave); // Misschien ook iets doen aan die delay die in deze functie zit!

  // Stepper loop code
  if (turnMotor) {
    for (int i = 0; i < 10; i++) {
      stepper();
      delayMicroseconds(800); // dit lager setten totdat om de motor sneller te laten draaien totdat het niet meer draait
    }
  }

  if (currentSlave >= CONNECTED_SLAVES) {
    currentSlave = 1;
  } else {
    currentSlave++;
  }
}

void readUsbAndSend(int target) {
  while (Serial.available()) {
    String s = Serial.readString();
    int i1 = s.substring(0,1).toInt();
    int i2 = s.substring(1,2).toInt();
    int i3 = s.substring(2).toInt();
    Serial.print("Sending ");
    Serial.print(i1);
    Serial.print(" and ");
    Serial.print(i2);
    Serial.print(" and ");
    Serial.println(i3);
    Wire.beginTransmission(target);
    Wire.write(i1);
    Wire.write(i2);
    Wire.write(i3);
    Wire.endTransmission();
  }
}

void requestButtonState(int target) {
  Wire.requestFrom(target, 1);
  delay(5);
  while (Wire.available()) {
    int i = Wire.read();
    Serial.println(i);
    turnMotor = i;
  }
}

void stepper() {
  switch (steps) {
    case 0:
      digitalWrite(IN1, HIGH); // werkt dit niet, dan misschien HIGH, LOW, HIGH, LOW proberen...
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 1:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      break;
    case 2:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, HIGH);
      break;
    case 3:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      break;
    default:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
  }

  if (clockwise) {
    steps++;
  } else {
    steps--;
  }

  if (steps > 3) {
    steps = 0;
  } else if (steps < 0) {
    steps = 3;
  }
}

