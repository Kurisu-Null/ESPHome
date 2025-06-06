"""SN74HC595 PWM component for ESPHome."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID, CONF_PIN, CONF_FREQUENCY
from esphome.components import sn74hc595

DEPENDENCIES = ['sn74hc595']
AUTO_LOAD = ['output']

# Define namespace
sn74hc595_pwm_ns = cg.esphome_ns.namespace('sn74hc595_pwm')
SN74HC595PWM = sn74hc595_pwm_ns.class_('SN74HC595PWM', cg.Component)
SN74HC595PWMOutput = sn74hc595_pwm_ns.class_('SN74HC595PWMOutput', output.FloatOutput, cg.Component)

# Define configuration keys
CONF_SHIFT_REGISTER_ID = 'shift_register_id'

# Configuration schema for component
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SN74HC595PWM),
    cv.Required(CONF_SHIFT_REGISTER_ID): cv.use_id(sn74hc595.SN74HC595Component),
    cv.Required(CONF_PIN): cv.int_range(min=0, max=255),  # Up to 32 shift registers (8 pins each)
    cv.Optional(CONF_FREQUENCY, default=1000): cv.int_range(min=1, max=20000),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    shift_register = await cg.get_variable(config[CONF_SHIFT_REGISTER_ID])
    cg.add(var.set_parent(shift_register))
    cg.add(var.set_pin(config[CONF_PIN]))
    cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    
    # Create and register output
    output_var = var.create_output()
    # Register with output platform
    await output.register_output(output_var, {})

# This component is registered as an output platform in output.py
MULTI_CONF = True