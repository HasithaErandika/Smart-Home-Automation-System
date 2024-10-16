#include <Keypad.h>

// Define the number of rows and columns on the keypad
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns

// Define the keymap, which maps each button to a character
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Define the pins for the rows and columns
byte rowPins[ROWS] = {30, 31, 32, 33}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {34, 35, 36, 37}; // Connect to the column pinouts of the keypad

// Create the Keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  // Start the serial communication to output the results
  Serial.begin(9600);
}

void loop() {
  // Get the key from the keypad
  char key = keypad.getKey();
  
  // If a key is pressed, print it to the serial monitor
  if (key) {
    Serial.println(key);
  }
}
