#include <Wire.h>

const int A = 2;  // For displaying segment "A"
const int B = 3;  // For displaying segment "B"
const int C = 4;  // For displaying segment "C"
const int D = 5;  // For displaying segment "D"
const int E = 6;  // For displaying segment "E"
const int F = 8;  // For displaying segment "F"
const int G = 9;  // For displaying segment "G"

// i2c gives back byte and arduino can turn it to an int 0/1
// but also into boolean, so why are we using int instead of boolean everywhere? - yoshua

const int BUTTON_UP_PIN = 11;
int goingUpButtonPressed = 0;
int goingDownButtonPressed = 0;

const int DOOR_LED_PIN = 10;

const int IR_OBSTACLE_PIN = 13;
int noObstacle = true;

int liftPosition = -1;
int openDoor = 0;

boolean resetFloorButtons = false;

void setup() {
  Serial.begin(9600);

  // For communication
  Wire.begin(1);
  Wire.onRequest(checkLiftDetectedByIrAndSendToMaster());
  Wire.onReceive(getLiftPosition);
  Wire.onRequest(sendEvent);

  // For lift segments
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);

  // For get lift. NOTE: only 1 of the 2 buttons for up or down
  pinMode(BUTTON_UP_PIN, INPUT);

  // For LED goes on when door open
  pinMode(DOOR_LED_PIN, OUTPUT);

  // For IR obstacle module (lift position near)
  pinMode(noObstacle, INPUT);
}

void loop() {
  keepReadingButtonUpAndDownUntilPressed(); // constantly read BUTTON_UP and DOWN until pressed

  handleDoorOpeningAndClosing(); // turn led on or off if openDoor = true

  ifLiftArrivedResetGoingUpOrDownButton(); // The lift buttons remain on pressed state until master gives a reset
}

/*********************** I2C CODE ***********************/

// get liftPosition and show it on LED display
void getLiftPosition(int howMany) {
  liftPosition = Wire.read();           // receive bit for lift as an integer
  openDoor = Wire.read();  // receive bit for floor door
  resetFloorButtons = Wire.read(); // 
  Serial.println(liftPosition);             // print the integer testing
  Serial.println(openDoor);

  displayHandler(liftPosition);          // show lift current location
}

// Send that I pressed the request lift button
void sendEvent() {
  Serial.println(goingUpButtonPressed);
  Serial.println(goingDownButtonPressed);
  byte buttonStates[] = { 
    goingUpButtonPressed, goingDownButtonPressed 
  };
  Wire.write(buttonStates, 2);
}

void checkLiftDetectedByIrAndSendToMaster() {
  noObstacle = digitalRead(IR_OBSTACLE_PIN);

  // TODO ADD THE CODE FOR SENDING DATA TO MASTER HERE

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
    goingDownButtonPressed = digitalRead(BUTTON_UP_PIN);
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

void displayHandler(int x) {
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
  digitalWrite(A,LOW);
  digitalWrite(B,LOW);
  digitalWrite(C,LOW);
  digitalWrite(D,LOW);
  digitalWrite(E,LOW);
  digitalWrite(F,LOW);
  digitalWrite(G,LOW);
}

