/*
  DigitalReadSerial
 Reads a digital input on pin 2, prints the result to the serial monitor 
 
 This example code is in the public domain.
 */

// digital pin 2 has a pushbutton attached to it. Give it a name:
int buttonUp = 10;
int buttonDown = 11;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the pushbutton's pin an input:
  pinMode(buttonDown, INPUT);
  pinMode(buttonUp, INPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
  int buttonStateUp = digitalRead(buttonUp);
  int buttonStateDown = digitalRead(buttonDown);
  
  // print out the state of the button:
  Serial.print("buttonStateUp: ");
  Serial.println(buttonStateUp);
  Serial.print("buttonStateDown: ");
  Serial.println(buttonStateDown);
  
  delay(1000);        // delay in between reads for stability
}



