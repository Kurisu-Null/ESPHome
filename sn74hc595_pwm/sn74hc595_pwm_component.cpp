#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/sn74hc595/sn74hc595.h"
#include "esphome/core/log.h"

namespace esphome {
namespace sn74hc595_pwm {

class SN74HC595PWMOutput;

// This class is used for the YAML schema
class SN74HC595PWM : public Component {
 public:
  SN74HC595PWM() {}

  SN74HC595PWMOutput *create_output();
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

// Include implementation from the .h file
#include "sn74hc595_pwm.h"

SN74HC595PWMOutput *SN74HC595PWM::create_output() {
  auto *output = new SN74HC595PWMOutput();
  output->set_parent(this->parent_);
  output->set_pin(this->pin_);
  output->set_frequency(this->frequency_);
  return output;
}

}  // namespace sn74hc595_pwm
}  // namespace esphome