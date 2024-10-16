#include <Servo.h>

Servo myServo;  // Create a Servo object

int servoPin = 6; // Define the pin where the servo is connected
int angle = 0;    // Variable to store the servo angle

void setup() {
  myServo.attach(servoPin); // Attach the servo on pin 9 to the Servo object
}

void loop() {
  // Sweep from 0 to 180 degrees
  for (angle = 0; angle <= 180; angle += 1) {
    myServo.write(angle);     // Set the servo to the current angle
    delay(15);                // Wait 15ms to allow the servo to reach the position
  }
  
  // Sweep from 180 to 0 degrees
  for (angle = 180; angle >= 0; angle -= 1) {
    myServo.write(angle);     // Set the servo to the current angle
    delay(15);                // Wait 15ms to allow the servo to reach the position
  }
}
