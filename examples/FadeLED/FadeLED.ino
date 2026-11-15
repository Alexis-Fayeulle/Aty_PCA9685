/*
  FadeLED - slowly brightens and dims an LED on channel 0.

  Wiring: PCA9685 channel 0 -> resistor (220 ohm) -> LED -> GND
          PCA9685 VCC -> 5V, GND -> GND, SDA -> SDA, SCL -> SCL
  Open the Serial Monitor (115200 baud) to see messages.
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

  pca.setFrequency(500);
  Serial.println(F("PCA9685 found, fading the LED on channel 0..."));
}

void loop() {
  for (int percent = 0; percent <= 100; percent++) {  // brighter
    pca.setDutyPercent(0, percent);
    delay(15);
  }
  for (int percent = 100; percent >= 0; percent--) {  // dimmer
    pca.setDutyPercent(0, percent);
    delay(15);
  }
}
