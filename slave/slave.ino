#include <Wire.h>

#define A 3// For displaying segment "A"
#define B 2 // For displaying segment "B"
#define C 7 // For displaying segment "C"
#define D 8 // For displaying segment "D"
#define E 9 // For displaying segment "E"
#define F 4 // For displaying segment "F"
#define G 5 // For displaying segment "G"
#define DP 6 // For displaying segment "."

// i2c gives back byte and arduino can turn it to an int 0/1
// but also into boolean, so why are we using int instead of boolean everywhere? - yoshua

// For buttons to request the lift
const int BUTTON_UP_PIN = 11;
const int BUTTON_DOWN_PIN = 12;
int goingUpButtonPressed = 0;
int goingDownButtonPressed = 0;

// For LED to simulate door opening/closing
const int DOOR_LED_PIN = 10;

// For ir detect lift arrived
// http://henrysbench.capnfatz.com/henrys-bench/arduino-sensors-and-input/arduino-ir-obstacle-sensor-tutorial-and-manual/
const int IR_PIN = 13;
int liftArrived = 0;      // false

// For displaying lift position on LED display
int liftPosition = -1;

// For receiving command for save to open door
int openDoor = 0;

// For data to send to master
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
  pinMode(liftArrived, INPUT);

  // For initialization
  int address = determineAddress();

  // For communication
  Wire.begin(address + 8); // https://www.arduino.cc/en/Reference/Wire inside NOTE: addresses should start from 8 
  Wire.onReceive(getLiftRelatedData);
  Wire.onRequest(sendDataToMaster);
}

void loop() {
  keepReadingButtonUpAndDownUntilPressed(); // constantly read BUTTON_UP and DOWN until pressed

  handleDoorAndResetFloorButtons();         // turn led on or off if openDoor = true
                                            // also reset lift buttons, because lift already arrived
}

/*********************** I2C CODE ***********************/

// get liftPosition and show it on LED display
void getLiftRelatedData(int numBytes) {
  liftPosition = Wire.read();           // receive bit for lift as an integer
  openDoor = Wire.read();               // receive bit for floor door

  Serial.println(liftPosition);  // remove after testing
  Serial.print("If openDoor = 1 LED ON + Reset floor button");
  Serial.println(openDoor);  // remove after testing

  ledDisplayHandler(liftPosition);         // show lift current location
}

void sendDataToMaster() {
  sendButtonStatesGoingUpAndDown();
  checkLiftArrivedAndSendToMaster();

  Wire.write(sendingData, 3);
}

// Send that I pressed the request lift button
void sendButtonStatesGoingUpAndDown() {
  Serial.println(goingUpButtonPressed);   // remove after testing
  Serial.println(goingDownButtonPressed); // remove after testing

  sendingData[0] = goingUpButtonPressed;
  sendingData[1] = goingDownButtonPressed; 
}

void checkLiftArrivedAndSendToMaster() {
  liftArrived = digitalRead(IR_PIN);
  liftArrived = !liftArrived; // because output is false if object arrived

  sendingData[2] = noLiftArrived;

  // remove after testing
  if (noLiftArrived) {
    Serial.println("clear");
  } else {
    Serial.println("OBSTACLE!!, OBSTACLE!!");
  }
}

/*********************** INITIALIZE LIFT CODE ***********************/

int determineAddress() {
  int selectingFloor = 0;
  while (true) {
    Serial.print("selectedFloor: ");
    Serial.println(selectingFloor);
    ledDisplayHandler(selectingFloor);
    keepReadingButtonUpAndDownUntilPressed();
    if (goingUpButtonPressed) {
      Serial.println("goingUpButtonPressed");
      selectingFloor++;
      goingUpButtonPressed = 0;
      if (selectingFloor > 9) {
        selectingFloor = 0;
      }
    }
    if (goingDownButtonPressed) {
      Serial.println("Floor selected (goingDownButtonPressed)");
      goingDownButtonPressed = 0;
      return selectingFloor;
    }
    delay(1000);
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
  Serial.print("goingUpButtonPressed: ");
  Serial.println(goingUpButtonPressed);
  Serial.print("goingDownButtonPressed: ");
  Serial.println(goingDownButtonPressed);
}

void handleDoorAndResetFloorButtons() {
  if (openDoor) {
    resetFloorButtons(); 

    digitalWrite(DOOR_LED_PIN, HIGH);
  } else {
    digitalWrite(DOOR_LED_PIN, LOW);
  }
  Serial.print("openDoor: ");
  Serial.println(openDoor);
}

void resetFloorButtons() {
  Serial.println("floor buttons reset");
  goingUpButtonPressed = 0;
  goingDownButtonPressed = 0;
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

