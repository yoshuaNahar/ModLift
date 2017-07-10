#include <Wire.h>

#define A 3   // For displaying segment "A"
#define B 2   // For displaying segment "B"
#define C 7   // For displaying segment "C"
#define D 8   // For displaying segment "D"
#define E 9   // For displaying segment "E"
#define F 4   // For displaying segment "F"
#define G 5   // For displaying segment "G"
#define DP 6  // For displaying segment "."

// Set debug mode with Serial
const boolean DEBUG_MODE = true;

// For buttons to request the lift
const int BUTTON_UP_PIN = 11;
const int BUTTON_DOWN_PIN = 12;
const int BUTTON_UP_LED_PIN = A2;
const int BUTTON_DOWN_LED_PIN = A1;
int goingUpButtonPressed = 0;
int goingDownButtonPressed = 0;

// For LED to simulate door opening/closing
const int DOOR_LED_PIN = 10;

// For ir detect lift arrived
// http://henrysbench.capnfatz.com/henrys-bench/arduino-sensors-and-input/arduino-ir-obstacle-sensor-tutorial-and-manual/
const int IR_PIN = 13;
int liftArrived = 0;      // false

// For displaying lift position on LED display
int liftPosition = 0;

// For receiving command for save to open door
int openDoor = 0;

// For data to send to master
byte sendingData[3];

void setup() {
  if (DEBUG_MODE) {
    Serial.begin(9600);                     // Begin serial communication
  }

  // For led display segments
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(DP, OUTPUT);

  // For get lift
  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);

  // For led buttons
  pinMode(BUTTON_UP_LED_PIN, OUTPUT);
  pinMode(BUTTON_DOWN_LED_PIN, OUTPUT);

  // For LED to simulate door opening/closing
  pinMode(DOOR_LED_PIN, OUTPUT);

  // For ir detect lift arrived
  pinMode(liftArrived, INPUT);

  // For initialization
  int address = determineAddress();         // Run initialization to determine the location of the segment
  Serial.print("address: ");
  Serial.println(address);

  // For communication
  Wire.begin(address + 8);                  // Start I2C communication // https://www.arduino.cc/en/Reference/Wire inside NOTE: addresses should start from 8 
  Wire.onReceive(getLiftRelatedData);       // Set the event handler to handle data incoming from the master
  Wire.onRequest(sendDataToMaster);         // Set the event handler to hande data sending to the master
}

void loop() {
  keepReadingButtonUpAndDownUntilPressed(); // Constantly read BUTTON_UP and DOWN until pressed
  setSendingDataArray();                    // Fill sending data array, so that wire function is clean
  handleDoorAndResetFloorButtons();         // Turn led on or off and reset buttons
  ledDisplayHandler(liftPosition);          // Set led display to the current floor of the lift
  handleButtonLed();                        // Turn the button leds on and off
}

/*********************** I2C CODE ***********************/

// NOTE: Dont add Serial.prints or complex code in I2C functions.
void getLiftRelatedData(int numBytes) {
  // Get liftPosition and show it on LED display
  liftPosition = Wire.read();           // Receive byte containing current lift position
  openDoor = Wire.read();               // Receive byte containing whether the door should open
}

void sendDataToMaster() {
  // Send goingUpButton, goingDownButton and liftArrived (ir data)
  Wire.write(sendingData, 3);
}

/*********************** INITIALIZE FLOOR CODE ***********************/

int determineAddress() {
  // Initializes the segment by determinging the location of the segment
  int selectingFloor = 0;                                            // Set current selected location on 0
  ledDisplayHandler(selectingFloor);                                 // Show the currenly selected location
  digitalWrite(DP, HIGH);                                            // Show a dot on the led display to indicate initialization
  while (true) {
    if (DEBUG_MODE) {
      Serial.print("selectedFloor: ");
      Serial.println(selectingFloor);
    }
    keepReadingButtonUpAndDownUntilPressed();                        // Read the buttons on the segment
    if (goingUpButtonPressed) {                                      // If the floor up button is pressed
      if (DEBUG_MODE) {
        Serial.println("goingUpButtonPressed");
      }
      selectingFloor++;                                              // Increase the currently selected floor by one
      goingUpButtonPressed = 0;                                      // Reset the floor up button
      if (selectingFloor > 9) {                                      // If the currently selected floor is more then 9
        selectingFloor = 0;                                          // Reset the currently selected floor to 0
      }
      ledDisplayHandler(selectingFloor);                             // Show the currently selected location
      digitalWrite(DP, HIGH);                                        // Show a dot on the led display to indicate initialization
    } 
    else if (goingDownButtonPressed) {                               // If the floor down button is pressed
      if (DEBUG_MODE) {
        Serial.println("Floor selected (goingDownButtonPressed)");
      }
      turnOff();                                                     // Turn off led display to show successful initialization
      delay(2000);                                                   // Wait 2 seconds
      goingDownButtonPressed = 0;                                    // Reset the floor down button
      digitalWrite(DP, LOW);                                         // Disable the dot to indicate normal function
      return selectingFloor;                                         // Return the selected floor
    }
    delay(1000);
  }
}

/*********************** FLOOR CODE ***********************/

void setSendingDataArray() {
  // Fills the array with data to be send to the master
  if (DEBUG_MODE) {
    Serial.println("set SendingDataArray()");
  }
  sendButtonStatesGoingUpAndDown();
  checkLiftArrived();
}


void sendButtonStatesGoingUpAndDown() {
  // Fills the array with the current button states
  sendingData[0] = goingUpButtonPressed;      // Puts button up state in the array
  sendingData[1] = goingDownButtonPressed;    // Puts button down state in the array
}

void checkLiftArrived() {
  // Checks to see if the IR-module is triggered and put it in the array
  liftArrived = digitalRead(IR_PIN);

  liftArrived = !liftArrived;       // Invert output because the IR-module returns false if it is triggered

    sendingData[2] = liftArrived;   // Put it in the array
}

void keepReadingButtonUpAndDownUntilPressed() {
  // Reads the buttons until the are pressed
  if (!goingUpButtonPressed) {                              // If button up is not pressed
    goingUpButtonPressed = digitalRead(BUTTON_UP_PIN);      // Read button up
  }
  if (!goingDownButtonPressed) {                            // If button down is not pressed
    goingDownButtonPressed = digitalRead(BUTTON_DOWN_PIN);  // Read button down
  }
}

void handleDoorAndResetFloorButtons() {
  // Turns on the door led if the doors are open en resets the buttons
  if (openDoor) {                       // If door should be open
    resetFloorButtons();                // Reset buttons

    digitalWrite(DOOR_LED_PIN, HIGH);   // Turn on door led
  } 
  else {                                // If door should be closed
    digitalWrite(DOOR_LED_PIN, LOW);    // Turn off door led
  }
}

void resetFloorButtons() {
  // Resets the buttons
  if (DEBUG_MODE) {
    Serial.println("floor buttons reset");
  }
  goingUpButtonPressed = 0;     // Turn button up off
  goingDownButtonPressed = 0;   // Turn button down off
}

void ledDisplayHandler(int x) {
  // Shows a number on the led display
  if (x >= 0 && x <= 9) {   // If the number is between 0-9
    turnOff();              // Turn off the led display
    displayDigit(x);        // Show the number on the led display
  }
}

void handleButtonLed(){
  // Turn the button leds on and off
  if (goingUpButtonPressed) {                     // If button up is on
    digitalWrite(BUTTON_UP_LED_PIN, HIGH);        // Turn button up led on
  } else {                                        // If button up is off
    digitalWrite(BUTTON_UP_LED_PIN, LOW);         // Turn button up led off
  }
  if (goingDownButtonPressed) {                   // If button down is on
    digitalWrite(BUTTON_DOWN_LED_PIN, HIGH);      // Turn button down led on
  } else {                                        // If button down is off
    digitalWrite(BUTTON_DOWN_LED_PIN, LOW);       // Turn button down led off
  }
}

// Bekijk deze link om te zien welke letters gekoppeld
// zijn aan de led segmenten:
// https://cdn.instructables.com/ORIG/FA0/S3TG/GZUAG6G2/FA0S3TGGZUAG6G2.gif
void displayDigit(int digit) {
  // Handles showing a specific digit on the led display
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
  // Turns off the led display
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);
  digitalWrite(E, LOW);
  digitalWrite(F, LOW);
  digitalWrite(G, LOW);
  digitalWrite(DP, LOW);
}


