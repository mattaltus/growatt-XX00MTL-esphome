#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "growatt_proto.h"


namespace esphome {
namespace growatt_mtl {

class GrowattMTLComponent : public uart::UARTDevice, public PollingComponent {
  public:
    void set_fault_pin(InternalGPIOPin *pin) { fault_pin_ = pin; }
    void set_comms_pin(InternalGPIOPin *pin) { comms_pin_ = pin; }

    void set_has_fault_binary_sensor(binary_sensor::BinarySensor *has_fault_binary_sensor) { has_fault_binary_sensor_ = has_fault_binary_sensor; }

    void set_status_sensor        (sensor::Sensor *status_sensor)         { status_sensor_         = status_sensor; }
    void set_fault_code_sensor    (sensor::Sensor *fault_code_sensor)     { fault_code_sensor_     = fault_code_sensor; }
    void set_voltage_pv1_sensor   (sensor::Sensor *voltage_pv1_sensor)    { voltage_pv1_sensor_    = voltage_pv1_sensor; }
    void set_voltage_pv2_sensor   (sensor::Sensor *voltage_pv2_sensor)    { voltage_pv2_sensor_    = voltage_pv2_sensor; }
    void set_power_pv_sensor      (sensor::Sensor *power_pv_sensor)       { power_pv_sensor_       = power_pv_sensor; }
    void set_voltage_ac_sensor    (sensor::Sensor *voltage_ac_sensor)     { voltage_ac_sensor_     = voltage_ac_sensor; }
    void set_current_ac_sensor    (sensor::Sensor *current_ac_sensor)     { current_ac_sensor_     = current_ac_sensor; }
    void set_freq_ac_sensor       (sensor::Sensor *freq_ac_sensor)        { freq_ac_sensor_        = freq_ac_sensor; }
    void set_power_ac_sensor      (sensor::Sensor *power_ac_sensor)       { power_ac_sensor_       = power_ac_sensor; }
    void set_temperature_sensor   (sensor::Sensor *temperature_sensor)    { temperature_sensor_    = temperature_sensor; }
    void set_energy_today_sensor  (sensor::Sensor *energy_today_sensor)   { energy_today_sensor_   = energy_today_sensor; }
    void set_energy_total_sensor  (sensor::Sensor *energy_total_sensor)   { energy_total_sensor_   = energy_total_sensor; }
    void set_total_time_sensor    (sensor::Sensor *total_time_sensor)     { total_time_sensor_     = total_time_sensor; }
    void set_power_max_sensor     (sensor::Sensor *power_max_sensor)      { power_max_sensor_      = power_max_sensor; }

    void set_fault_text_text_sensor      (text_sensor::TextSensor *fault_text_text_sensor)       { fault_text_text_sensor_       = fault_text_text_sensor; }
    void set_firmware_version_text_sensor(text_sensor::TextSensor *firmware_version_text_sensor) { firmware_version_text_sensor_ = firmware_version_text_sensor; }
    void set_manufacturer_text_sensor    (text_sensor::TextSensor *manufacturer_text_sensor)     { manufacturer_text_sensor_     = manufacturer_text_sensor; }
    void set_serial_number_text_sensor   (text_sensor::TextSensor *serial_number_text_sensor)    { serial_number_text_sensor_    = serial_number_text_sensor; }

    void setup() override;
    void update() override;
    void dump_config() override;

    void send_cmd(int command);
    gw_data_t *recv_data(int command);

  protected:
    binary_sensor::BinarySensor *has_fault_binary_sensor_;

    sensor::Sensor *status_sensor_{nullptr};
    sensor::Sensor *fault_code_sensor_{nullptr};
    sensor::Sensor *voltage_pv1_sensor_{nullptr};
    sensor::Sensor *voltage_pv2_sensor_{nullptr};
    sensor::Sensor *power_pv_sensor_{nullptr};
    sensor::Sensor *voltage_ac_sensor_{nullptr};
    sensor::Sensor *current_ac_sensor_{nullptr};
    sensor::Sensor *freq_ac_sensor_{nullptr};
    sensor::Sensor *power_ac_sensor_{nullptr};
    sensor::Sensor *temperature_sensor_{nullptr};
    sensor::Sensor *energy_today_sensor_{nullptr};
    sensor::Sensor *energy_total_sensor_{nullptr};
    sensor::Sensor *total_time_sensor_{nullptr};
    sensor::Sensor *power_max_sensor_{nullptr};

    text_sensor::TextSensor *fault_text_text_sensor_{nullptr};
    text_sensor::TextSensor *firmware_version_text_sensor_{nullptr};
    text_sensor::TextSensor *manufacturer_text_sensor_{nullptr};
    text_sensor::TextSensor *serial_number_text_sensor_{nullptr};

    InternalGPIOPin *comms_pin_;
    InternalGPIOPin *fault_pin_;

}; // class

} // growatt-mtk
} // esphome