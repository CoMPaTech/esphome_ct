import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import fan
from esphome.const import CONF_ID

CONF_ADDRESS = "address"
CONF_INITIAL_CODE = "initial_code"
CONF_CS_PIN = "cs_pin"
CONF_GDO0_PIN = "gdo0_pin"
CONF_FREQUENCY = "frequency"
CONF_PACKET_LENGTH = "packet_length"

ithofan_ns = cg.esphome_ns.namespace("ithofan")
IthoFanComponent = ithofan_ns.class_("IthoFanComponent", fan.Fan, cg.Component)

# Reference the CC1101 C++ class directly; we won't expose cc1101 in YAML
cc1101_ns = cg.esphome_ns.namespace("cc1101")
CC1101Component = cc1101_ns.class_("CC1101Component", cg.Component)

CONFIG_SCHEMA = fan._FAN_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(IthoFanComponent),
        cv.Required(CONF_ADDRESS): cv.hex_uint32_t,
        cv.Optional(CONF_INITIAL_CODE, default=1): cv.uint16_t,

        # Radio config (embedded here instead of cc1101:)
        cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_GDO0_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_FREQUENCY): cv.int_range(min=300_000_000, max=1_000_000_000),
        cv.Optional(CONF_PACKET_LENGTH, default=7): cv.int_range(min=1, max=64),
    }
)

async def to_code(config):
    # Create the fan component
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await fan.register_fan(var, config)

    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_code(config[CONF_INITIAL_CODE]))

    # Create and configure the CC1101 radio (embedded, no YAML cc1101:)
    radio = cg.new_Pvariable(cg.allocate_id("itho_cc1101"), CC1101Component)
    await cg.register_component(radio, {})  # No YAML schema; we configure via setters

    # Configure pins and radio params (call the CC1101 setters available in your tree)
    cs = await pins.gpio_output_pin_expression(config[CONF_CS_PIN])
    gdo0 = await pins.gpio_input_pin_expression(config[CONF_GDO0_PIN])

    # These setters names may differ slightly depending on your cc1101.cpp:
    cg.add(radio.set_cs_pin(cs))
    cg.add(radio.set_gdo0_pin(gdo0))
    cg.add(radio.set_frequency(config[CONF_FREQUENCY]))
    cg.add(radio.set_packet_mode(True))
    cg.add(radio.set_packet_length(config[CONF_PACKET_LENGTH]))

    # Wire radio into fan
    cg.add(var.set_radio(radio))

