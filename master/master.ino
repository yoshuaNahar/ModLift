#include<Wire.h>

// setup display
#define A 22// For displaying segment "A"
#define B 24 // For displaying segment "B"
#define C 26 // For displaying segment "C"
#define D 28 // For displaying segment "D"
#define E 30 // For displaying segment "E"
#define F 32 // For displaying segment "F"
#define G 34 // For displaying segment "G"

// Setup stepper
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

// For stepper
int steps = 0;
boolean clockwise = true;

// For I2C and lift
const int CONNECTED_SLAVES = 5;
int floorButtonUp[CONNECTED_SLAVES];
int floorButtonDown[CONNECTED_SLAVES];
boolean doorOpen[CONNECTED_SLAVES];           // If Door on floor should open, also reset floor get lift buttons
boolean liftAvailable[CONNECTED_SLAVES];

int currentFloor = 0;

// de moveUp en moveDown is toch eigenlijk niet nodig? Er zijn maar 2 variablen nodig,
// moet de lift bewegen en welke richting
boolean movingUp = true;                      // which direction the elevator is going
boolean moveUp = false;                       // Should the moter move clockwise
boolean moveDown = false;                     // Should the motor move counter-clockwise

void setup() {
  Serial.begin(9600);

  Wire.begin();

  // For led display segments
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);

  // Setup stepper
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // this data is send to the slaves, so set to false first
  for (int i = 0; i < CONNECTED_SLAVES; i++) {
    doorOpen[i] = false;
    liftAvailable[i] = false;
    floorButtonUp[i] = 0;
    floorButtonDown[i] = 0;
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
  Serial.println(currentFloor);
  ledDisplayHandler(currentFloor);
  delay(100);
  //Serial.println("End of loop");
}

/*********************** I2C CODE ***********************/

void getAndSendDataToAllFloors() {
  for (int i = 8; i < CONNECTED_SLAVES + 8; i++) { // https://www.arduino.cc/en/Reference/Wire inside NOTE: addresses should start from 8 
    //readSerialAndSendLiftRelatedData(i); // testing code
    Serial.print("getAndSend to slave ");
    Serial.println(i);
    Serial.println("GetButtonPressedOfFloor");
    if(!getButtonPressedOfFloor(i)){ 
      Serial.println("Failed");
    }
    // De delay mogelijk verwijderen om de motor
    // nog soepeler te laten lopen, of the for loop in tunrMotorOnRequest verlengen
    delay(5);
    Serial.println("sendLiftRelatedData");
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

boolean getButtonPressedOfFloor(int floorIndex) {
  Serial.println("Begin request");
  Wire.requestFrom(floorIndex, 3);
  Serial.println("End request");
  delay(10);

  Serial.println("Availibility check");
  if (Wire.available()) {
    Serial.println("Reading the wire");
    floorButtonUp[floorIndex-8] = Wire.read();
    floorButtonDown[floorIndex-8] = Wire.read();
    liftAvailable[floorIndex-8] = Wire.read();

    if(liftAvailable[floorIndex-8]){
      currentFloor = (floorIndex-8);
    }
    
    Serial.print("Button up ");
    Serial.print(floorButtonUp[floorIndex-8]);
    Serial.print(" and button down ");
    Serial.print(floorButtonDown[floorIndex-8]);
    Serial.print(" and ir data ");
    Serial.print(liftAvailable[floorIndex-8]);
    Serial.print(" from slave ");
    Serial.println(floorIndex);
    return true;
  }
  return false;
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
  Serial.print("Check to see if current floor wants to use lift: ");
  if ((floorButtonDown[currentFloor] || floorButtonUp[currentFloor]) && liftAvailable[currentFloor]) {  // if current floor has a button pressed and lift is available
    if (movingUp && floorButtonUp[currentFloor]) {
      // if lift is moving up and floorUp is pressed, stop at current floor
      moveUp = false;
      moveDown = false;
      doorOpen[currentFloor] = true;
      Serial.println("Current floor wants to use lift");

      delay(3000); // testing if needed
      return;
    } else if (!movingUp && floorButtonDown[currentFloor]) {
      // if lift is moving down and floorDown is pressed, stop at current floor
      moveUp = false;
      moveDown = false;
      doorOpen[currentFloor] = true;
      Serial.println("Current floor wants to use lift");

      delay(3000); // testing if needed
      return;
    } else if (movingUp) {
      // if lift is moving up and buttonDown is pressed and there are no buttons pressed above me, stop at current floor
      boolean stopHere = true;
      for(int i = currentFloor + 1; i < (sizeof(floorButtonUp)/sizeof(int)); i++){
        if (floorButtonUp[i] || floorButtonDown[i]){
          stopHere = false;
          break;
        }
      }

      if (stopHere){
        moveUp = false;
        moveDown = false;
        doorOpen[currentFloor] = true;
        Serial.println("Current floor wants to use lift");
        delay(3000);
        return;
      }
    } else if (!movingUp) {
      // if lift is moving down and buttonUp is pressed and there are no buttons pressed below me, stop at current floor
      boolean stopHere = true;
      for(int i = currentFloor - 1; i >= 0; i--){
        if (floorButtonUp[i] || floorButtonDown[i]){
          stopHere = false;
          break;
        }
      }

      if (stopHere){
        moveUp = false;
        moveDown = false;
        doorOpen[currentFloor] = true;
        Serial.println("Current floor wants to use lift");
        delay(3000);
        return;
      }
    } else {
      Serial.println("current floor wants to go in a different direction, skipping floor");
    }
  } else {
    Serial.println("Current floor does not want to use the lift");
    doorOpen[currentFloor] = false;
  }

  Serial.print("Check for other floor wants to use lift: ");
  if (movingUp) {
    // check floors above current floor to see if anyone wants to go up
    for(int i = currentFloor; i < (sizeof(floorButtonUp)/sizeof(int)); i++) {  // sizeof(int) used bacause arduino is a bitch https://www.arduino.cc/en/Reference/Sizeof
      if (floorButtonUp[i] == 1) {
        Serial.println("A floor above me wants to go up");
        moveUp = true;
        moveDown = false;
        movingUp = true;
        return;
      }
    }
    // check floors above current floor to see if anyone wants to go down
    for(int i = currentFloor; i < (sizeof(floorButtonDown)/sizeof(int)); i++) {
      if (floorButtonDown[i] == 1) {
        Serial.println("A floor above me wants to go down");
        moveUp = true;
        moveDown = false;
        movingUp = true;
        return;
      }
    }
    // check floor below current floor to see if anyone wants to go down
    for(int i = currentFloor; i >= 0; i--) {
      if (floorButtonDown[i] == 1) {
        Serial.println("A floor below me wants to go up");
        moveUp = false;
        moveDown = true;
        movingUp = false;
        return;
      }
    }
    // check floors below current floor to see if anyone wants to go up
    for(int i = currentFloor; i >= 0; i--) {
      if (floorButtonUp[i] == 1) {
        Serial.println("A floor below me wants to go down");
        moveUp = false;
        moveDown = true;
        movingUp = false;
        return;
      }
    }
    Serial.println("Nothing wants to use lift");
  } else if (!movingUp) {
    
    // check floors below current floor to see if anyone wants to go up
    for(int i = currentFloor; i >= 0; i--) {
      if (floorButtonUp[i] == 1) {
        Serial.println("A floor below me wants to go up");
        moveUp = false;
        moveDown = true;
        movingUp = false;
        return;
      }
    }
    // check floor below current floor to see if anyone wants to go down
    for (int i = currentFloor; i >= 0; i--) {
      if (floorButtonDown[i] == 1) {
        Serial.println("A floor below me wants to go down");
        moveUp = false;
        moveDown = true;
        movingUp = false;
        return;
      }
    }
    // check floors above current floor to see if anyone wants to go down
    for (int i = currentFloor; i < (sizeof(floorButtonDown)/sizeof(int)); i++) {
      if (floorButtonDown[i] == 1) {
        Serial.println("A floor above me wants to go down");
        moveUp = true;
        moveDown = false;
        movingUp = true;
        return;
      }
    }
    // check floors above current floor to see if anyone wants to go up
    for (int i = currentFloor; i < (sizeof(floorButtonUp)/sizeof(int)); i++) {
      if (floorButtonUp[i] == 1) {
        Serial.println("A floor above me wants to go up");
        moveUp = true;
        moveDown = false;
        movingUp = true;
        return;
      }
    }
  }
  Serial.println("Nothing wants to use lift");
  moveUp = false;
  moveDown = false;
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

  for (int i = 0; i < 1000; i++) { // remove or increase when needed,
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

void ledDisplayHandler(int x) {
  if (x >= 0 && x <= 9) {
    turnOff();
    displayDigit(x); 
  }
}

// Bekijk deze link om te zien welke letters gekoppeld
// zijn aan de led segmenten:
// https://cdn.instructables.com/ORIG/FA0/S3TG/GZUAG6G2/FA0S3TGGZUAG6G2.gif
void displayDigit(int digit) {
  // Conditions for displaying segment A
  if(digit != 1 && digit != 4) {
    digitalWrite(A, HIGH);
  }
  // Conditions for displaying segment B
  if(digit != 5 && digit != 6) {
    digitalWrite(B, HIGH);
  }
  // Conditions for displaying segment C
  if(digit != 2) {
    digitalWrite(C, HIGH);
  }
  // Conditions for displaying segment D
  if(digit != 1 && digit != 4 && digit != 7) {
    digitalWrite(D, HIGH);
  } 
  // Conditions for displaying segment E 
  if(digit == 2 || digit == 6 || digit == 8 || digit == 0) {
    digitalWrite(E, HIGH);
  }
  // Conditions for displaying segment F
  if(digit != 1 && digit != 2 && digit != 3 && digit != 7) {
    digitalWrite(F, HIGH);
  }
  // Conditions for displaying segment G
  if (digit != 0 && digit != 1 && digit != 7) {
    digitalWrite(G, HIGH);
  }
}

void turnOff() {
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);
  digitalWrite(E, LOW);
  digitalWrite(F, LOW);
  digitalWrite(G, LOW);
}


