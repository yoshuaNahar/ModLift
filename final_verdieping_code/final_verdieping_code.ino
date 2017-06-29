#include <Wire.h>

const int A = 2;  // For displaying segment "A"
const int B = 3;  // For displaying segment "B"
const int C = 4;  // For displaying segment "C"
const int D = 5;  // For displaying segment "D"
const int E = 6;  // For displaying segment "E"
const int F = 8;  // For displaying segment "F"
const int G = 9;  // For displaying segment "G"

const int BUTTON_UP_PIN = 11;
int buttonStateUp = 0;
int buttonStateDown = 0;

const int DOOR_LED_PIN = 10;

const int IR_OBSTACLE_PIN = 13;
int noObstacle = true;

int liftPosition = -1;
int openDoor = 0;

boolean resetButtons = false;

void setup() {
  Serial.begin(9600);

  // For communication
  Wire.begin(1);
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
//  displayDigit(1);
  delay(1000);
//  turnOff();

  if (buttonStateUp == 0) {
    buttonStateUp = digitalRead(BUTTON_UP_PIN);
  }
  
//  if (buttonStateUp) {
//    Serial.println("Button pressed");
//  } else {
//    Serial.println("Button not pressed!");
//  }
  
  if (openDoor) {
    digitalWrite(DOOR_LED_PIN, HIGH); 
  } else {
    digitalWrite(DOOR_LED_PIN, LOW); 
  }
  
  checkLiftDetectedByIr();
  
  // The lift remains on buttonsState pressed until master gives a reset
  if (resetButtons) {
    buttonStateUp = 0;
    resetButtons = false;
  }
}

// Executes whenever data is received from master
// this function is registered as an event
void getLiftPosition(int howMany) {
  liftPosition = Wire.read();           // receive bit for lift as an integer
  openDoor = Wire.read();  // receive bit for floor door
  resetButtons = Wire.read(); // 
  Serial.println(liftPosition);             // print the integer testing
  Serial.println(openDoor);
  displayController(liftPosition);          // show lift current location
}

// Send that I pressed the request lift button
void sendEvent() {
  Serial.println(buttonStateUp);
  Serial.println(buttonStateDown);
  byte buttonStates[] = { buttonStateUp, buttonStateDown };
  Wire.write(buttonStates, 2);
}

void displayController(int x) {
  if (x >= 0 && x <= 9) {
    turnOff();
    displayDigit(x); 
  }
}

void checkLiftDetectedByIr() {
 noObstacle = digitalRead(IR_OBSTACLE_PIN);
  if (noObstacle) {
    // TODO: ADD THE CODE TO OPEN DOOR HERE
    Serial.println("clear"); // remove after testing
  } else {
    Serial.println("OBSTACLE!!, OBSTACLE!!"); // remove after testing
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

