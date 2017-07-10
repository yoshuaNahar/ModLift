#include<Wire.h>

// setup display
#define A 34// For displaying segment "A"
#define B 32 // For displaying segment "B"
#define C 30 // For displaying segment "C"
#define D 28 // For displaying segment "D"
#define E 26 // For displaying segment "E"
#define F 24 // For displaying segment "F"
#define G 22 // For displaying segment "G"

// Setup stepper
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

// For stepper
int steps = 0;
boolean clockwise = true;

// setup buttons
#define BUTTON1 53
#define BUTTON2 51
#define BUTTON3 49
#define BUTTON4 47
#define BUTTON5 45
#define BUTTON6 43

// setup button led

#define LED_BUTTON1 52
#define LED_BUTTON2 50
#define LED_BUTTON3 48
#define LED_BUTTON4 46
#define LED_BUTTON5 44
#define LED_BUTTON6 42

// For I2C and lift
const int CONNECTED_SLAVES = 5;               // Amount of slaves attached
int floorButtonUp[CONNECTED_SLAVES];          // Array containing button information from the slaves
int floorButtonDown[CONNECTED_SLAVES];        // Array containing button information from the slaves
int floorButtonElevator[6] = {0,0,0,0,0,0};   // Array containing button information from the master
boolean doorOpen[CONNECTED_SLAVES];           // If Door on floor should open, also reset floor get lift buttons
boolean liftAvailable[CONNECTED_SLAVES];      // Array containing which IR-modules are currently triggered

int currentFloor = 0;                         // Current location of the elevator

boolean movingUp = true;                      // which direction the elevator is going
boolean moveUp = false;                       // Should the motor move clockwise
boolean moveDown = false;                     // Should the motor move counter-clockwise

void setup() {
  Serial.begin(9600);                         // Begin serial communication

  Wire.begin();                               // Begin I2C communication

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

  // setup buttons
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  pinMode(BUTTON4, INPUT);
  pinMode(BUTTON5, INPUT);
  pinMode(BUTTON6, INPUT);

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
  getAndSendDataToAllFloors();                  // Retrieve which buttons are pressed and send the location of the elevator to the slaves
  Serial.println("Reading elevator buttons");
  readButtons();                                // Read the buttons connected to the master
  handleLedButton();                            // Turn the leds on the buttons on and off
  Serial.println("debugArray");
  debugArray();                                 // Print contents of the arrays to the terminal for debugging purposes
  Serial.println("checkForMoveLift");
  checkForMoveLift();                           // Check to see whether the elevator needs to move
  Serial.println("moveLift");
  moveLift();                                   // Move the elevator to its destination
  Serial.print("Current lift location is ");
  Serial.println(currentFloor);
  ledDisplayHandler(currentFloor);              // Change the display on the master
  delay(100);
  //Serial.println("End of loop");
}

/*********************** I2C CODE ***********************/

void getAndSendDataToAllFloors() {
  // For each of the connected slaves, request the currently pressed buttons and send where the elevator is and whether the door should open
  for (int i = 8; i < CONNECTED_SLAVES + 8; i++) { // https://www.arduino.cc/en/Reference/Wire inside NOTE: addresses should start from 8 
    Serial.print("getAndSend to slave ");
    Serial.println(i);
    Serial.println("GetButtonPressedOfFloor");
    if(!getButtonPressedOfFloor(i)){  // Requests the currently presseds buttons on a slave
      Serial.println("Failed");
    }
    
    delay(5);
    Serial.println("sendLiftRelatedData");
    sendLiftRelatedData(i);   // Send current location and door status to the slaves
  }
}

void readSerialAndSendLiftRelatedData(int floorIndex) {
  // Reads the terminal and sends the data to a slave, for debugging purposes only
  if (Serial.available()) {
    String s = Serial.readString();                   // Read the serial terminal
    int currentFloor = s.substring(0, 1).toInt();     // Get the first number
    int openDoor = s.substring(1, 2).toInt();         // Get the second number
    Serial.print("Sending ");
    Serial.print(currentFloor);
    Serial.print(" and ");
    Serial.print(openDoor);
    
    Wire.beginTransmission(floorIndex);               // Begin transmitting to a slave
    Wire.write(currentFloor);                         // Sends a byte containing the current location of the elevator
    Wire.write(openDoor);                             // Sends a byte containing whether the door should open on that floor
    Wire.endTransmission();                           // Stops transmitting to a slave
  }
}

void sendLiftRelatedData(int floorIndex) {
  // Sends the current floor and whether the door should open to the slave
  Serial.print("Sending ");
  Serial.print(currentFloor);
  Serial.print(" and ");
  Serial.print(doorOpen[floorIndex-8]);
  Serial.print(" to slave ");
  Serial.println(floorIndex);
  
  Wire.beginTransmission(floorIndex);                 // Begin transmitting to a slave
  Wire.write(currentFloor);                           // Sends a byte containing the current location of the elevator
  Wire.write(doorOpen[floorIndex-8]);                 // Sends a byte containing whether the door should open on that floor
  Wire.endTransmission();                             // Stops transmitting to a slave
}

boolean getButtonPressedOfFloor(int floorIndex) {
  // Requests the button states from a slave
  Serial.println("Begin request");
  Wire.requestFrom(floorIndex, 3);                    // Requests the slave to send 3 bytes of information
  Serial.println("End request");
  delay(10);                                          // Small delay to make sure the entire message has been received

  Serial.println("Availibility check");
  if (Wire.available()) {                             // If there is an answer
    Serial.println("Reading the wire");
    floorButtonUp[floorIndex-8] = Wire.read();        // Read the first byte containing the button up state
    floorButtonDown[floorIndex-8] = Wire.read();      // Read the second byte containing the button down state
    liftAvailable[floorIndex-8] = Wire.read();        // Read the third byte containing the IR-module state

    if(liftAvailable[floorIndex-8]){                  // If the IR-module is triggered
      currentFloor = (floorIndex-8);                  // Update currentFloor to this floor
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
  // Prints all the arrays to the terminal for debugging purposes
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
  Serial.println("Elevator button");
  for (int i = 0; i < 6; i++) {
    Serial.println(floorButtonElevator[i]);
  }
}

void readButtons(){
  // Read all the buttons connected to the master
  if (!floorButtonElevator[0]){
    floorButtonElevator[0] = digitalRead(BUTTON1);
  }
  if (!floorButtonElevator[1]){
    floorButtonElevator[1] = digitalRead(BUTTON2);
  }
  if (!floorButtonElevator[2]){
    floorButtonElevator[2] = digitalRead(BUTTON3);
  }
  if (!floorButtonElevator[3]){
    floorButtonElevator[3] = digitalRead(BUTTON4);
  }
  if (!floorButtonElevator[4]){
    floorButtonElevator[4] = digitalRead(BUTTON5);
  }
  if (!floorButtonElevator[5]){
    floorButtonElevator[5] = digitalRead(BUTTON6);
  }
}

void handleLedButton(){
  // Turns on the led if the button is pressed, or turn it off when it isn't
  if (floorButtonElevator[0]){
    digitalWrite(LED_BUTTON1, HIGH);
  } else {
    digitalWrite(LED_BUTTON1, LOW);
  }
  if (floorButtonElevator[1]){
    digitalWrite(LED_BUTTON2, HIGH);
  } else {
    digitalWrite(LED_BUTTON2, LOW);
  }
  if (floorButtonElevator[2]){
    digitalWrite(LED_BUTTON3, HIGH);
  } else {
    digitalWrite(LED_BUTTON3, LOW);
  }
  if (floorButtonElevator[3]){
    digitalWrite(LED_BUTTON4, HIGH);
  } else {
    digitalWrite(LED_BUTTON4, LOW);
  }
  if (floorButtonElevator[4]){
    digitalWrite(LED_BUTTON5, HIGH);
  } else {
    digitalWrite(LED_BUTTON5, LOW);
  }
  if (floorButtonElevator[5]){
    digitalWrite(LED_BUTTON6, HIGH);
  } else {
    digitalWrite(LED_BUTTON6, LOW);
  }
}

void checkForMoveLift() {
  // Main algorithm to check to where the lift should move
  Serial.print("Check to see if current floor wants to use lift: ");
  if ((floorButtonDown[currentFloor] || floorButtonUp[currentFloor] || floorButtonElevator[currentFloor]) && liftAvailable[currentFloor]) {  // If there is a button pressed for the current floor and the lift is available
    if (movingUp && (floorButtonUp[currentFloor] || floorButtonElevator[currentFloor])) {
      // If lift is moving up and floorUp is pressed, stop at current floor
      moveUp = false;
      moveDown = false;
      doorOpen[currentFloor] = true;
      floorButtonElevator[currentFloor] = false;
      Serial.println("Current floor wants to use lift");

      delay(3000); // Might not be nessesary
      return;
    } else if (!movingUp && (floorButtonDown[currentFloor] || floorButtonElevator[currentFloor])) {
      // If lift is moving down and floorDown is pressed, stop at current floor
      moveUp = false;
      moveDown = false;
      doorOpen[currentFloor] = true;
      floorButtonElevator[currentFloor] = false;
      Serial.println("Current floor wants to use lift");

      delay(3000); // Might not be nessesary
      return;
    } else if (movingUp) {
      // if lift is moving up and buttonDown is pressed and there are no buttons pressed above me, stop at current floor
      boolean stopHere = true;
      for(int i = currentFloor + 1; i < (sizeof(floorButtonUp)/sizeof(int)); i++){
        if (floorButtonUp[i] || floorButtonDown[i] || floorButtonElevator[i]){
          stopHere = false;
          break;
        }
      }

      if (stopHere){
        moveUp = false;
        moveDown = false;
        doorOpen[currentFloor] = true;
        floorButtonElevator[currentFloor] = false;
        Serial.println("Current floor wants to use lift");
        delay(3000);
        return;
      }
    } else if (!movingUp) {
      // if lift is moving down and buttonUp is pressed and there are no buttons pressed below me, stop at current floor
      boolean stopHere = true;
      for(int i = currentFloor - 1; i >= 0; i--){
        if (floorButtonUp[i] || floorButtonDown[i] || floorButtonElevator[i]){
          stopHere = false;
          break;
        }
      }

      if (stopHere){
        moveUp = false;
        moveDown = false;
        doorOpen[currentFloor] = true;
        floorButtonElevator[currentFloor] = false;
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
  if (movingUp) {   // When the lift is moving up
    // check floors above current floor to see if anyone wants to go up
    for(int i = currentFloor; i < (sizeof(floorButtonUp)/sizeof(int)); i++) {  // sizeof(int) used bacause arduino is a bitch https://www.arduino.cc/en/Reference/Sizeof
      if (floorButtonUp[i] || floorButtonElevator[i]) {
        Serial.println("A floor above me wants to go up");
        moveUp = true;
        moveDown = false;
        movingUp = true;
        return;
      }
    }
    // check floors above current floor to see if anyone wants to go down
    for(int i = currentFloor; i < (sizeof(floorButtonDown)/sizeof(int)); i++) {
      if (floorButtonDown[i] || floorButtonElevator[i]) {
        Serial.println("A floor above me wants to go down");
        moveUp = true;
        moveDown = false;
        movingUp = true;
        return;
      }
    }
    // check floor below current floor to see if anyone wants to go down
    for(int i = currentFloor; i >= 0; i--) {
      if (floorButtonDown[i] || floorButtonElevator[i]) {
        Serial.println("A floor below me wants to go up");
        moveUp = false;
        moveDown = true;
        movingUp = false;
        return;
      }
    }
    // check floors below current floor to see if anyone wants to go up
    for(int i = currentFloor; i >= 0; i--) {
      if (floorButtonUp[i] || floorButtonElevator[i]) {
        Serial.println("A floor below me wants to go down");
        moveUp = false;
        moveDown = true;
        movingUp = false;
        return;
      }
    }
    Serial.println("Nothing wants to use lift");
  } else if (!movingUp) {  // When the lift is moving down
    
    // check floors below current floor to see if anyone wants to go up
    for(int i = currentFloor; i >= 0; i--) {
      if (floorButtonUp[i] || floorButtonElevator[i]) {
        Serial.println("A floor below me wants to go up");
        moveUp = false;
        moveDown = true;
        movingUp = false;
        return;
      }
    }
    // check floor below current floor to see if anyone wants to go down
    for (int i = currentFloor; i >= 0; i--) {
      if (floorButtonDown[i] || floorButtonElevator[i]) {
        Serial.println("A floor below me wants to go down");
        moveUp = false;
        moveDown = true;
        movingUp = false;
        return;
      }
    }
    // check floors above current floor to see if anyone wants to go down
    for (int i = currentFloor; i < (sizeof(floorButtonDown)/sizeof(int)); i++) {
      if (floorButtonDown[i] || floorButtonElevator[i]) {
        Serial.println("A floor above me wants to go down");
        moveUp = true;
        moveDown = false;
        movingUp = true;
        return;
      }
    }
    // check floors above current floor to see if anyone wants to go up
    for (int i = currentFloor; i < (sizeof(floorButtonUp)/sizeof(int)); i++) {
      if (floorButtonUp[i] || floorButtonElevator[i]) {
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
  // Controls the lift controller
  if (moveUp) {
    Serial.println("going up");
    liftController(true);
  } else if (moveDown) {
    Serial.println("going down");
    liftController(false);
  }
}

void liftController(boolean up) {
  // Controls how many the stepping motor should rotate
  clockwise = !up;

  if(floorButtonElevator[5]){ //When the emergency button is pressed, stop moving the motor untill the arduino resets
    return;
  }
  
  for (int i = 0; i < 1000; i++) { // remove or increase when needed,
    // with this for loop the motor moves more between requests.
    stepper();
    delayMicroseconds(2000);
  }
}

// http://www.nmbtc.com/step-motors/engineering/full-half-and-microstepping/
// Using two-phase fullstep
void stepper() {
  // controls the steppenmotor
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
  // Shows a number on the led display
  if (x >= 0 && x <= 9) {
    turnOff();
    displayDigit(x); 
  }
}

// Bekijk deze link om te zien welke letters gekoppeld
// zijn aan de led segmenten:
// https://cdn.instructables.com/ORIG/FA0/S3TG/GZUAG6G2/FA0S3TGGZUAG6G2.gif
void displayDigit(int digit) {
  // Shows a specific digit on the led display
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
  // Turns the led display off
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);
  digitalWrite(E, LOW);
  digitalWrite(F, LOW);
  digitalWrite(G, LOW);
}


