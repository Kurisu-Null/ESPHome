#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/sn74hc595/sn74hc595.h"
#include "esphome/core/log.h"

#ifdef USE_ESP8266
#include "esphome/core/esphal.h"
#include <Ticker.h>
namespace ticker = esphome;
#endif

namespace esphome {
namespace sn74hc595_pwm {

class SN74HC595PWM;

class SN74HC595PWMOutput : public Component, public output::FloatOutput {
 public:
  SN74HC595PWMOutput() {}

  void set_parent(sn74hc595::SN74HC595Component *parent) { this->parent_ = parent; }
  void set_pin(uint8_t pin) { this->pin_ = pin; }
  void set_frequency(uint32_t frequency_hz) { this->frequency_ = frequency_hz; }

  void setup() override {
    // Register ourselves with the PWM timer
    register_for_pwm_updates();
  }

  void write_state(float state) override {
    // Store PWM value (0.0-1.0)
    this->state_ = state;
  }

  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  sn74hc595::SN74HC595Component *parent_;
  uint8_t pin_;
  float state_ = 0.0f;
  uint32_t frequency_ = 1000;  // Default PWM frequency: 1kHz
  static std::vector<SN74HC595PWMOutput *> instances_;
#ifdef USE_ESP32
  static hw_timer_t *pwm_timer_;
#endif

  void register_for_pwm_updates() {
    // If this is the first instance, set up the timer
    if (instances_.empty()) {
      setup_pwm_timer_static();
    }
    
    // Add this instance to the list of outputs to update
    instances_.push_back(this);
  }

  static void setup_pwm_timer_static() {
    // The first instance should already be added to instances_ vector
    if (instances_.empty()) {
      return;  // No instances yet, nothing to do
    }
    
    // Get frequency from first instance (all should use the same frequency)
    uint32_t frequency = instances_[0]->frequency_;
    
#ifdef USE_ESP32
    if (pwm_timer_ != nullptr) {
      timerAlarmDisable(pwm_timer_);
      timerDetachInterrupt(pwm_timer_);
      timerEnd(pwm_timer_);
    }
    
    // Configure timer for PWM generation
    // Using timer 0 with divider of 80 (1MHz) for ESP32
    pwm_timer_ = timerBegin(0, 80, true);
    timerAttachInterrupt(pwm_timer_, &pwm_timer_isr, true);
    
    // Calculate interval in microseconds
    // We want 256 steps per PWM cycle for 8-bit resolution
    uint32_t interval_us = 1000000 / (frequency * 256);
    timerAlarmWrite(pwm_timer_, interval_us, true);
    timerAlarmEnable(pwm_timer_);
#elif defined(USE_ESP8266)
    // ESP8266 timer setup
    // Use ticker library for ESP8266
    static ticker::Ticker pwm_ticker;
    uint32_t interval_ms = 1000 / (frequency * 256);
    pwm_ticker.attach_ms(interval_ms, &pwm_timer_isr);
#endif
  }

#if defined(USE_ESP32) || defined(USE_ESP8266)
  static void IRAM_ATTR pwm_timer_isr() {
#else
  static void pwm_timer_isr() {
#endif
    static uint8_t pwm_counter = 0;
    
    // Update all shift register pins based on their PWM values
    // We cannot modify the contents of the vector in the ISR
    // and we need to check the size first for safety
    if (!instances_.empty()) {
      size_t count = instances_.size();
      for (size_t i = 0; i < count; i++) {
        if (instances_[i] != nullptr) {
          uint8_t pwm_value = instances_[i]->state_ * 255;
          bool pin_state = pwm_value > pwm_counter;
          
          // Write to the specific pin on the shift register
          instances_[i]->parent_->set_pin(instances_[i]->pin_, pin_state);
        }
      }
    }
    
    // Increment PWM counter (0-255, 8-bit resolution)
    pwm_counter = (pwm_counter + 1) % 256;
  }
};

// Initialize static members
std::vector<SN74HC595PWMOutput *> SN74HC595PWMOutput::instances_;
#ifdef USE_ESP32
hw_timer_t *SN74HC595PWMOutput::pwm_timer_ = nullptr;
#endif

// SN74HC595PWM component class
class SN74HC595PWM : public Component {
 public:
  SN74HC595PWM() {}

  SN74HC595PWMOutput *create_output() {
    auto *output = new SN74HC595PWMOutput();
    output->set_parent(this->parent_);
    output->set_pin(this->pin_);
    output->set_frequency(this->frequency_);
    return output;
  }

  void set_parent(sn74hc595::SN74HC595Component *parent) { parent_ = parent; }
  void set_pin(uint8_t pin) { pin_ = pin; }
  void set_frequency(uint32_t frequency) { frequency_ = frequency; }

  void setup() override {}
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  sn74hc595::SN74HC595Component *parent_;
  uint8_t pin_;
  uint32_t frequency_ = 1000;
};

}  // namespace sn74hc595_pwm
}  // namespace esphome