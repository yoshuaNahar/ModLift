#include <Wire.h>

#define A 2 // For displaying segment "A"
#define B 3 // For displaying segment "B"
#define C 4 // For displaying segment "C"
#define D 5 // For displaying segment "D"
#define E 6 // For displaying segment "E"
#define F 8 // For displaying segment "F"
#define G 9 // For displaying segment "G"

// i2c gives back byte and arduino can turn it to an int 0/1
// but also into boolean, so why are we using int instead of boolean everywhere? - yoshua

// For buttons to request the lift
const int BUTTON_UP_PIN = 11;
const int BUTTON_DOWN_PIN = 12;
int goingUpButtonPressed = 0;
int goingDownButtonPressed = 0;
boolean resetFloorButtons = false;

// For LED to simulate door opening/closing
const int DOOR_LED_PIN = 10;

// For ir detect lift arrived
const int IR_OBSTACLE_PIN = 13;
int noObstacle = 1;                 // true

// For displaying lift position on LED display
int liftPosition = -1;

// For receiving command for save to open door
int openDoor = 0;

// For Data to send to master
byte sendingData[3];

void setup() {
  Serial.begin(9600);

  // For led display segments
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);

  // For get lift
  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);
  
  // For LED to simulate door opening/closing
  pinMode(DOOR_LED_PIN, OUTPUT);

  // For ir detect lift arrived
  pinMode(noObstacle, INPUT);

  // For initializationl
  int address = determineAddress();

  // For communication
  Wire.begin(address + 8); // https://www.arduino.cc/en/Reference/Wire inside NOTE: addresses should start from 8 
  Wire.onReceive(getLiftRelatedData);
  Wire.onRequest(sendDataToMaster);
}

void loop() {
  keepReadingButtonUpAndDownUntilPressed(); // constantly read BUTTON_UP and DOWN until pressed

  handleDoorOpeningAndClosing();            // turn led on or off if openDoor = true

  ifLiftArrivedResetGoingUpOrDownButton();  // The lift buttons remain on pressed state until master gives a reset
}

/*********************** I2C CODE ***********************/

// get liftPosition and show it on LED display
void getLiftRelatedData(int numBytes) {
  liftPosition = Wire.read();           // receive bit for lift as an integer
  openDoor = Wire.read();               // receive bit for floor door
  resetFloorButtons = Wire.read();

  Serial.println(liftPosition);  // remove after testing
  Serial.println(openDoor);  // remove after testing
  Serial.println(resetFloorButtons); // remove after testing

  ledDisplayHandler(liftPosition);         // show lift current location
}

void sendDataToMaster() {
  sendButtonStatesGoingUpANdDown();
  checkLiftDetectedByIrAndSendToMaster();

  Wire.write(sendingData, 3);
}

// Send that I pressed the request lift button
void sendButtonStatesGoingUpAndDown() {
  Serial.println(goingUpButtonPressed);   // remove after testing
  Serial.println(goingDownButtonPressed); // remove after testing

  sendingData[0] = goingUpButtonPressed;
  sendingData[1] = goingDownButtonPressed; 
}

void checkLiftDetectedByIrAndSendToMaster() {
  noObstacle = digitalRead(IR_OBSTACLE_PIN);

  sendingData[2] = noObstacle;

  // remove after testing
  if (noObstacle) {
    Serial.println("clear");
  } else {
    Serial.println("OBSTACLE!!, OBSTACLE!!");
  }
}

/*********************** NON I2C CODE ***********************/

void keepReadingButtonUpAndDownUntilPressed() {
  if (goingUpButtonPressed == 0) {
    goingUpButtonPressed = digitalRead(BUTTON_UP_PIN);
  }
  if (goingDownButtonPressed == 0) {
    goingDownButtonPressed = digitalRead(BUTTON_DOWN_PIN);
  }
}

void handleDoorOpeningAndClosing() {
  if (openDoor) {
    digitalWrite(DOOR_LED_PIN, HIGH);
  } else {
    digitalWrite(DOOR_LED_PIN, LOW);
  }
}

void ifLiftArrivedResetGoingUpOrDownButton() {
  if (resetFloorButtons) {
    if (true) { // TODO: ONLY RESET IF LIFT GOING IN THE CORRECT DIRECTION AS THE BUTTON PRESSED
      goingUpButtonPressed = 0;
    } else {
      goingDownButtonPressed = 0;
    }
    resetFloorButtons = false;
  }
}

void ledDisplayHandler(int x) {
  if (x >= 0 && x <= 9) {
    turnOff();
    displayDigit(x); 
  }
}

// Run during initialization
int determineAddress() {
  int selectingFloor = 0;
  while (true) {
    Serial.println(selectingFloor);
    ledDisplayHandler(selectingFloor);
    keepReadingButtonUpAndDownUntilPressed();
    if (goingUpButtonPressed) {
      Serial.println("ButtonPressed");
      selectingFloor++;
      goingUpButtonPressed = 0;
      if (selectingFloor > 9) {
        selectingFloor = 0;
      }
    }
    if (goingDownButtonPressed) {
      Serial.println("Floor selected");
      goingDownButtonPressed = 0;
      return selectingFloor;
    }
    delay(1000);
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

