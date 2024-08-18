import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID

from . import CONF_GROWATT_ID, GrowattMTLComponent

DEPENDENCIES = ["growatt_mtl"]

CODEOWNERS = ["@mattaltus"]

CONF_FIRMWARE_VERSION = "firmware_version"
CONF_MANUFACTURER = "manufacturer"
CONF_SERIAL_NUMBER = "serial_number"


TEXT_SENSORS = [
    CONF_FIRMWARE_VERSION,
    CONF_MANUFACTURER,
    CONF_SERIAL_NUMBER,
]


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_GROWATT_ID): cv.use_id(GrowattMTLComponent),
        cv.Optional(CONF_FIRMWARE_VERSION): text_sensor.TEXT_SENSOR_SCHEMA.extend(
            {cv.GenerateID(): cv.declare_id(text_sensor.TextSensor)}
        ),
        cv.Optional(CONF_MANUFACTURER): text_sensor.TEXT_SENSOR_SCHEMA.extend(
            {cv.GenerateID(): cv.declare_id(text_sensor.TextSensor)}
        ),
        cv.Optional(CONF_SERIAL_NUMBER): text_sensor.TEXT_SENSOR_SCHEMA.extend(
            {cv.GenerateID(): cv.declare_id(text_sensor.TextSensor)}
        ),
    }
)


def to_code(config):
    hub = yield cg.get_variable(config[CONF_GROWATT_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            yield text_sensor.register_text_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
