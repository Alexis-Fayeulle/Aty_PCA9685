/*
  FindAddress - lists every I2C device connected to the board.

  Use it when "PCA9685 not found": open the Serial Monitor (115200 baud).
  A PCA9685 usually shows up as 0x40 (and also 0x70, its "all call" address).
  If the list is empty, the problem is the wiring or the power.
*/

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println(F("Scanning the I2C bus..."));

  int found = 0;
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("Device found at 0x"));
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      found++;
    }
  }

  if (found == 0) Serial.println(F("Nothing found: check SDA, SCL, VCC and GND."));
  else Serial.println(F("Done. Use the address in: Aty_PCA9685 pca(0x40);"));
}

void loop() {}
