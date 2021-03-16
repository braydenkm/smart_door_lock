/*
CME/EE 495 - Capstone Design
Bluetooth Smart Door Lock
Created:      22.02.2021
Last Updated: 15.03.2021 

Group 14
  Jackson Romanchuk - 11233901 - jwr920
  Josh Bernier      - 11233918 - jdb145
  Nick Anderson     - 11981226 - nwa764
  Brayden Martin    - 11232114 - bkm257
*/

// LIBRARIES
#include <EEPROM.h>           // Storing non-volatile data.
#include <SoftwareSerial.h>   // phone communication of blutetooth.


// ENUMS
enum State {
  LOCKED,
  LOCKING,
  UNLOCKED,
  UNLOCKING
};

enum Request {
  LOCK,
  UNLOCK,
  NONE
};


// CONSTANTS
#define LED               13
#define MOTOR_ENABLE      4
#define MOTOR_RED         3
#define MOTOR_BLACK       2
#define TX                1
#define RX                0

#define MOTOR_DELAY       2000    // milliseconds
#define STATE_ADDRESS     0


// VARIABLES
State lock_state;
SoftwareSerial phone = SoftwareSerial(RX, TX);
unsigned long start_of_delay;


// FUNCTIONS
// Set up device.
void setup() {
  // Set baud rate for phone port.
  phone.begin(9600);

  // Set pin directions.
  pinMode(MOTOR_ENABLE, OUTPUT);
  pinMode(MOTOR_RED,    OUTPUT);
  pinMode(MOTOR_BLACK,  OUTPUT);
  pinMode(LED,          OUTPUT);
  pinMode(TX,           OUTPUT);
  pinMode(RX,           INPUT);

  // Load default UNLOCKED state.
  lock_state = UNLOCKED;
  
  // Load state from memory if available.
  unsigned short stored_state = 8;
  EEPROM.get(STATE_ADDRESS, stored_state);
  if (stored_state <= 3){
    switch(stored_state) {
      case 0: lock_state = LOCKED;     break;
      case 1: lock_state = LOCKING;    break;
      case 2: lock_state = UNLOCKED;   break;
      case 3: lock_state = UNLOCKING;  break;
    }
  }

  // Set start of delay
  start_of_delay = millis();
}


// Main loop.
// Enter state function for the matching lock_state.
void loop() {
  switch(lock_state) {
    case LOCKED:    locked();     break;
    case LOCKING:   locking();    break;
    case UNLOCKED:  unlocked();   break;
    case UNLOCKING: unlocking();  break;
  }
}


// Locked state.
// Loop through this function while lock_state == locked.
void locked() {
  if (bluetooth_request() == UNLOCK){
    lock_state = UNLOCKING;
    start_of_delay = millis();
    phone.print("UNLOCKING\n");
  }
}


// Locking state.
// Loop through this function while lock_state == locking.
void locking() {
  // Spin the motor to unlock.
  digitalWrite(MOTOR_BLACK, LOW);
  digitalWrite(MOTOR_RED, HIGH);
  digitalWrite(MOTOR_ENABLE, HIGH);
  
  // Wait while motor spins.
  if (millis() - start_of_delay >= MOTOR_DELAY) {
    // Stop the motor.
    digitalWrite(MOTOR_BLACK, LOW);
    digitalWrite(MOTOR_RED, LOW);
    digitalWrite(MOTOR_ENABLE, LOW);
    digitalWrite(LED, HIGH);

    lock_state = LOCKED;
    phone.print("LOCKED\n");
    EEPROM.put(STATE_ADDRESS, lock_state);
  }
}


// Unlocked state.
// Loop through this function while lock_state == unlocked.
void unlocked() {
  digitalWrite(LED, LOW);
  if (bluetooth_request() == LOCK) {
    lock_state = LOCKING;
    start_of_delay = millis();
    phone.print("LOCKING\n");
  }
}


// Unlocking state.
// Loop through this function while lock_state == unlocking.
void unlocking() {
  // Spin the motor to unlock.
  digitalWrite(MOTOR_BLACK, HIGH);
  digitalWrite(MOTOR_RED, LOW);
  digitalWrite(MOTOR_ENABLE, HIGH);
  
  // Wait while motor spins.
  if (millis() - start_of_delay >= MOTOR_DELAY) {
    // Stop the motor.
    digitalWrite(MOTOR_BLACK, LOW);
    digitalWrite(MOTOR_RED, LOW);
    digitalWrite(MOTOR_ENABLE, LOW);

    lock_state = UNLOCKED;
    phone.print("UNLOCKED\n");
    EEPROM.put(STATE_ADDRESS, lock_state);
  }
}


// Check for bluetooth requests and return result.
enum Request bluetooth_request() {
  // Leave if phone input is empty.
  if (phone.available() == 0) return NONE;

  // Read in character and clear the buffer.
  char character = phone.read();
  while(phone.available() != 0) phone.read();
  
  // Leave if phone input is not 'a'
  if (character != 'a') return NONE;
  
  // Return LOCK or UNLOCK depending on current state.
  return (lock_state == LOCKED) ? UNLOCK : LOCK;
}
