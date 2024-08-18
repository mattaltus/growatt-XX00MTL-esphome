from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]

DEPENDENCIES = ["uart"]

CODEOWNERS = ["@mattaltus"]

MULTI_CONF = True

growatt_mtl_ns = cg.esphome_ns.namespace("growatt_mtl")
GrowattMTLComponent = growatt_mtl_ns.class_("GrowattMTLComponent", uart.UARTDevice, cg.Component)

CONF_GROWATT_ID = "growatt_mtl_id"
CONF_COMMS_PIN = "comms_pin"
CONF_FAULT_PIN = "fault_pin"

CONFIG_SCHEMA = uart.UART_DEVICE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(GrowattMTLComponent),
        cv.Required(CONF_COMMS_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_FAULT_PIN): pins.gpio_output_pin_schema,
    }
)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield uart.register_uart_device(var, config)

async def setup(var, config):
    pin = await cg.gpio_pin_expression(config[CONF_COMMS_PIN])
    cg.add(var.set_comms_pin(pin))

    pin = await cg.gpio_pin_expression(config[CONF_FAULT_PIN])
    cg.add(var.set_fault_pin(pin))
