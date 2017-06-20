#include <Wire.h>

const int A = 2;  // For displaying segment "A"
const int B = 3;  // For displaying segment "B"
const int C = 4;  // For displaying segment "C"
const int D = 5;  // For displaying segment "D"
const int E = 6;  // For displaying segment "E"
const int F = 8;  // For displaying segment "F"
const int G = 9;  // For displaying segment "G"

const int BUTTON_PIN = 12;
int buttonState = 0;

const int LED_PIN = 13;

int liftIndex = -1;
int openDoor = 0;

void setup() {
  Serial.begin(9600); // for testing

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

  pinMode(BUTTON_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
}

void loop() {
//  displayDigit(1);
  delay(1000);
//  turnOff();

  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState) {
    Serial.println("Button pressed");
  } else {
    Serial.println("Button not pressed!");
  }
  
  if (openDoor) {
    digitalWrite(LED_PIN, HIGH); 
  } else {
    digitalWrite(LED_PIN, LOW); 
  }
}

// Executes whenever data is received from master
// this function is registered as an event
void getLiftPosition(int howMany) {
  liftIndex = Wire.read();           // receive bit for lift as an integer
  openDoor = Wire.read();            // receive bit for floor door
  Serial.println(liftIndex);             // print the integer testing
  Serial.println(openDoor);
  displayController(liftIndex);          // show lift current location
}

// Send that I pressed the request lift button
void sendEvent() {
  Serial.println("hello ");
  Serial.println(buttonState);
  Wire.write(buttonState);
}

void displayController(int x) {
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

