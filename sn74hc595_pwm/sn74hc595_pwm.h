#include "esphome.h"
#include "esphome/components/sn74hc595/sn74hc595.h"

namespace esphome {
namespace sn74hc595_pwm {

class SN74HC595PWMOutput : public Component, public FloatOutput {
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
  static hw_timer_t *pwm_timer_;

  void register_for_pwm_updates() {
    // If this is the first instance, set up the timer
    if (instances_.empty()) {
      setup_pwm_timer_static();
    }
    
    // Add this instance to the list of outputs to update
    instances_.push_back(this);
  }

  static void setup_pwm_timer_static() {
    // Get frequency from first instance (all should use the same frequency)
    uint32_t frequency = instances_[0]->frequency_;
    
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
  }

  static void IRAM_ATTR pwm_timer_isr() {
    static uint8_t pwm_counter = 0;
    
    // Update all shift register pins based on their PWM values
    for (auto instance : instances_) {
      uint8_t pwm_value = instance->state_ * 255;
      bool pin_state = pwm_value > pwm_counter;
      
      // Write to the specific pin on the shift register
      instance->parent_->set_pin(instance->pin_, pin_state);
    }
    
    // Increment PWM counter (0-255, 8-bit resolution)
    pwm_counter = (pwm_counter + 1) % 256;
  }
};

// Initialize static members
std::vector<SN74HC595PWMOutput *> SN74HC595PWMOutput::instances_;
hw_timer_t *SN74HC595PWMOutput::pwm_timer_ = nullptr;

}  // namespace sn74hc595_pwm
}  // namespace esphome