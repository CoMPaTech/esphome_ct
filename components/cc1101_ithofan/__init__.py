import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.cc1101 as cc1101
from esphome import pins

DEPENDENCIES = ["cc1101"]

cc1101_ns = cg.esphome_ns.namespace("itho_cc1101") 
IthoCC1101 = cc1101_ns.class_("IthoCC1101", cc1101.CC1101Component)

CONF_ID = "id"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(IthoCC1101),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

