import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

ithofan_ns = cg.esphome_ns.namespace("ithofan")
IthoFanComponent = ithofan_ns.class_("IthoFanComponent", cg.Component)

CONF_ADDRESS = "address"
CONF_INITIAL_CODE = "initial_code"
CONF_REPEAT = "repeat"
CONF_TX_ID = "tx_id"
CONF_RX_ID = "rx_id"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.declare_id(IthoFanComponent),
#    cv.Required(CONF_ADDRESS): cv.hex_uint32_t,
    cv.Optional(CONF_INITIAL_CODE, default=1): cv.uint16_t,
    cv.Optional(CONF_REPEAT, default=4): cv.uint32_t,
    cv.Required(CONF_TX_ID): cv.use_id(cg.RemoteTransmitterComponent),
    cv.Required(CONF_RX_ID): cv.use_id(cg.RemoteReceiverComponent),
})


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_code(config[CONF_INITIAL_CODE]))

    tx = await cg.get_variable(config[CONF_TX_ID])
    rx = await cg.get_variable(config[CONF_RX_ID])
    cg.add(var.set_tx(tx))
    cg.add(var.set_rx(rx))

