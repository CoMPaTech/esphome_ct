import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
#from esphome.const import CONF_ID, CONF_CS_PIN, CONF_GDO0_PIN, CONF_FREQUENCY, CONF_PACKET_LENGTH

cc1101_ns = cg.esphome_ns.namespace("itho_cc1101")
IthoCC1101 = cc1101_ns.class_("IthoCC1101", cg.Component)

CONF_ID = "id"
CONF_CS_PIN = "cs_pin"
CONF_GDO0_PIN = "gdo0_pin"
CONF_FREQUENCY = "frequency"
CONF_PACKET_LENGTH = "packet_length"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(IthoCC1101),
    cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_GDO0_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_FREQUENCY): cv.int_range(min=300_000_000, max=1_000_000_000),
    cv.Optional(CONF_PACKET_LENGTH, default=7): cv.int_range(min=1, max=64)
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Convert pin configs into expressions
    cs = await cg.gpio_pin_expression(config[CONF_CS_PIN])
    gdo0 = await cg.gpio_pin_expression(config[CONF_GDO0_PIN])

    cg.add(var.set_cs_pin(cs))
    cg.add(var.set_gdo0_pin(gdo0))

    cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    cg.add(var.set_packet_length(config[CONF_PACKET_LENGTH]))


