#include<Wire.h>

// Setup stepper
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

// For stepper
int steps = 0;
boolean clockwise = true;

// For I2C and lift
const int CONNECTED_SLAVES = 2;
int floorButtonUp[CONNECTED_SLAVES];
int floorButtonDown[CONNECTED_SLAVES];
boolean doorOpen[CONNECTED_SLAVES];           // If Door on floor is open, also reset floor get lift buttons
boolean liftArrived[CONNECTED_SLAVES];

int currentFloor = 0;
boolean movingUp = true;                      // which direction the elevator is going
boolean moveUp = false;                       // Should the moter move clockwise
boolean moveDown = false;                     // Should the motor move counter-clockwise

void setup() {
  Serial.begin(9600);

  Wire.begin();

  // Setup stepper
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // why does this only happen for doorOpen and not floorButtonUp and floorButtonDown
  for (int i = 0; i < CONNECTED_SLAVES; i++) {
    doorOpen[i] = false;
  }
}

void loop() {
  Serial.println("getAndSendDataToAllFloors");
  getAndSendDataToAllFloors();
  Serial.println("debugArray");
  debugArray();
  Serial.println("checkForMoveLift");
  checkForMoveLift();
  Serial.println("moveLift");
  moveLift();
  delay(100);
  //Serial.println("End of loop");
}

/*********************** I2C CODE ***********************/

void getAndSendDataToAllFloors() {
  for (int i = 8; i < CONNECTED_SLAVES + 8; i++) { // https://www.arduino.cc/en/Reference/Wire inside NOTE: addresses should start from 8 
    //readSerialAndSendLiftRelatedData(i); // testing code
    getButtonPressedOfFloor(i); // De delay mogelijk verwijderen om de motor
    // nog soepeler te laten lopen, of the for loop in tunrMotorOnRequest verlengen
    delay(5);
    sendLiftRelatedData(i);
  }
}

void readSerialAndSendLiftRelatedData(int floorIndex) {
  if (Serial.available()) {
    String s = Serial.readString();
    int currentFloor = s.substring(0, 1).toInt();
    int openDoor = s.substring(1, 2).toInt();
    Serial.print("Sending ");
    Serial.print(currentFloor);
    Serial.print(" and ");
    Serial.print(openDoor);
    Wire.beginTransmission(floorIndex);
    Wire.write(currentFloor);
    Wire.write(openDoor);
    Wire.endTransmission();
  }
}

void sendLiftRelatedData(int floorIndex) {
  Serial.print("Sending ");
  Serial.print(currentFloor);
  Serial.print(" and ");
  Serial.print(doorOpen[floorIndex-8]);
  Serial.print(" to slave ");
  Serial.println(floorIndex);
  Wire.beginTransmission(floorIndex);
  Wire.write(currentFloor);
  Wire.write(doorOpen[floorIndex-8]);
  Wire.endTransmission();
}

void getButtonPressedOfFloor(int floorIndex) {
  Wire.requestFrom(floorIndex, 3);
  delay(10);

  if (Wire.available()) {
    floorButtonUp[floorIndex-8] = Wire.read();
    floorButtonDown[floorIndex-8] = Wire.read();
    liftArrived[floorIndex-8] = Wire.read();
    
    Serial.print("Button up ");
    Serial.print(floorButtonUp[floorIndex-8]);
    Serial.print(" and button down ");
    Serial.print(floorButtonDown[floorIndex-8]);
    Serial.print(" and ir data ");
    Serial.print(liftArrived[floorIndex-8]);
    Serial.print(" from slave ");
    Serial.println(floorIndex);
  }
}

/*********************** NON I2C CODE ***********************/

void debugArray() {
  Serial.print("Array length is ");
  Serial.println(sizeof(floorButtonUp)/sizeof(int));
  Serial.println("Floor button up");
  for (int i = 0; i < (sizeof(floorButtonUp)/sizeof(int)); i++) {
    Serial.println(floorButtonUp[i]);
  }
  Serial.println("Floor button down");
  for (int i = 0; i < (sizeof(floorButtonDown)/sizeof(int)); i++) {
    Serial.println(floorButtonDown[i]);
  }
}

void checkForMoveLift() {
  if (floorButtonDown[currentFloor] || floorButtonUp[currentFloor]) { // floor
    moveUp = false;
    moveDown = false;
    doorOpen[currentFloor] = true;
    Serial.println("Current floor wants to use lift");
    return;
  } else {
    doorOpen[currentFloor] = false;
  }

  if (movingUp) {
    // check floors above current floor to see if anyone wants to go up
    for(int i = currentFloor; i < (sizeof(floorButtonUp)/sizeof(int)); i++) {  // sizeof(int) used bacause arduino is a bitch https://www.arduino.cc/en/Reference/Sizeof
      if (floorButtonUp[i] == 1) {
        Serial.println("A floor above me wants to go up");
        moveUp = true;
        return;
      }
    }
    // check floors above current floor to see if anyone wants to go down
    for(int i = currentFloor; i < (sizeof(floorButtonDown)/sizeof(int)); i++) {
      if (floorButtonDown[i] == 1) {
        Serial.println("A floor above me wants to go down");
        moveUp = true;
        return;
      }
    }
    // check floor below current floor to see if anyone wants to go down
    for(int i = currentFloor; i >= 0; i--) {
      if (floorButtonDown[i] == 1) {
        Serial.println("A floor below me wants to go up");
        moveDown = true;
        movingUp = false;
        return;
      }
    }
    // check floors below current floor to see if anyone wants to go up
    for(int i = currentFloor; i >= 0; i--) {
      if (floorButtonUp[i] == 1) {
        Serial.println("A floor below me wants to go down");
        moveDown = true;
        movingUp = false;
        return;
      }
    }
  } else {
    // check floors below current floor to see if anyone wants to go up
    for(int i = currentFloor; i >= 0; i--) {
      if (floorButtonUp[i] == 1) {
        Serial.println("A floor below me wants to go up");
        moveDown = true;
        return;
      }
    }
    // check floor below current floor to see if anyone wants to go down
    for (int i = currentFloor; i >= 0; i--) {
      if (floorButtonDown[i] == 1) {
        Serial.println("A floor below me wants to go down");
        moveDown = true;
        return;
      }
    }
    // check floors above current floor to see if anyone wants to go down
    for (int i = currentFloor; i < (sizeof(floorButtonDown)/sizeof(int)); i++) {
      if (floorButtonDown[i] == 1) {
        Serial.println("A floor above me wants to go down");
        moveUp = true;
        movingUp = true;
        return;
      }
    }
    // check floors above current floor to see if anyone wants to go up
    for (int i = currentFloor; i < (sizeof(floorButtonUp)/sizeof(int)); i++) {
      if (floorButtonUp[i] == 1) {
        Serial.println("A floor above me wants to go up");
        moveUp = true;
        movingUp = true;
        return;
      }
    }
  }
}

void moveLift() {
  if (moveUp) {
    Serial.println("going up");
    liftController(true);
  } else if (moveDown) {
    Serial.println("going down");
    liftController(false);
  }
}

void liftController(boolean up) {
  clockwise = !up;

  for (int i = 0; i < 500; i++) { // remove or increase when needed,
    // with this for loop the motor moves more between requests.
    stepper();
    delayMicroseconds(2000);
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

