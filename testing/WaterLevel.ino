const int waterSensorPin = 24;  // Define the analog pin for the water sensor

void setup() {
  Serial.begin(9600);           // Start serial communication at 9600 baud
}

void loop() {
  int waterLevel = analogRead(waterSensorPin);  // Read the analog value from the sensor

  // If the sensor reads 1023, it could mean it's fully submerged or there's an issue
  if (waterLevel == 1023) {
    Serial.println("Warning: Sensor is reading maximum value (1023). Check water level or sensor connection.");
  } else {
    // Map the value to a percentage or any custom scale if needed
    int waterLevelPercent = map(waterLevel, 900, 970, 0, 100);
    Serial.print("Water Level (Raw): ");
    Serial.print(waterLevel);  // Print the raw analog value (0-1023)
    
    Serial.print(" | Water Level (Percent): ");
    Serial.print(waterLevelPercent);  // Print the percentage value (0-100%)
    
    Serial.println("%");
  }

  delay(3500);  // Wait for half a second before the next reading
}
