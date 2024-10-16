#include <SoftwareSerial.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>                                                                                          
#include <Wire.h>
#include <DHT.h>

// --- Pin Definitions ---
const int motionSensorPin = 23; // 5v
const int doorSensorPin = 11; // n GND
const int alarmPin = 8; // 3.3V
const int doorLockPin = 6; // 5V
const int rfidReaderSS = 53; // 3.3V
const int rfidReaderRST = 9; //
const int dhtPin = 7; // 3.3v
const int relayPin1 = A8; // Relay 1 (Light1) 5v
const int relayPin2 = A9; // Relay 2 (Light2) 5v
const int ldrPin = A1; // 3.3V
const int redBulbPin = 12;   // Pin for the red bulb 
const int greenBulbPin = 13; // Pin for the green bulb
const int acPin = A10;       // Pin for controlling the AC 5v
const int waterLevelSensorPin = 24; // Water level sensor 3.3V
const int waterMotorRelayPin = A11;  // Water motor relay 5v
const int waterFlow = 25; // Water flow sensor 5V

// --- RFID Setup ---
MFRC522 rfid(rfidReaderSS, rfidReaderRST); // Create an MFRC522 object

// --- DHT Sensor Setup ---
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE); // Initialize DHT sensor

// --- Keypad Setup ---
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {30, 31, 32, 33}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {34, 35, 36, 37}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Servo Setup ---
Servo doorLockServo;

// --- Constants and Timing ---
const unsigned long doorUnlockDuration = 10000;  // 10 seconds for door lock
const unsigned long tempCheckInterval = 30000;   // 30 seconds
const unsigned long waterCheckInterval = 10000;  // 10 seconds
const int ldrThreshold = 600;                    // LDR threshold (calibrate based on real-world usage)

//   ---- Flags ---
bool monitoringActive = false;  // Flag to activate monitoring after first door unlock

// Define threshold values for water levels 
int lowLevelThreshold = 900;    // Water level below this indicates low level
int highLevelThreshold =970;   // Water level above this indicates high level


// --- Global Variables ---
bool systemArmed = false;
bool userAuthenticated = false;
bool rfidCheckMessagePrinted = false;
bool enteringPassword = false;
bool doorOpened = false;
bool waterMonitoringActive = false;
bool lastLDRState = false;

String enteredPassword = "";            
String validRFIDCardID = "659C8A3F";   // Change this RFID card identifire as you want
String correctPassword = "1234";       // Change this keypad password as you want

unsigned long doorUnlockTime = 0;
unsigned long lastTempReadingTime = 0;
unsigned long lastWaterCheckTime = 0;


// --- Function Prototypes ---
void checkMotionSensor();
void controlDoorLock();
void authenticateRFID();
void authenticateKeypad();
void resetSystem();
void beep(int numBeeps, int duration = 100);
void unlockDoor();
void lockDoor();
void monitorTemperature();
void monitorWaterLevel();
void fireAlarm();
void controlRelays();
void updateBulbStates();
void handleAuthentication();
void manageDoorLockTiming();
void activateTemperatureAndLightControl();
void checkWaterFlow();

// --- Setup Function ---
void setup()
{
    Serial.begin(115200);

    // Initialize Pins
    pinMode(alarmPin, OUTPUT);
    pinMode(motionSensorPin, INPUT);
    pinMode(doorSensorPin, INPUT_PULLUP);
    pinMode(relayPin1, OUTPUT);
    pinMode(relayPin2, OUTPUT);
    pinMode(ldrPin, INPUT);
    pinMode(redBulbPin, OUTPUT);
    pinMode(greenBulbPin, OUTPUT);
    pinMode(acPin, OUTPUT);
    pinMode(waterMotorRelayPin, OUTPUT);  // Water motor relay
    pinMode(waterLevelSensorPin, INPUT);  // Water level sensor

    // Initialize Outputs to Default States
    digitalWrite(alarmPin, HIGH);
    digitalWrite(relayPin1, LOW);
    digitalWrite(relayPin2, LOW);
    digitalWrite(redBulbPin, LOW);
    digitalWrite(greenBulbPin, LOW);
    digitalWrite(waterMotorRelayPin, LOW);  // Ensure motor is off

    // Initialize Servo
    doorLockServo.attach(doorLockPin);
    doorLockServo.write(0);  // Lock the door initially

    // Initialize RFID
    SPI.begin();
    rfid.PCD_Init();
    Serial.println("RFID reader initialized");

    // Initialize DHT Sensor
    dht.begin();
    Serial.println("DHT Sensor initialized");

    Serial.println("System ready.");
}

// --- Main Loop Function ---
void loop()
{
    if (!doorOpened)
    {
        checkMotionSensor();  // Only check motion sensor when the door is not opened
    }

    manageDoorLockTiming();  // Manage door lock timing (automatically lock door after 10 seconds)

    // Handle authentication ONLY when the system is armed and NOT authenticated
    if (systemArmed && !userAuthenticated)
    {
        handleAuthentication();  // Handles RFID or keypad authentication
    }

    // Start monitoring after the door has been unlocked for the first time
    if (monitoringActive)
    {
        // Monitor temperature every 30 seconds
        if (millis() - lastTempReadingTime >= tempCheckInterval)
        {
            monitorTemperature();
            lastTempReadingTime = millis();
        }

        // Monitor water level every 10 seconds
        if (millis() - lastWaterCheckTime >= waterCheckInterval)
        {
            monitorWaterLevel();
            lastWaterCheckTime = millis();
        }

        // Lighting and Fan Control
        controlRelays();

        // --- Print key press from the keypad ---
        char key = keypad.getKey();  
        if (key) {  
        Serial.print("Key pressed: ");
        Serial.println(key);  // Print the key to the Serial Monitor
    }
    }
}


// --- Function Implementations ---

// 1. **Motion Detection and System Arming**
void checkMotionSensor()
{
    if (!doorOpened && digitalRead(motionSensorPin) == HIGH)
    {  // Motion detected
        if (!systemArmed)
        {
            Serial.println("Motion detected: System armed");
            systemArmed = true;
            rfidCheckMessagePrinted = false;
            updateBulbStates();  // Turn on red bulb
        }
        else
        {
            // Indicate motion while already armed
            digitalWrite(redBulbPin, LOW);
            delay(100);
            digitalWrite(redBulbPin, HIGH);
            delay(100);
        }
    }
}

// 2. **User Authentication Process**
void handleAuthentication()
{
    if (!enteringPassword && !rfidCheckMessagePrinted)
    {
        Serial.println("Waiting for RFID or Keypad authentication...");
        rfidCheckMessagePrinted = true;
    }

    // Attempt RFID Authentication
    authenticateRFID();

    // Attempt Keypad Authentication
    char key = keypad.getKey();
    if (key)
    {
        if (key == '*')
        {
            resetSystem();
        }
        else
        {
            enteringPassword = true;
            if (key == '#')
            {
                authenticateKeypad();
                enteredPassword = "";  // Reset password after authentication attempt
                enteringPassword = false;
                rfidCheckMessagePrinted = false;
            }
            else
            {
                enteredPassword += key;  // Append key to entered password
            }
        }
    }
}

// 3. **RFID Authentication**
void authenticateRFID()
{
    if (rfid.PICC_IsNewCardPresent())
    {
        if (rfid.PICC_ReadCardSerial())
        {
            String cardID = "";
            for (byte i = 0; i < rfid.uid.size; i++)
            {
                if (rfid.uid.uidByte[i] < 0x10)
                {
                    cardID += "0";  // Leading zero for single hex digit
                }
                cardID += String(rfid.uid.uidByte[i], HEX);
            }
            cardID.toUpperCase();
            Serial.print("RFID Tag detected: ");
            Serial.println(cardID);

            if (cardID == validRFIDCardID)
            {
                Serial.println("RFID Authentication successful");
                userAuthenticated = true;
                rfid.PICC_HaltA();
                controlDoorLock();
                beep(5);
            }
            else
            {
                Serial.println("Invalid RFID Tag");
                beep(3);
            }
        }
    }
}

// 4. **Keypad Authentication**
void authenticateKeypad()
{
    Serial.print("Entered password: ");
    Serial.println(enteredPassword);

    if (enteredPassword == correctPassword)
    {
        Serial.println("Keypad Authentication successful");
        userAuthenticated = true;
        controlDoorLock();
        beep(2);
    }
    else
    {
        Serial.println("Incorrect password");
        beep(4);
        enteredPassword = "";  // Reset password on failure
    }
}

// 5. **Temperature Monitoring and AC Control**
void monitorTemperature()
{
    float temperature = dht.readTemperature();

    if (isnan(temperature))
    {
        Serial.println("Failed to read temperature");
        return;
    }

    Serial.print("Current temperature: ");
    Serial.print(temperature);
    Serial.println("°C");

    if (temperature >= 28)
    {
        Serial.println("Temperature is high, turning on the AC.");
        digitalWrite(acPin, HIGH);  // Turn on AC
    }
    else
    {
        Serial.println("Temperature is normal, turning off the AC.");
        digitalWrite(acPin, LOW);   // Turn off AC
    }
}

// 6. **Water Level Monitoring and Motor Control**
void monitorWaterLevel() {
    // Read the water level sensor value from the analog pin
    int waterLevel = analogRead(waterLevelSensorPin);  // Adjust the pin number if necessary

    // For debugging: print the water level value
    // Serial.print("Water Level Sensor Value: ");
    // Serial.println(waterLevel);
    
    // Map the value to a percentage or any custom scale if needed
    int waterLevelPercent = map(waterLevel, 900, 970, 0, 100);
    Serial.print("Water Level (Raw): ");
    Serial.print(waterLevel);  // Print the raw analog value (0-1023)
    
    Serial.print(" | Water Level (Percent): ");
    Serial.print(waterLevelPercent);  // Print the percentage value (0-100%)
    
    Serial.println("%");
  

  delay(3500);  // Wait for half a second before the next reading

    
    // Check water level and update system behavior accordingly
    if (waterLevel < lowLevelThreshold) {
        // Low water level detected
        Serial.println("Water level is too low! Turning on the water motor.");
        digitalWrite(waterMotorRelayPin, HIGH);  // Turn off the water motor
        beep(2);  // Optional: Beep 2 times to indicate a low water level
    } 
    else if (waterLevel > highLevelThreshold) {
        // High water level detected
        Serial.println("Water level is high. No need to turn on the water motor.");
        digitalWrite(waterMotorRelayPin, LOW);  // Turn on the water motor
    } 
    else {
        // Water level is in the normal range
        Serial.println("Water level is in the normal range.");
    }
}



// 7. **Door Lock Control**
void controlDoorLock()
{
    if (userAuthenticated)
    {
        unlockDoor();
        doorUnlockTime = millis();  // Record the time the door was unlocked
        monitoringActive = true;    // Activate monitoring after first door unlock
    }
}


// Unlock the door by moving the servo to the "unlocked" position
void unlockDoor()
{
    Serial.println("Unlocking the door...");
    doorLockServo.write(90);  // Adjust the angle as needed for your servo
    doorOpened = true;
    updateBulbStates();       // Turn on green bulb
}

// Lock the door by moving the servo to the "locked" position
void lockDoor()
{
    Serial.println("Locking the door...");
    doorLockServo.write(0);   // Adjust the angle as needed for your servo
    doorOpened = false;
    updateBulbStates();       // Turn on red bulb
}

// 8. **Door Unlock Timing Management**
void manageDoorLockTiming()
{
    if (doorOpened)
    {
        // Check if the door unlock duration (10 seconds) has passed
        if (millis() - doorUnlockTime > doorUnlockDuration)
        {
            // Check the magnetic sensor to see if the door is still open
            if (digitalRead(doorSensorPin) == HIGH)  // Door is still open
            {
                Serial.println("Warning: Door is still open! Starting continuous beeping sequence.");
                while (digitalRead(doorSensorPin) == HIGH)
                {
                    // Beep continuously as long as the door is still open
                    beep(1, 200);  // Beep every 200ms
                    delay(200);
                }

                // Once the door is closed, lock it
                Serial.println("Door closed, locking the door.");
                lockDoor();  
                systemArmed = false;        // Disarm the system after locking
                userAuthenticated = false;  // Reset user authentication status
                rfidCheckMessagePrinted = false;  // Ready to display RFID check message again
            }
            else
            {
                // If the door is closed within the unlock duration, lock it normally
                Serial.println("Door closed, locking the door.");
                lockDoor();
                systemArmed = false;        // Disarm the system after locking
                userAuthenticated = false;  // Reset user authentication status
                rfidCheckMessagePrinted = false;  // Ready to display RFID check message again
            }
        }
    }
}



// 9. **Relay Control for Lights and Fan based on LDR Sensor**
void controlRelays() 
{
    // Read the LDR sensor value once
    int ldrValue = analogRead(ldrPin);  
    // Serial.print("LDR Value: ");
    // Serial.println(ldrValue);
    
    // Determine current LDR state based on the threshold
    bool currentLDRState = (ldrValue < ldrThreshold);  // Use threshold to determine light or dark

    // Check for a change in the LDR state (dark to light or light to dark)
    if (currentLDRState && lastLDRState == false)  // Dark detected
    {
        Serial.println("It's bright. Turning off the light1 and light2.");
        digitalWrite(relayPin1, HIGH);  // Turn on relay 1 (light1)
        digitalWrite(relayPin2, LOW);  // Turn on relay 2 (light2)
    } 
    else if (!currentLDRState && lastLDRState == true)  // Bright detected
    {
        Serial.println("It's dark. Turning on the light1 and light2.");
        digitalWrite(relayPin1, HIGH);   // Turn off relay 1 (light1)
        digitalWrite(relayPin2, HIGH);   // Turn off relay 2 (light2)
    }

    // Update last LDR state for the next comparison
    lastLDRState = currentLDRState;
}


// 10. **Bulb State Management**
void updateBulbStates()
{
    if (doorOpened)
    {
        digitalWrite(greenBulbPin, HIGH);  // Green bulb on
        digitalWrite(redBulbPin, LOW);     // Red bulb off
    }
    else
    {
        digitalWrite(greenBulbPin, LOW);   // Green bulb off
        digitalWrite(redBulbPin, HIGH);    // Red bulb on
    }
}

// 11. **Alarm Triggering**
void fireAlarm()
{
    Serial.println("Fire detected! Triggering alarm.");
    digitalWrite(alarmPin, LOW);  // Trigger the alarm
}

// 12. **Beep for Incorrect Authentication Attempts**
void beep(int numBeeps, int duration)
{
    for (int i = 0; i < numBeeps; i++)
    {
        digitalWrite(alarmPin, LOW);  // Buzzer ON
        delay(duration);
        digitalWrite(alarmPin, HIGH); // Buzzer OFF
        delay(duration);
    }
}

// 13. **System Reset Function**
void resetSystem()
{
    Serial.println("System reset.");

    // Disarm the system and reset all flags and states
    systemArmed = false;               // Disarm the system
    userAuthenticated = false;         // Reset user authentication
    monitoringActive = false;          // Stop monitoring
    waterMonitoringActive = false;     // Stop water monitoring
    doorOpened = false;                // Reset door status
    enteredPassword = "";              // Clear entered password
    rfidCheckMessagePrinted = false;   // Reset RFID message flag
    lastLDRState = false;              // Reset LDR state

    // Turn off water motor
    Serial.println("Stopping water motor...");
    digitalWrite(waterMotorRelayPin, LOW);  // Ensure motor is off

    // Turn off lights
    Serial.println("Turning off lights...");
    digitalWrite(relayPin1, LOW);      // Turn off light 1
    digitalWrite(relayPin2, LOW);      // Turn off light 2

    // Turn off AC
    Serial.println("Turning off air conditioning...");
    digitalWrite(acPin, LOW);          // Turn off the AC

    // Lock the door (ensuring system starts with the door locked)
    Serial.println("Locking the door...");
    lockDoor();

    // Update bulb states to reflect system disarmed (red bulb on, green bulb off)
    updateBulbStates();

    // Reset timing variables
    doorUnlockTime = 0;
    lastTempReadingTime = 0;
    lastWaterCheckTime = 0;

    // System now goes back to initial state: waiting for motion to arm itself
    Serial.println("System is reset and disarmed. Waiting for motion detection to arm.");
}



// 14. **Check Door Sensor and Handle Beeping if Door is Left Open**
void checkDoorSensorAndHandleBeeping()
{
    if (digitalRead(doorSensorPin) == LOW)  // Assuming LOW means the door is still open
    {
        Serial.println("Door is still open after 10 seconds.");
        beep(10);  // Beep 10 times if the door is still open
    }
    
    // Wait for the door to close
    while (digitalRead(doorSensorPin) == LOW)
    {
        // Wait in this loop until the door is closed (doorSensorPin becomes HIGH)
    }

    // Once the door is closed, lock the door
    Serial.println("Door is now closed. Locking the door.");
    lockDoor();  // Call the function to lock the door using the servo motor
}

// 15. **Check Water Flow and Turn Off Motor if Flow is Below Threshold**
void checkWaterFlow() {
    // Turn on the water motor
    digitalWrite(waterMotorRelayPin, HIGH);  // Turn on the water motor
    Serial.println("Water motor turned on.");

    // Wait for 2 seconds before checking the water flow
    delay(2000);

    // Now check the water flow after the motor is on
    int waterFlow = analogRead(waterFlow);
      // Read the flow sensor value on pin 26

    if (waterFlow < 1024) {  // No water flow detected
        Serial.println("No water flow detected! Turning off the water motor.");
        digitalWrite(waterMotorRelayPin, LOW);  // Turn off the water motor
        beep(3);  // Optional: Beep 3 times to indicate the motor is turned off
    } else {
        Serial.print("Water flow detected: ");
        Serial.println(waterFlow);  // Print the water flow value for monitoring
    }
}


