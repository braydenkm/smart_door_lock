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


// VARIABLES
State lock_state;
const int motor_red = 5;
const int motor_black = 6;
const int motor_spin_time = 3000; // TODO Figure out actual time required.


// FUNCTIONS
// Set up device.
void setup() {
  lock_state = LOCKED;
  init_bluetooth();
  pinMode(motor_red, OUTPUT);
  pinMode(motor_black, OUTPUT);
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
  // TODO check for bluetooth unlock request.
  if (bluetooth_request() == UNLOCK)
    lock_state = UNLOCKING;
}


// Locking state.
// Loop through this function while lock_state == locking.
void locking() {
  // Spin the motor to unlock.
  digitalWrite(motor_red, HIGH);
  digitalWrite(motor_black, LOW);
  
  // Wait 3 seconds.
  delay(motor_spin_time);

  // Stop the motor.
  digitalWrite(motor_red, LOW);

  // TODO visual and audio feedback.

  lock_state = LOCKED;
}


// Unlocked state.
// Loop through this function while lock_state == unlocked.
void unlocked() {
  // TODO check for bluetooth lock request.
  if (bluetooth_request() == LOCK)
    lock_state = LOCKING;
}


// Unlocking state.
// Loop through this function while lock_state == unlockingd.
void unlocking() {
  // Spin the motor to unlock.
  digitalWrite(motor_red, LOW);
  digitalWrite(motor_black, HIGH);
  
  // Wait 3 seconds.
  delay(motor_spin_time);
  
  // Stop the motor.
  digitalWrite(motor_black, LOW);

  // TODO visual and audio feedback.

  lock_state = UNLOCKED;
}


// Check for bluetooth requests and return result.
enum Request bluetooth_request() {
  // TODO (TEMP), replace with working conditions after bluetooth is set-up.
  bool locking_condition = false;
  bool unlocking_condition = false;
  if (locking_condition)
    return LOCK;
  if (unlocking_condition)
    return UNLOCK;
  return NONE;
}


// Initializes the bluetooth module.
void init_bluetooth() {
  // TODO set-up bluetooth.
}
