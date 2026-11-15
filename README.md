# Aty_PCA9685

Control **16 PWM outputs with only 2 wires** (SDA / SCL) using the PCA9685 board:
dim LEDs, drive servos, vary the speed of motors... No need to understand registers or I²C.

*Fonctionne avec Arduino Uno, Nano, Mega, ESP32, etc. Voir la section « En français » plus bas.*

## 1. Wiring

| PCA9685 board | Arduino Uno / Nano | Arduino Mega |
|---|---|---|
| VCC | 5V | 5V |
| GND | GND | GND |
| SDA | A4 | 20 |
| SCL | A5 | 21 |

The `V+` screw terminal powers servos / LEDs: use an external supply for anything bigger than a
small LED. Connect every GND together.

## 2. Install

Arduino IDE → **Sketch → Include Library → Manage Libraries…** → search `Aty_PCA9685` → Install.
(Or *Add .ZIP Library* if you downloaded the ZIP from GitHub.)

## 3. First sketch

File → Examples → Aty_PCA9685 → **BasicPWM**. Or copy this:

```cpp
#include <Aty_PCA9685.h>

Aty_PCA9685 pca;

void setup() {
  pca.begin();                // starts everything
  pca.setFrequency(1000);     // PWM speed in Hz
  pca.setDutyPercent(0, 50);  // output 0 at 50 %
}

void loop() {}
```

## 4. The functions you need

| What you want | Code |
|---|---|
| Start the board | `pca.begin();` (returns `false` if the board is not found) |
| Choose the PWM frequency (24 – 1526 Hz) | `pca.setFrequency(1000);` |
| Power of an output, in percent | `pca.setDutyPercent(channel, 50);` |
| Power of an output, 0 – 4095 (12 bits) | `pca.setDuty(channel, 2048);` |
| Always on / always off | `pca.fullOn(channel);` / `pca.fullOff(channel);` |
| Servo position | `pca.setFrequency(50);` then `pca.setPulseMicroseconds(channel, 1500);` |
| Same value on all 16 outputs | `pca.setAllDuty(2048);` |
| Switch off the chip / back on | `pca.sleep();` / `pca.wakeUp();` |

`channel` is the number printed on the board, **0 to 15**.
Several boards? Give each its address: `Aty_PCA9685 second(0x41);`

## 5. It does not work?

Open the **Serial Monitor** (115200 baud): every example tells you what it is doing. If you see
`PCA9685 Error! The board does not answer.`, the Arduino cannot talk to the board: check the
wiring (a swapped SDA / SCL is the most common mistake) and the power. Then:

1. Run the example **FindAddress** and open the Serial Monitor (115200 baud). You should see
   `0x40`. If nothing appears, check SDA / SCL / VCC / GND.
2. If the address is not `0x40`, use it: `Aty_PCA9685 pca(0x41);`
3. Servo jitters or nothing moves: power it from an external supply, not from the Arduino.
4. The PWM frequency is shared by **all** outputs. Servos need 50 Hz, LEDs look better above
   ~200 Hz: do not mix them on the same board.

## 6. Good to know

- The PCA9685 cannot go above **1526 Hz**, so it **cannot make the 25 kHz signal of a 4-wire PC
  fan**. A 2- or 3-wire fan through a MOSFET/driver module works fine.
- The board's oscillator is only accurate to a few percent. If you need the exact frequency, call
  `pca.setOscillatorFrequency(measured_hz)` before `setFrequency()`.
- Built from the NXP PCA9685 datasheet (Rev. 4).

## En français

- **Branchement** : VCC → 5V, GND → GND, SDA → A4 (Uno/Nano) ou 20 (Mega), SCL → A5 (Uno/Nano) ou 21 (Mega).
- **Installation** : Arduino IDE → Croquis → Inclure une bibliothèque → Gérer les bibliothèques → `Aty_PCA9685`.
- **Utilisation** : voir les exemples `BasicPWM`, `FadeLED` et `Servo` (Fichier → Exemples → Aty_PCA9685).
- **Rien ne marche ?** Lance l'exemple `FindAddress` : il affiche l'adresse du module (en général `0x40`).
- **Limite** : fréquence de 24 à 1526 Hz seulement, donc pas de PWM 25 kHz pour les ventilateurs
  PC à 4 fils.

## License

MIT
