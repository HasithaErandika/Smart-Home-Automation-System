const int sensorPin = 11; // Pin connected to the magnetic door sensor
const int ledPin = 13;   // Pin connected to the onboard LED

void setup() {
  Serial.begin(9600);        // Initialize serial communication
  pinMode(sensorPin, INPUT); // Set sensor pin as an input
  pinMode(ledPin, OUTPUT);   // Set LED pin as an output
  digitalWrite(ledPin, LOW); // Ensure LED is off
}

void loop() {
  int sensorState = digitalRead(sensorPin); // Read the state of the magnetic door sensor

  if (sensorState == HIGH) { // Door is open
    Serial.println("Door is open");
    digitalWrite(ledPin, HIGH); // Turn on the LED
  } else { // Door is closed
    Serial.println("Door is closed");
    digitalWrite(ledPin, LOW); // Turn off the LED
  }

  delay(500); // Wait for 500 milliseconds before reading again
}
