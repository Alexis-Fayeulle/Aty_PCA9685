/**
 * SPDX-FileCopyrightText: 2025 Alexis Fayeulle
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * Driver for the NXP PCA9685 16-channel, 12-bit PWM I2C controller.
 *
 * Reference: NXP PCA9685 datasheet, Rev. 4 (16 April 2015).
 *
 * Notes:
 *  - begin() calls Wire.begin() for you (pass false to manage the bus yourself).
 *  - The PWM frequency is shared by all 16 channels: 24 Hz to 1526 Hz with the internal
 *    25 MHz oscillator. It cannot reach 25 kHz (PC fan standard).
 *  - Duty cycles are 12-bit values: 0 (always OFF) to 4095 (always ON).
 */
class Aty_PCA9685 {
public:
  static constexpr uint8_t  DEFAULT_ADDRESS = 0x40;  ///< Default I2C address (A5..A0 = 0)
  static constexpr uint8_t  CHANNEL_COUNT   = 16;
  static constexpr uint16_t MAX_DUTY        = 4095;  ///< 12-bit maximum (always ON)
  static constexpr float    MIN_FREQUENCY   = 24.0f; ///< Hz, with 25 MHz oscillator
  static constexpr float    MAX_FREQUENCY   = 1526.0f;

  /**
   * @param address I2C address of the chip (0x40 to 0x7F depending on A0..A5 pins)
   * @param wire    I2C bus to use
   */
  explicit Aty_PCA9685(uint8_t address = DEFAULT_ADDRESS, TwoWire& wire = Wire);

  /**
   * @brief Starts the I2C bus, checks that the chip answers, wakes it up and enables
   *        register auto-increment.
   * @param initWire false if you already called Wire.begin() with your own settings
   * @return true if the chip answered on the bus
   */
  bool begin(bool initWire = true);

  /// @brief true if the chip acknowledges its address
  bool isConnected();

  /**
   * @brief Sets the PWM frequency of all channels.
   * @param hz Wanted frequency, clamped to MIN_FREQUENCY..MAX_FREQUENCY
   * @return the frequency really applied (the prescaler is an integer), or 0 on I2C error
   */
  float setFrequency(float hz);

  /// @brief Frequency currently programmed in the chip, in Hz (0 on I2C error)
  float getFrequency();

  /// @brief Sets the prescaler directly (3..255). Frequency = osc / (4096 * (prescale + 1)).
  bool setPrescale(uint8_t prescale);

  /// @brief Real frequency of the oscillator, if it differs from 25 MHz (default)
  void setOscillatorFrequency(uint32_t hz) { _oscillatorHz = hz; }

  /**
   * @brief Sets the duty cycle of a channel.
   * @param channel 0..15
   * @param duty    0 (always OFF) to 4095 (always ON), larger values are clamped
   */
  bool setDuty(uint8_t channel, uint16_t duty);

  /// @brief Sets the duty cycle in percent (0.0 to 100.0)
  bool setDutyPercent(uint8_t channel, float percent);

  /**
   * @brief Sets the pulse width in microseconds (typical use: hobby servos, 1000..2000 us,
   *        after setFrequency(50)). The value is limited to one PWM period.
   */
  bool setPulseMicroseconds(uint8_t channel, uint16_t microseconds);

  /// @brief Same as setDuty() but for all 16 channels at once
  bool setAllDuty(uint16_t duty);

  /// @brief Output permanently ON (bypasses the PWM counter)
  bool fullOn(uint8_t channel);

  /// @brief Output permanently OFF (bypasses the PWM counter)
  bool fullOff(uint8_t channel);

  /**
   * @brief Raw access: output goes high at tick `on` and low at tick `off` (0..4095 each).
   *        `on` and `off` must not be equal (datasheet 7.3.3); returns false in that case.
   */
  bool setPWM(uint8_t channel, uint16_t on, uint16_t off);

  /// @brief Puts the oscillator to sleep. Outputs are frozen/off until wakeUp().
  bool sleep();

  /// @brief Wakes the chip up and restarts the channels that were running before sleep().
  bool wakeUp();

  /// @brief Inverts the output logic (MODE2 INVRT bit)
  bool setInvertedOutputs(bool inverted);

  /// @brief true: totem-pole outputs (default), false: open-drain outputs (MODE2 OUTDRV bit)
  bool setTotemPoleOutputs(bool totemPole);

  /// @brief Raw register access
  bool    writeRegister(uint8_t reg, uint8_t value);
  uint8_t readRegister(uint8_t reg, bool* ok = nullptr);

private:
  // Register map (datasheet table 4)
  static constexpr uint8_t REG_MODE1       = 0x00;
  static constexpr uint8_t REG_MODE2       = 0x01;
  static constexpr uint8_t REG_LED0_ON_L   = 0x06;
  static constexpr uint8_t REG_ALL_LED_ON_L = 0xFA;
  static constexpr uint8_t REG_PRE_SCALE   = 0xFE;

  // MODE1 bits (table 5)
  static constexpr uint8_t MODE1_RESTART = 0x80;
  static constexpr uint8_t MODE1_AI      = 0x20;
  static constexpr uint8_t MODE1_SLEEP   = 0x10;

  // MODE2 bits (table 6)
  static constexpr uint8_t MODE2_INVRT  = 0x10;
  static constexpr uint8_t MODE2_OUTDRV = 0x04;

  // LEDn_ON_H / LEDn_OFF_H bit 4 (table 7)
  static constexpr uint8_t FULL_BIT = 0x10;

  static constexpr uint8_t MIN_PRESCALE = 3;

  bool writeChannel(uint8_t regBase, uint16_t on, uint16_t off);
  bool updateRegister(uint8_t reg, uint8_t mask, bool set);

  TwoWire& _wire;
  uint8_t  _address;
  uint32_t _oscillatorHz = 25000000UL;
  float    _frequency    = 200.0f;  // chip default, refreshed by begin()/setFrequency()
};
