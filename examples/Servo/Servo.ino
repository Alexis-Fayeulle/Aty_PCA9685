/*
  Servo - moves a hobby servo (SG90, MG90S, ...) on channel 0.

  Wiring: servo signal (yellow/orange) -> channel 0 "PWM" pin
          servo red   -> the V+ terminal of the board (external 5 V supply)
          servo brown -> GND
          PCA9685 VCC -> 5V, GND -> GND, SDA -> SDA, SCL -> SCL
  Do not power several servos from the Arduino 5 V pin.
  Open the Serial Monitor (115200 baud) to see messages.

  A servo wants a pulse of about 1000 us (one end), 1500 us (middle), 2000 us (other end),
  repeated 50 times per second. Some servos accept 500..2500 us, test carefully.
*/

#include <Aty_PCA9685.h>

Aty_PCA9685 pca;

void setup() {
  Serial.begin(115200);

  if (!pca.begin()) {
    Serial.println(F("PCA9685 Error! The board does not answer."));
    Serial.println(F("Check VCC, GND, SDA, SCL, or run the FindAddress example."));
    while (true) {}  // stop here
  }

  pca.setFrequency(50);  // servos expect 50 Hz
  Serial.println(F("PCA9685 found, moving the servo on channel 0..."));
}

void loop() {
  Serial.println(F("one end (1000 us)"));
  pca.setPulseMicroseconds(0, 1000);
  delay(1000);

  Serial.println(F("middle (1500 us)"));
  pca.setPulseMicroseconds(0, 1500);
  delay(1000);

  Serial.println(F("other end (2000 us)"));
  pca.setPulseMicroseconds(0, 2000);
  delay(1000);
}
