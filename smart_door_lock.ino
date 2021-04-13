/*
CME/EE 495 - Capstone Design
Bluetooth Smart Door Lock
Created:      22.02.2021
Last Updated: 13.04.2021 

Group 14
  Jackson Romanchuk - 11233901 - jwr920
  Josh Bernier      - 11233918 - jdb145
  Nick Anderson     - 11981226 - nwa764
  Brayden Martin    - 11232114 - bkm257
*/


// LIBRARIES
#include <EEPROM.h>           // Storing non-volatile data.
#include <SoftwareSerial.h>   // Phone communication with blutetooth.
#include <LowPower.h>         // Sleep mode.


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
#define MOTOR_ENABLE      4
#define MOTOR_RED         3
#define MOTOR_BLACK       5
#define TX                1
#define RX                0
#define INTERRUPT         2

#define MOTOR_DELAY       600     // 6/10 second
#define AUTO_LOCK_TIME    5000    // 5 seconds
#define STATE_ADDRESS     0


// VARIABLES
State lock_state;
SoftwareSerial phone = SoftwareSerial(RX, TX);
unsigned long motor_spin_start;
unsigned long auto_lock_start;


// FUNCTIONS
// Set up device.
void setup() {
  // Set baud rate for phone port.
  phone.begin(9600);

  // Set pin directions.
  pinMode(MOTOR_ENABLE, OUTPUT);
  pinMode(MOTOR_RED,    OUTPUT);
  pinMode(MOTOR_BLACK,  OUTPUT);
  pinMode(TX,           OUTPUT);
  pinMode(RX,           INPUT);
  pinMode(INTERRUPT,    INPUT_PULLUP);
  attachInterrupt(INTERRUPT, wakeUp, CHANGE);

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
  motor_spin_start  = millis();
  auto_lock_start   = millis();
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


// Handle the interrupt.
void wakeUp() {}


// Put the MCU into sleep mode.
void mcu_sleep() {
  phone.print("Sleep\n");
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
}


// Change into the new state and store state.
void change_state(enum State new_state) {
  lock_state = new_state;
  switch(lock_state) {
    case LOCKED:    phone.println("Locked");
                    EEPROM.put(STATE_ADDRESS, lock_state);
                    break;
    case LOCKING:   phone.println("Locking");
                    break;
    case UNLOCKED:  phone.println("Unlocked");
                    EEPROM.put(STATE_ADDRESS, lock_state);
                    break;
    case UNLOCKING: phone.println("Unlocking");
                    break;
  }
}


// Spin the motor in given direction.
void spin_motor(enum Request direction) {
  if (direction == NONE)      return;
  if (direction == LOCK)      digitalWrite(MOTOR_BLACK, HIGH);
  if (direction == UNLOCK)    digitalWrite(MOTOR_RED,   HIGH);
  digitalWrite(MOTOR_ENABLE,  HIGH);
}


// Stop spinning the motor.
void stop_motor() {
  digitalWrite(MOTOR_BLACK,   LOW);
  digitalWrite(MOTOR_RED,     LOW);
  digitalWrite(MOTOR_ENABLE,  LOW);
}


// Locked state.
// Loop through this function while lock_state == locked.
void locked() {
  if (bluetooth_request() == UNLOCK){
    change_state(UNLOCKING);
    motor_spin_start = millis();
  }
  if (millis() - auto_lock_start >= AUTO_LOCK_TIME)
    mcu_sleep();
}


// Locking state.
// Loop through this function while lock_state == locking.
void locking() {
  spin_motor(LOCK);
  
  // Wait while motor spins.
  if (millis() - motor_spin_start >= MOTOR_DELAY) {
    stop_motor();
    change_state(LOCKED);
    mcu_sleep();
  }
}


// Unlocked state.
// Loop through this function while lock_state == unlocked.
void unlocked() {
  bool lock_request = bluetooth_request() == LOCK;
  bool auto_lock_timeout = (millis() - auto_lock_start) >= AUTO_LOCK_TIME;
  
  if (lock_request || auto_lock_timeout) {
    change_state(LOCKING);
    motor_spin_start = millis();
  }
}


// Unlocking state.
// Loop through this function while lock_state == unlocking.
void unlocking() {
  spin_motor(UNLOCK);
  
  // Wait while motor spins.
  if (millis() - motor_spin_start >= MOTOR_DELAY) {
    stop_motor();
    change_state(UNLOCKED);
    auto_lock_start = millis();
  }
}


// Check for bluetooth requests and return result.
enum Request bluetooth_request() {
  char passcode[] = "open";
  const int passcode_length = sizeof(passcode) / sizeof(passcode[0]) - 1;
  
  // Leave if phone input is empty.
  if (phone.available() == 0) return NONE;

  // Leave if entered passcode is not same length.
  delay(75);
  if (phone.available() != passcode_length){
    phone.print("Incorrect\n");
    while(phone.available() != 0) phone.read();
    auto_lock_start = millis();
    return NONE;
  }

  // Read in attempt passcode.
  char attempt[passcode_length + 1];
  for (int i = 0; i < passcode_length; i++)
    if (phone.peek() != -1)
      attempt[i] = phone.read();

  // Clear the buffer.
  while(phone.available() != 0) phone.read();
  phone.println(attempt);
  
  // Leave if passcode attempt != correct passcode.
  for (int i = 0; i < passcode_length; i++) {
    if (attempt[i] != passcode[i]) {
      phone.print("Incorrect\n");
      return NONE;
    }
  }
  phone.print("Correct\n");
  
  // Return LOCK or UNLOCK depending on current state.
  return (lock_state == LOCKED) ? UNLOCK : LOCK;
}
