from esphome import pins
from esphome.components import fan
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.cpp_helpers import gpio_pin_expression

from esphome.const import CONF_DISABLED_BY_DEFAULT, CONF_NAME, CONF_OUTPUT_ID, CONF_DATA_PIN, CONF_PRESET_MODES, CONF_RESTORE_MODE, CONF_SPEED_COUNT

cc1101fan_ns = cg.esphome_ns.namespace("cc1101fan")
CC1101Fan = cc1101fan_ns.class_("CC1101Fan", cg.PollingComponent, fan.Fan)

MAP_OFF_TO_ZERO = "map_off_to_zero"

# Define mapping manually
FAN_RESTORE_MODE_OPTIONS = {
    "ALWAYS_ON": fan.FanRestoreMode.ALWAYS_ON,
    "ALWAYS_OFF": fan.FanRestoreMode.ALWAYS_OFF,
}

CONFIG_SCHEMA = fan.fan_schema(CC1101Fan).extend({
  cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(CC1101Fan),
  cv.Required(CONF_DATA_PIN): cv.All(pins.internal_gpio_input_pin_schema),
  cv.Required(MAP_OFF_TO_ZERO): cv.boolean,
  cv.Optional(CONF_NAME, default="Domestic Fan"): str,
  cv.Optional(CONF_SPEED_COUNT, default=3): cv.int_range(min=1, max=100),
  cv.Optional(CONF_DISABLED_BY_DEFAULT, default=False): cv.boolean,
  cv.Optional(CONF_RESTORE_MODE, default="ALWAYS_ON"): cv.enum(FAN_RESTORE_MODE_OPTIONS),
  cv.Optional(CONF_PRESET_MODES): fan.validate_preset_modes,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID], config[CONF_SPEED_COUNT], config[MAP_OFF_TO_ZERO])
    await cg.register_component(var, config)
    await fan.register_fan(var, config)

    data_pin = await gpio_pin_expression(config[CONF_DATA_PIN])
    cg.add(var.set_data_pin(data_pin))

    if CONF_PRESET_MODES in config:
        cg.add(var.set_preset_modes(config[CONF_PRESET_MODES]))

    return var

