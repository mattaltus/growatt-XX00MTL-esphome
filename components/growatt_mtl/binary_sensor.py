import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ICON, CONF_ID, ICON_EMPTY

from . import CONF_GROWATT_ID, GrowattMTLComponent

DEPENDENCIES = ["growatt_mtl"]

CODEOWNERS = ["@mattaltus"]

CONF_HAS_FAULT = "has_fault"

BINARY_SENSORS = [
    CONF_HAS_FAULT,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_GROWATT_ID): cv.use_id(GrowattMTLComponent),
        cv.Optional(CONF_HAS_FAULT): binary_sensor.BINARY_SENSOR_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(binary_sensor.BinarySensor),
                cv.Optional(CONF_ICON, default=ICON_EMPTY): cv.icon,
            }
        ),
    }
)


def to_code(config):
    hub = yield cg.get_variable(config[CONF_GROWATT_ID])
    for key in BINARY_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            yield binary_sensor.register_binary_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
