#include <SPI.h>
#include <SD.h>
#include <ESP8266WiFi.h>
#include <DHT.h> // Include DHT sensor library
#include <Wire.h> // Include library for I2C communication for fingerprint sensor
#include <Fingerprint.h> // Include fingerprint sensor library
#include <Keypad.h> // Include keypad library
#include <Servo.h> // Include servo library

// --- Define Pin Connections ---
// Water Management
const int waterLevelPin = A0;
const int flowSensorPin = 2;
const int pumpPin = 5;

// Lighting
const int lightSensorPin = A1;
const int ledPin = 9;

// Security
const int rfidReaderPin = 10;
const int keypadPin1 = 30;  // Row 1
const int keypadPin2 = 31;  // Row 2
const int keypadPin3 = 32;  // Row 3
const int keypadPin4 = 33;  // Row 4
const int keypadCols[4] = { 34, 35, 36, 37 }; // Keypad columns
const int doorLockPin = 6;

// Alarm
const int alarmPin = 8; 
const int doorSensorPin = 11; 
const int motionSensorPin = 23;

// SD Card
const int chipSelect = 53; 

// Wi-Fi
const char* ssid = "YOUR_WIFI_SSID"; 
const char* password = "YOUR_WIFI_PASSWORD"; 

// Data Logging Variables
File dataFile;

// --- RFID Reader Variables ---
// Example:  If using a MFRC522 RFID reader
#include <MFRC522.h>
#define RST_PIN  9           // Configured above
#define SS_PIN  44           // Configured above 
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
byte rfidUID[4]; // Array to store RFID card UID
byte rfidCardUID[4] = {0x01, 0x02, 0x03, 0x04}; // Example valid card UID

// --- Keypad Variables ---
const byte ROWS = 4; // Number of rows on the keypad
const byte COLS = 4; // Number of columns on the keypad
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {keypadPin1, keypadPin2, keypadPin3, keypadPin4}; 
byte colPins[COLS] = {keypadCols[0], keypadCols[1], keypadCols[2], keypadCols[3]};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

// --- Servo Motor ---
Servo doorLockServo;

// --- Alarm System ---
const int beepDuration = 250; // Duration of a beep sound in milliseconds

// --- Function Prototypes ---
void readRFID();
bool authenticateKeypad();

// --- Setup ---
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // Wait for serial port to be available
  }

  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("Card initialized");

  // Initialize Wi-Fi module
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize sensors and actuators
  pinMode(waterLevelPin, INPUT);
  pinMode(flowSensorPin, INPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(lightSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(rfidReaderPin, INPUT);
  pinMode(doorLockPin, OUTPUT);
  pinMode(alarmPin, OUTPUT);
  pinMode(doorSensorPin, INPUT);
  pinMode(motionSensorPin, INPUT);

  // Initialize servo for door lock
  doorLockServo.attach(doorLockPin);
  doorLockServo.write(0); // Initialize door lock in closed position

  // Initialize RFID reader
  SPI.begin(); // Initialize SPI communication
  mfrc522.PCD_Init(); // Initialize the MFRC522 RFID reader

  // Initialize keypad
  keypad.begin(makeKeymap(keys), rowPins, colPins); 

  // Set interrupt for motion sensor
  pinMode(motionSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(motionSensorPin), motionDetected, HIGH); 
}

// --- Loop ---
void loop() {
  // --- Water Management ---
  int waterLevel = analogRead(waterLevelPin);
  int flowRate = pulseIn(flowSensorPin, HIGH); 

  // --- Lighting ---
  int lightLevel = analogRead(lightSensorPin);

  // --- Security ---
  if (digitalRead(motionSensorPin) == HIGH) {
    Serial.println("Motion detected!");
    
    // Authenticate using RFID card
    readRFID(); 
    if (compareUIDs(rfidUID, rfidCardUID)) { 
      unlockDoor();
    } else { 
      // Authenticate using keypad
      if (authenticateKeypad()) {
        unlockDoor();
      }
    }
  }

  // --- Other System Logic ---
  // ... (Control water pump, lights, etc.)

  // --- Delay ---
  delay(100);
}

// --- Functions ---

// Read RFID card UID
void readRFID() {
  // Check if a card is present
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Get the card UID
    for (byte i = 0; i < 4; i++) {
      rfidUID[i] = mfrc522.uid.uidByte[i];
    }
  }
}

// Compare RFID card UID with a valid card
bool compareUIDs(byte uid1[], byte uid2[]) {
  for (byte i = 0; i < 4; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

// Authenticate using keypad
bool authenticateKeypad() {
  char password[4];
  char key = keypad.getKey();
  int index = 0;
  
  while (key != '#') { // Accept input until '#' is pressed
    if (key) {
      password[index++] = key;
      Serial.print(key);
    }
    key = keypad.getKey();
  }
  
  // Example: Check if the password is correct (replace with your actual password check)
  if (strcmp(password, "1234") == 0) {
    return true;
  } else {
    return false;
  }
}

// Unlock the door
void unlockDoor() {
  doorLockServo.write(90); // Open door (adjust angle if needed)
  
  // Beep the siren twice
  digitalWrite(alarmPin, HIGH);
  delay(beepDuration);
  digitalWrite(alarmPin, LOW);
  delay(beepDuration);
  digitalWrite(alarmPin, HIGH);
  delay(beepDuration);
  digitalWrite(alarmPin, LOW);

  // Close the door after a minute
  delay(60000); // Wait for 1 minute (60000 milliseconds)
  doorLockServo.write(0); // Close the door (adjust angle if needed)
}

// Handle motion detected
void motionDetected() {
  // Beep the siren once
  digitalWrite(alarmPin, HIGH);
  delay(beepDuration);
  digitalWrite(alarmPin, LOW); 
}