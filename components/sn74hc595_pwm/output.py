import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID, CONF_PIN, CONF_FREQUENCY
from esphome.components import sn74hc595
from . import sn74hc595_pwm_ns, SN74HC595PWMOutput

DEPENDENCIES = ['sn74hc595']

# Define configuration keys
CONF_SHIFT_REGISTER_ID = 'shift_register_id'

# Configuration schema for output platform
PLATFORM_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend({
    cv.GenerateID(output.CONF_ID): cv.declare_id(SN74HC595PWMOutput),
    cv.Required(CONF_SHIFT_REGISTER_ID): cv.use_id(sn74hc595.SN74HC595Component),
    cv.Required(CONF_PIN): cv.int_range(min=0, max=255),
    cv.Optional(CONF_FREQUENCY, default=1000): cv.int_range(min=1, max=20000),
})

async def to_code(config):
    var = cg.new_Pvariable(config[output.CONF_ID])
    await cg.register_component(var, config)
    
    shift_register = await cg.get_variable(config[CONF_SHIFT_REGISTER_ID])
    cg.add(var.set_parent(shift_register))
    cg.add(var.set_pin(config[CONF_PIN]))
    cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    
    await output.register_output(var, config)