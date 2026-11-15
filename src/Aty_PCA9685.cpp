/**
 * SPDX-FileCopyrightText: 2025 Alexis Fayeulle
 *
 * SPDX-License-Identifier: MIT
 */

#include "Aty_PCA9685.h"

// Datasheet 7.3.1 note [2]: the oscillator needs 500 us to start once SLEEP is cleared.
static const unsigned int OSCILLATOR_STARTUP_US = 600;

Aty_PCA9685::Aty_PCA9685(uint8_t address, TwoWire& wire) : _wire(wire), _address(address) {}

bool Aty_PCA9685::isConnected() {
  _wire.beginTransmission(_address);
  return _wire.endTransmission() == 0;
}

bool Aty_PCA9685::begin(bool initWire) {
  if (initWire) _wire.begin();
  if (!isConnected()) return false;

  bool ok;
  uint8_t mode1 = readRegister(REG_MODE1, &ok);
  if (!ok) return false;

  // Wake up (SLEEP=0), enable auto-increment, do not write RESTART yet
  uint8_t awake = (mode1 & ~(MODE1_RESTART | MODE1_SLEEP)) | MODE1_AI;
  if (!writeRegister(REG_MODE1, awake)) return false;
  delayMicroseconds(OSCILLATOR_STARTUP_US);

  // Datasheet 7.3.1.1: channels that were running are restarted by writing 1 to RESTART
  _frequency = getFrequency();
  if (mode1 & MODE1_RESTART) return writeRegister(REG_MODE1, awake | MODE1_RESTART);
  return true;
}

bool Aty_PCA9685::setPrescale(uint8_t prescale) {
  if (prescale < MIN_PRESCALE) prescale = MIN_PRESCALE;  // hardware minimum (datasheet 7.3.5)

  bool ok;
  uint8_t mode1 = readRegister(REG_MODE1, &ok);
  if (!ok) return false;

  bool wasAsleep = mode1 & MODE1_SLEEP;

  // PRE_SCALE can only be written while SLEEP = 1 (datasheet 7.3.5)
  uint8_t asleep = (mode1 & ~MODE1_RESTART) | MODE1_SLEEP;
  if (!writeRegister(REG_MODE1, asleep)) return false;
  if (!writeRegister(REG_PRE_SCALE, prescale)) return false;

  if (wasAsleep) return true;  // the chip was sleeping: leave it as it was

  uint8_t awake = asleep & ~MODE1_SLEEP;
  if (!writeRegister(REG_MODE1, awake)) return false;
  delayMicroseconds(OSCILLATOR_STARTUP_US);

  // Restart the channels that were running (RESTART was set by the sleep)
  return writeRegister(REG_MODE1, awake | MODE1_RESTART);
}

float Aty_PCA9685::setFrequency(float hz) {
  if (hz < MIN_FREQUENCY) hz = MIN_FREQUENCY;
  if (hz > MAX_FREQUENCY) hz = MAX_FREQUENCY;

  // Datasheet equation 1: prescale = round(osc / (4096 * rate)) - 1
  float prescale = (float)_oscillatorHz / (4096.0f * hz) - 1.0f + 0.5f;
  if (prescale < MIN_PRESCALE) prescale = MIN_PRESCALE;
  if (prescale > 255.0f) prescale = 255.0f;

  uint8_t value = (uint8_t)prescale;
  if (!setPrescale(value)) return 0.0f;
  _frequency = (float)_oscillatorHz / (4096.0f * ((float)value + 1.0f));
  return _frequency;
}

float Aty_PCA9685::getFrequency() {
  bool ok;
  uint8_t prescale = readRegister(REG_PRE_SCALE, &ok);
  if (!ok) return 0.0f;
  return (float)_oscillatorHz / (4096.0f * ((float)prescale + 1.0f));
}

bool Aty_PCA9685::writeChannel(uint8_t regBase, uint16_t on, uint16_t off) {
  // Auto-increment (enabled in begin()) lets us load the 4 registers in one transaction.
  // The output only changes once the 4th byte is acknowledged (datasheet 7.3.3).
  _wire.beginTransmission(_address);
  _wire.write(regBase);
  _wire.write((uint8_t)(on & 0xFF));
  _wire.write((uint8_t)(on >> 8));
  _wire.write((uint8_t)(off & 0xFF));
  _wire.write((uint8_t)(off >> 8));
  return _wire.endTransmission() == 0;
}

bool Aty_PCA9685::setPWM(uint8_t channel, uint16_t on, uint16_t off) {
  if (channel >= CHANNEL_COUNT) return false;
  on &= 0x0FFF;
  off &= 0x0FFF;
  if (on == off) return false;  // "should never be programmed with the same values"
  return writeChannel(REG_LED0_ON_L + 4 * channel, on, off);
}

bool Aty_PCA9685::fullOn(uint8_t channel) {
  if (channel >= CHANNEL_COUNT) return false;
  return writeChannel(REG_LED0_ON_L + 4 * channel, (uint16_t)FULL_BIT << 8, 0);
}

bool Aty_PCA9685::fullOff(uint8_t channel) {
  if (channel >= CHANNEL_COUNT) return false;
  // Full OFF has priority over full ON and over the counters (datasheet 7.3.3)
  return writeChannel(REG_LED0_ON_L + 4 * channel, 0, (uint16_t)FULL_BIT << 8);
}

bool Aty_PCA9685::setDuty(uint8_t channel, uint16_t duty) {
  if (duty == 0) return fullOff(channel);
  if (duty >= MAX_DUTY) return fullOn(channel);
  return setPWM(channel, 0, duty);
}

bool Aty_PCA9685::setDutyPercent(uint8_t channel, float percent) {
  if (percent < 0.0f) percent = 0.0f;
  if (percent > 100.0f) percent = 100.0f;
  return setDuty(channel, (uint16_t)(percent * (float)MAX_DUTY / 100.0f + 0.5f));
}

bool Aty_PCA9685::setPulseMicroseconds(uint8_t channel, uint16_t microseconds) {
  // One PWM period lasts 1e6 / frequency microseconds and is split in 4096 ticks
  float ticks = (float)microseconds * _frequency * 4096.0f / 1000000.0f + 0.5f;
  if (ticks > (float)MAX_DUTY) ticks = (float)MAX_DUTY;
  return setDuty(channel, (uint16_t)ticks);
}

bool Aty_PCA9685::setAllDuty(uint16_t duty) {
  if (duty == 0) return writeChannel(REG_ALL_LED_ON_L, 0, (uint16_t)FULL_BIT << 8);
  if (duty >= MAX_DUTY) return writeChannel(REG_ALL_LED_ON_L, (uint16_t)FULL_BIT << 8, 0);
  return writeChannel(REG_ALL_LED_ON_L, 0, duty);
}

bool Aty_PCA9685::sleep() {
  return updateRegister(REG_MODE1, MODE1_SLEEP, true);
}

bool Aty_PCA9685::wakeUp() {
  bool ok;
  uint8_t mode1 = readRegister(REG_MODE1, &ok);
  if (!ok) return false;

  uint8_t awake = mode1 & ~(MODE1_RESTART | MODE1_SLEEP);
  if (!writeRegister(REG_MODE1, awake)) return false;
  delayMicroseconds(OSCILLATOR_STARTUP_US);

  if (mode1 & MODE1_RESTART) return writeRegister(REG_MODE1, awake | MODE1_RESTART);
  return true;
}

bool Aty_PCA9685::setInvertedOutputs(bool inverted) {
  return updateRegister(REG_MODE2, MODE2_INVRT, inverted);
}

bool Aty_PCA9685::setTotemPoleOutputs(bool totemPole) {
  return updateRegister(REG_MODE2, MODE2_OUTDRV, totemPole);
}

bool Aty_PCA9685::updateRegister(uint8_t reg, uint8_t mask, bool set) {
  bool ok;
  uint8_t value = readRegister(reg, &ok);
  if (!ok) return false;

  if (reg == REG_MODE1) value &= ~MODE1_RESTART;  // writing a 1 there would trigger a restart
  value = set ? (value | mask) : (value & ~mask);
  return writeRegister(reg, value);
}

bool Aty_PCA9685::writeRegister(uint8_t reg, uint8_t value) {
  _wire.beginTransmission(_address);
  _wire.write(reg);
  _wire.write(value);
  return _wire.endTransmission() == 0;
}

uint8_t Aty_PCA9685::readRegister(uint8_t reg, bool* ok) {
  _wire.beginTransmission(_address);
  _wire.write(reg);
  bool success = _wire.endTransmission(false) == 0;

  uint8_t value = 0;
  if (success && _wire.requestFrom(_address, (uint8_t)1) == 1) {
    value = _wire.read();
  } else {
    success = false;
  }

  if (ok != nullptr) *ok = success;
  return value;
}
