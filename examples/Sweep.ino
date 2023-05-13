#include <ESP32C3_Servo.h>

Servo myservo;  

int pos = 0;    // variable to store the servo position

int servoPin = 5;

void setup() {
  myservo.attach(servoPin);   // attaches the servo on pin 5 to the servo object
                              // using default min/max of 500us and 2500us
                              // different servos may require different min/max settings
                              // for an accurate 0 to 180 sweep
}

void loop() {
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

