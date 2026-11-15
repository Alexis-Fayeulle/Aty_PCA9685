/*
  BasicPWM - the simplest possible example.

  Wiring (Arduino Uno / Nano / Mega):
    PCA9685 VCC -> 5V        PCA9685 GND -> GND
    PCA9685 SDA -> SDA       PCA9685 SCL -> SCL
    (Uno/Nano: SDA = A4, SCL = A5. Mega: SDA = 20, SCL = 21.)

  Open the Serial Monitor (115200 baud) to see what the board is doing.
  Plug an LED + resistor (or a motor driver) on channel 0 to see something.
*/

#include <Aty_PCA9685.h>

Aty_PCA9685 pca;  // chip at the default address 0x40

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting the PCA9685..."));

  if (!pca.begin()) {
    Serial.println(F("PCA9685 Error! The board does not answer."));
    Serial.println(F("  1. Check the 4 wires: VCC, GND, SDA, SCL (SDA and SCL swapped?)"));
    Serial.println(F("  2. Check that the board is powered (its LED should be on)."));
    Serial.println(F("  3. Run the FindAddress example to see the real address."));
    while (true) {}  // stop here
  }
  Serial.println(F("PCA9685 found."));

  float real = pca.setFrequency(1000);  // speed of the PWM signal, in Hz (24 to 1526)
  if (real == 0) {
    Serial.println(F("PCA9685 Error! Could not set the frequency (connection lost?)."));
    while (true) {}
  }
  Serial.print(F("PWM frequency: "));
  Serial.print(real);
  Serial.println(F(" Hz"));

  pca.setDutyPercent(0, 50);  // channel 0 at 50 %
  pca.setDutyPercent(1, 25);  // channel 1 at 25 %
  pca.fullOn(2);              // channel 2 always ON  (100 %)
  pca.fullOff(3);             // channel 3 always OFF (0 %)
  Serial.println(F("Outputs set: 0 = 50 %, 1 = 25 %, 2 = ON, 3 = OFF."));
}

void loop() {
  // nothing to do: the chip keeps the PWM running by itself
}
