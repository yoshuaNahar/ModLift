// Deze code is om specifieke functionaliteit te testen zonder I2C

const int A = 2;  // For displaying segment "A"
const int B = 3;  // For displaying segment "B"
const int C = 4;  // For displaying segment "C"
const int D = 5;  // For displaying segment "D"
const int E = 6;  // For displaying segment "E"
const int F = 8;  // For displaying segment "F"
const int G = 9;  // For displaying segment "G"

// IR
const int isObstaclePin = 13;
boolean noObstacle = true;

void setup() {
  Serial.begin(9600); // for testing
  // For lift segments
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);
  
  // IR
  pinMode(noObstacle, INPUT);
}

void loop() {
  displayDigit(0);
  delay(1000);
  turnOff();
  
  noObstacle = digitalRead(isObstaclePin);
  if (noObstacle) {
    Serial.println("clear");
  } else {
    Serial.println("OBSTACLE!!, OBSTACLE!!");
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

