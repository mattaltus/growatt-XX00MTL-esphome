import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_FREQUENCY,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_EMPTY,
    ICON_FLASH,
    ICON_GAUGE,
    ICON_POWER,
    ICON_EMPTY,
    ICON_CURRENT_AC,
    ICON_TIMER,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_CELSIUS,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_WATT,
    UNIT_WATT_HOURS,
    UNIT_HERTZ,
    UNIT_EMPTY,
    UNIT_HOUR,
)

from . import CONF_GROWATT_ID, GrowattMTLComponent

DEPENDENCIES = ["growatt_mtl"]

CODEOWNERS = ["@mattaltus"]

CONF_STATUS = "status"
CONF_FAULT_CODE = "fault_code"
CONF_VOLTAGE_PV1 = "voltage_pv1"
CONF_VOLTAGE_PV2 = "voltage_pv2"
CONF_POWER_PV = "power_pv"
CONF_VOLTAGE_AC = "voltage_ac"
CONF_CURRENT_AC = "current_ac"
CONF_FREQ_AC = "freq_ac"
CONF_POWER_AC = "power_ac"
CONF_TEMPERATURE = "temperature"
CONF_ENERGY_TODAY = "energy_today"
CONF_ENERGY_TOTAL = "energy_total"
CONF_TOTAL_TIME = "total_time"
CONF_POWER_MAX = "power_max"

SENSORS = [
    CONF_STATUS,
    CONF_FAULT_CODE,
    CONF_VOLTAGE_PV1,
    CONF_VOLTAGE_PV2,
    CONF_POWER_PV,
    CONF_VOLTAGE_AC,
    CONF_CURRENT_AC,
    CONF_FREQ_AC,
    CONF_POWER_AC,
    CONF_TEMPERATURE,
    CONF_ENERGY_TODAY,
    CONF_ENERGY_TOTAL,
    CONF_TOTAL_TIME,
    CONF_POWER_MAX,
]


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_GROWATT_ID): cv.use_id(GrowattMTLComponent),
        cv.Optional(CONF_STATUS): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
        ),
        cv.Optional(CONF_FAULT_CODE): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
        ),
        cv.Optional(CONF_VOLTAGE_PV1): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            icon=ICON_FLASH,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
        ),
        cv.Optional(CONF_VOLTAGE_PV2): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            icon=ICON_FLASH,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
        ),
        cv.Optional(CONF_VOLTAGE_AC): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            icon=ICON_FLASH,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
        ),
        cv.Optional(CONF_CURRENT_AC): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            icon=ICON_CURRENT_AC,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_CURRENT,
        ),
        cv.Optional(CONF_POWER_PV): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            icon=ICON_POWER,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_POWER,
        ),
        cv.Optional(CONF_POWER_AC): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            icon=ICON_POWER,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_POWER,
        ),
        cv.Optional(CONF_FREQ_AC): sensor.sensor_schema(
            unit_of_measurement=UNIT_HERTZ,
            icon=ICON_POWER,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_FREQUENCY,
        ),
        cv.Optional(CONF_ENERGY_TODAY): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT_HOURS,
            icon=ICON_POWER,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_ENERGY_TOTAL): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT_HOURS,
            icon=ICON_POWER,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            icon=ICON_GAUGE,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POWER_MAX): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            icon=ICON_POWER,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_POWER,
        ),
        cv.Optional(CONF_TOTAL_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_HOUR,
            icon=ICON_TIMER,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_DURATION,
        ),
    }
)


def to_code(config):
    hub = yield cg.get_variable(config[CONF_GROWATT_ID])
    for key in SENSORS:
        if key in config:
            conf = config[key]
            sens = yield sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
