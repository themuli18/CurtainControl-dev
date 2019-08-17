/*  Arduino code for curtain control with CNC Shield

Created by Aike Mueller with the help of a large number of tutorials and
example code.
This code is in the public domain
*/

 /*
Created by Yvan / https://Brainy-Bits.com
This code is in the public domain...
You can: copy it, use it, modify it, share it or just plain ignore it!
Thx!
*/

#include <EEPROM.h>
//EEPROM Library to store status during power cycles.

long address=0;
long number1 = 10;
long number2 = 987654321;

//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to address + 3.
long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }

void EEPROMWritelong(int address, long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }

long storedposition = EEPROMReadlong(address);
     

#include "AccelStepper.h" 
// AccelStepper Library http://www.airspayce.com/mikem/arduino/AccelStepper/



// AccelStepper Setup

#define MOTOR_A_ENABLE_PIN 8
#define MOTOR_A_STEP_PIN 3
#define MOTOR_A_DIR_PIN 6
AccelStepper stepperX(1, MOTOR_A_STEP_PIN, MOTOR_A_DIR_PIN); 

#define home_switch 10 // Pin 10 connected to Reed Switch (Limit Switch)


// Stepper Travel Variables
long TravelX;  // Used to store the X value entered in the Serial Monitor
long TravelAvailable;
int move_finished=1;  // Used to check if move is completed
long initial_homing=-1;  // Used to Home Stepper at startup


void setup() {
   stepperX.setEnablePin(MOTOR_A_ENABLE_PIN);
   stepperX.setPinsInverted(false, false, true);
   Serial.begin(9600);  // Start the Serial monitor with speed of 9600 Bauds
   pinMode(home_switch, INPUT_PULLUP);
   
   delay(5);  // Wait for Driver wake up

  stepperX.enableOutputs();
  pinMode(MOTOR_A_ENABLE_PIN, OUTPUT);
   
   //  Set Max Speed and Acceleration of each Steppers at startup for homing
  stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(1000.0);  // Set Acceleration of Stepper
 

Serial.println("Last known position is:");
Serial.println(storedposition);
// Home if no last position recorded (value 999999999 is stored in eeprom)

if (storedposition == 999999999) {


// Start Homing procedure of Stepper Motor at startup

  Serial.print("No Last known position! Stepper is Homing . . . . . . . . . . . ");

  while (digitalRead(home_switch)) {  // Make the Stepper move CCW until the switch is activated   
    stepperX.moveTo(initial_homing);  // Set the position to move to
    initial_homing--;  // Decrease by 1 for next move if needed
    stepperX.run();  // Start moving the stepper
  //  delay(5); you may need a delay here 
}

  stepperX.setCurrentPosition(0);  // Set the current position as zero for now
  stepperX.setMaxSpeed(2000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(500.0);  // Set Acceleration of Stepper
  initial_homing=1;

  while (!digitalRead(home_switch)) { // Make the Stepper move CW until the switch is deactivated
    stepperX.moveTo(initial_homing);  
    stepperX.run();
    initial_homing++;
    //  delay(5); you may need a delay here 
  }
  
  stepperX.setCurrentPosition(0);
  EEPROMWritelong(address, 0);
  Serial.println("Homing Completed");
  Serial.println("");
  stepperX.setMaxSpeed(2000.0);      // Set Max Speed of Stepper (Faster for regular movements)
  stepperX.setAcceleration(500.0);  // Set Acceleration of Stepper



}
// Print out Instructions on the Serial Monitor at Start
  Serial.println("Enter Travel distance (Positive for CW / Negative for CCW and Zero for back to Home): ");
  EEPROMWritelong(address, 0);
}







void loop() {
TravelX= Serial.parseInt();
while (Serial.available()==0) {  
}
if (move_finished == 1) { // Check if values are available in the Serial Buffer

  TravelX= Serial.parseInt();  // Put numeric value from buffer in TravelX variable
    Serial.print("Moving stepper into position: ");
    Serial.println(TravelX);
    stepperX.moveTo(TravelX);  // Set new moveto position of Stepper
     // Set variable for checking move of the Stepper
  
 delay(1000);  // Wait 1 seconds before moving the Stepper
  }


//  if (TravelX >= 0 && TravelX <= 1350) {

// Check if the Stepper has reached desired position
  while ((stepperX.distanceToGo() != 0)) {
  //  Serial.println(TravelX);
    stepperX.run();  // Move Stepper into position
    move_finished=0;
      }

// If move is completed display message on Serial Monitor
  if ((move_finished == 0) && (stepperX.distanceToGo() == 0)) {
    Serial.println("COMPLETED!");
   Serial.println("");
    Serial.println("Enter Travel distance (Positive for CW / Negative for CCW and Zero for back to Home): ");
   move_finished=1;  // Reset move variable
  }
//  }
}
