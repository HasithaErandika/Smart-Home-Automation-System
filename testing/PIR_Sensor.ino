int pirPin = 23;   
int ledPin = 13;  // Onboard LED on pin 13

void setup() {
  pinMode(pirPin, INPUT);  
  pinMode(ledPin, OUTPUT); 
  Serial.begin(9600);      
}

void loop() {
  int sensorValue = digitalRead(pirPin); // Read the PIR sensor

  if (sensorValue == HIGH) { // If motion is detected
    digitalWrite(ledPin, HIGH); // Turn on LED
    Serial.println("Motion detected!"); // Print to Serial Monitor
  } else {
    digitalWrite(ledPin, LOW); // Turn off LED
    Serial.println("No motion."); // Print to Serial Monitor
  }

  delay(1000); // Delay for a second before the next read
}
