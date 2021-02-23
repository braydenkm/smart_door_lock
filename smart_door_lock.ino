/*
CME/EE 495 - Capstone Design
Bluetooth Smart Door Lock
Created:      22.02.2021
Last Updated: 22.02.2021 

Group 14
  Jackson Romanchuk - 11233901 - jwr920
  Josh Bernier      - 11233918 - jdb145
  Nick Anderson     - 11981226 - nwa764
  Brayden Martin    - 11232114 - bkm257
*/

// LIBRARIES
#include <EEPROM.h>   // For storing non-volatile data.


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


// VARIABLES / CONSANTS
State lock_state;

#define LED               13
#define TEST_INPUT        2
#define MOTOR_RED         5
#define MOTOR_BLACK       6
#define MOTOR_SPIN_TIME   3000    // TODO Figure out actual time required.
#define STATE_ADDRESS     0


// FUNCTIONS
// Set up device.
void setup() {
  // Set baud rate for debugging.
  Serial.begin(9600);

  // Set pin directions.
  pinMode(TEST_INPUT, INPUT);
  pinMode(MOTOR_RED, OUTPUT);
  pinMode(MOTOR_BLACK, OUTPUT);
  pinMode(LED, OUTPUT);

  // Initialize bluetooth.
  init_bluetooth();

  // Load default UNLOCKED state.
  lock_state = UNLOCKED;
  
  // Load state from memory if available.
  int stored_state = -1;
  EEPROM.get(STATE_ADDRESS, stored_state);
  if (0 <= stored_state && stored_state <= 3)
    lock_state = stored_state;
}


// Main loop.
// Enter state function for the matching lock_state.
void loop() {
  debug_text();
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
  digitalWrite(MOTOR_RED, HIGH);
  digitalWrite(MOTOR_BLACK, LOW);
  
  // Wait 3 seconds.
  delay(MOTOR_SPIN_TIME);

  // Stop the motor.
  digitalWrite(MOTOR_RED, LOW);

  // TODO visual and audio feedback.

  lock_state = LOCKED;
  EEPROM.put(STATE_ADDRESS, LOCKED);
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
  digitalWrite(MOTOR_RED, LOW);
  digitalWrite(MOTOR_BLACK, HIGH);
  
  // Wait 3 seconds.
  delay(MOTOR_SPIN_TIME);
  
  // Stop the motor.
  digitalWrite(MOTOR_BLACK, LOW);

  // TODO visual and audio feedback.

  lock_state = UNLOCKED;
  EEPROM.put(STATE_ADDRESS, UNLOCKED);
}


// Check for bluetooth requests and return result.
enum Request bluetooth_request() {
  if (lock_state == LOCKED) {
    // TODO replace with checking bluetooth.
    if (digitalRead(TEST_INPUT) == HIGH)
      return UNLOCK;
  }
  if (lock_state == UNLOCKED) {
    // TODO replace with checking bluetooth.
    if (digitalRead(TEST_INPUT) == HIGH)
      return LOCK;
  }
  return NONE;
}


// Initializes the bluetooth module.
void init_bluetooth() {
  // TODO set-up bluetooth.
}


// Debugging text.
// Prints out digital I/O and state.
void debug_text() {
  String temp0, temp1, temp2, temp3;
  temp0 = "in: ";
  temp1 = String(digitalRead(TEST_INPUT));
  temp2 = ", state: ";
  temp3 = String(lock_state);
  
  String debug_output = temp0 + temp1 +temp2 + temp3;
  Serial.println(debug_output);
}
