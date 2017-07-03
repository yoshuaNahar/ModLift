#include<Wire.h>

// Setup stepper
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

// For stepper
int steps = 0;
boolean clockwise = true;
int turnMotor = 0;

// For I2C
const int CONNECTED_SLAVES = 1;
int currentSlave = 0;           // begane grond is verdieping 0

void setup() {
  Wire.begin();

  Serial.begin(9600);

  // Setup stepper
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  readSerialAndSendToFloor(currentSlave);
  getButtonPressedOfFloor(currentSlave); // De delay mogelijk verwijderen om de motor 
  // nog soepeler te laten lopen, of the for loop in tunrMotorOnRequest verlengen

  turnMotorOnRequest();

  loopThroughAllFloors();
}

/*********************** I2C CODE ***********************/

void readSerialAndSendToFloor(int target) {
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

void getButtonPressedOfFloor(int target) {
  Wire.requestFrom(target, 1);
  delay(5);
  while (Wire.available()) {
    int i = Wire.read();
    Serial.println(i);
    turnMotor = i;
  }
}

/*********************** NON I2C CODE ***********************/

void loopThroughAllFloors() {
  currentSlave++;

  if (currentSlave < CONNECTED_SLAVES) {
    return;
  } else {
    currentSlave = 0;
  }
}

void turnMotorOnRequest() {
  if (turnMotor) {
    for (int i = 0; i < 10; i++) { // remove or increase when needed,
    // with this for loop the motor moves more between requests.
      stepper();
      delayMicroseconds(800); // 800 is min, online 2000 is mostly used
    }
  }
}

// http://www.nmbtc.com/step-motors/engineering/full-half-and-microstepping/
// Using two-phase fullstep
void stepper() {
  switch (steps) {
  case 0:
    digitalWrite(IN1, HIGH);
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

