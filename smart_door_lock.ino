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
#include <SoftwareSerial.h>   // Serial communication of blutetooth.


// STRUCTS / ENUM
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
#define TEST_INPUT        6
#define RX                0
#define TX                1
#define MOTOR_ENABLE      4
#define MOTOR_RED         3
#define MOTOR_BLACK       2
#define MOTOR_SPIN_TIME   2000    // TODO Figure out actual time required.
#define STATE_ADDRESS     0


// VARIABLES
State lock_state;
SoftwareSerial serial = SoftwareSerial(RX, TX);


// FUNCTIONS
// Set up device.
void setup() {
  // Initialize bluetooth.
  init_bluetooth();

  // Set pin directions.
  pinMode(MOTOR_ENABLE, OUTPUT);
  pinMode(MOTOR_RED,    OUTPUT);
  pinMode(MOTOR_BLACK,  OUTPUT);
  pinMode(LED,          OUTPUT);
  pinMode(TX,           OUTPUT);
  pinMode(RX,           INPUT);
//  pinMode(TEST_INPUT,   INPUT);

  // Load default UNLOCKED state.
  lock_state = UNLOCKED;
  
  // Load state from memory if available.
  int stored_state = -1;
  EEPROM.get(STATE_ADDRESS, stored_state);
  if (0 <= stored_state && stored_state <= 3){
    switch(stored_state) {
      case 0: lock_state = LOCKED;     break;
      case 1: lock_state = LOCKING;    break;
      case 2: lock_state = UNLOCKED;   break;
      case 3: lock_state = UNLOCKING;  break;
    }
  }
}


// Main loop.
// Enter state function for the matching lock_state.
void loop() {
//  debug_text();
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
  digitalWrite(LED, HIGH);
  if (bluetooth_request() == UNLOCK)
    lock_state = UNLOCKING;
}


// Locking state.
// Loop through this function while lock_state == locking.
void locking() {
  // Spin the motor to unlock.
  digitalWrite(MOTOR_ENABLE, HIGH);
  digitalWrite(MOTOR_BLACK, LOW);
  digitalWrite(MOTOR_RED, HIGH);
  
  // Wait 3 seconds.
  delay(MOTOR_SPIN_TIME);

  // Stop the motor.
  digitalWrite(MOTOR_BLACK, LOW);
  digitalWrite(MOTOR_RED, LOW);
  digitalWrite(MOTOR_ENABLE, LOW);

  // TODO visual and audio feedback.

  lock_state = LOCKED;
  EEPROM.put(STATE_ADDRESS, lock_state);
}


// Unlocked state.
// Loop through this function while lock_state == unlocked.
void unlocked() {
  digitalWrite(LED, LOW);
  if (bluetooth_request() == LOCK)
    lock_state = LOCKING;
}


// Unlocking state.
// Loop through this function while lock_state == unlocking.
void unlocking() {
  // Spin the motor to unlock.
  digitalWrite(MOTOR_ENABLE, HIGH);
  digitalWrite(MOTOR_BLACK, HIGH);
  digitalWrite(MOTOR_RED, LOW);
  
  // Wait 3 seconds.
  delay(MOTOR_SPIN_TIME);
  
  // Stop the motor.
  digitalWrite(MOTOR_BLACK, LOW);
  digitalWrite(MOTOR_RED, LOW);
  digitalWrite(MOTOR_ENABLE, LOW);

  // TODO visual and audio feedback.

  lock_state = UNLOCKED;
  EEPROM.put(STATE_ADDRESS, lock_state);
}


// Check for bluetooth requests and return result.
enum Request bluetooth_request() {
//  if (digitalRead(TEST_INPUT) == HIGH) {
//    serial.print(lock_state);
//    return (lock_state == LOCKED) ? UNLOCK : LOCK;
//  }

  if (serial.available() != 0){
    char c = serial.read();
    if (c == 'a') {
      // Clear the buffer.
      while(serial.available() != 0)
        serial.read();
      serial.print(lock_state);
      if (lock_state == LOCKED) return UNLOCK;
      return LOCK;
    }
  }
      
  return NONE;
}


// Initializes the bluetooth module.
void init_bluetooth() {
  // Set baud rate for debugging.
//  Serial.begin(9600);

  // Set baud rate for serial port.
  serial.begin(9600);
}


// Debugging text.
// Prints out digital I/O and state.
void debug_text() {
  String debug_string =
    "in: " + String(digitalRead(TEST_INPUT)) + ", state: " + String(lock_state);
  Serial.println(debug_string);
}
