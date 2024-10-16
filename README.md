# Smart Home Automation System

This repository contains the source code and documentation for a Smart Home Automation System developed as part of a university team project. Our system integrates modern IoT technology to enhance home security, air conditioning, water conservation, and energy efficiency.

## Key Features

1. RFID and Keypad-Based Access Control: Enhances home security by providing personalized access control using RFID technology.
2. Air conditioning System: Utilizes the AC on temperature values and the user can see the values.
3. Water management system: Detects potential water levels using water level and water flow sensors, helping mitigate damage and prevent water waste.
4. Automated Lighting Control: This system adjusts indoor lighting based on natural light levels using light sensors, reducing energy consumption.

## System Architecture

Our system utilizes the following components:
- Microcontroller/Hub: Arduino Mega
- Sensors: RFID reader, Motion sensor, Water leak sensors, Light sensors
- Actuators: Relays for lights, Alarms
- Communication: TX RX communication

## Project Structure

- `src/`: Contains the source code for the project.
- `docs/`: Documentation related to system architecture, and sensor wiring.
-      Note that this wiring can be changed following the source code to the pin outputs and inputs... This can give you a basic idea...
- `tests/`: Unit tests for key functionalities of the system.

## Getting Started

### Prerequisites

- Microcontroller Arduino Mega
- RFID reader
- 4*4 Keypad
- Motion sensor (PIR)
- Door Sensor (Magnetic Proximity)
- Water level sensor
- Water Flow sensor
- Light depending sensor module (LDR Module)
- Temperature and Humidity Sensor (DHT11)
- Low-level trigger Buzzer Module
- Relay module (5v)

 Here’s how the functionality can be structured and described in your `README.md` for your smart home automation system:

---

## Functionality

### I. Security System

Motion Detection & Arming:
- The system arms itself automatically when motion is detected by the PIR motion sensor (if not already armed).
- A red LED turns on to indicate that the system is armed.
- While armed, further motion briefly flashes the red LED, signaling continued detection.

Authentication:
- When motion is detected, the system requires authentication to disarm and unlock the door. It supports two authentication methods:
    1. **RFID**: The system scans for an RFID card. If a valid card is detected, the system disarms, and the door unlocks.
    2. **Keypad**: Users can enter a password. If the password matches, the system disarms and the door unlocks.
- The `*` key on the keypad can be used to reset the system.

Door Control:
- The door lock servo is managed based on successful authentication.
  - After successful RFID or password authentication, the door unlocks for 10 seconds.
  - If the door isn’t opened within that time, it automatically relocks.
  - Once the door is opened and closed, it relocks.
  - A magnetic door sensor monitors the door status (open or closed).
  - If the door doesn’t close properly after unlocking, the alarm is triggered for safety purposes.

Alarm System:
- The alarm (buzzer) is triggered in the following situations:
  - If the door fails to close properly after unlocking.
  - Additional alarm conditions can be implemented, such as sensor tampering or unauthorized access.

---

### II. Air Conditioning System

Temperature Monitoring:
- The system uses a DHT11 sensor to monitor temperature and humidity at regular intervals (every 30 seconds).

Automatic Climate Control:
- If the temperature exceeds **30°C**, the AC relay activates, turning on the AC unit to cool the area.
- Once the temperature drops below **30°C**, the AC relay deactivates, turning off the AC.

---

### III. Lighting System

Light Sensor:
- The system uses an LDR (Light Dependent Resistor) to measure ambient light levels.

Automatic Light Control:
- The system activates the lights based on two conditions:
  - When the door is opened:
    - If it's dark (LDR value below a set threshold), the system activates both the light relays.
    - If it's bright, a single light is on in the living room and other relays remain off.
  
- **Important Note**: Temperature monitoring and lighting control systems only begin functioning once the door is opened for the first time.

---

### Status Indicators

- Red LED: Indicates when the security system is armed.
- Green LED: Signals successful authentication (either via RFID or keypad).

