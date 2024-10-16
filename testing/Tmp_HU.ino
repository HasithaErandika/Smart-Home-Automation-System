#include <DHT.h>

#define DHTPIN 7    // Pin connected to the DATA pin of the sensor
#define DHTTYPE DHT11   // Define DHT type (DHT11, DHT22)

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); 
  dht.begin(); // Start the DHT sensor
}

void loop() {
  // Wait a few seconds between measurements
  delay(2000);
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a slow sensor)
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Celsius (isFahrenheit = false)
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  // Print temperature and humidity values to the Serial Monitor
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C\t");
  Serial.print("Heat Index: ");
  Serial.print(heatIndex);
  Serial.println(" °C");
}
