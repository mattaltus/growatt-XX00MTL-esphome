#include "growatt-mtl.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "growatt_proto.h"
#include "esphome/core/gpio.h"

namespace esphome {
namespace growatt_mtl {

  static const char *const TAG = "growatt-mtl";

  struct ctx {
    uint32_t ctx;
    size_t   offset;
  };

  static void ctx_ini_(struct ctx *ctx) {
    ctx->ctx = 0;
    ctx->offset = 0;
  }
  static void ctx_add_(struct ctx *ctx, void *d, size_t len) {
    char *ptr = (char *)d;
    for (size_t offset = 0; offset < len; offset++) {
      ctx->ctx += (uint8_t)(((uint8_t)ptr[offset]) ^ (offset + ctx->offset));
    }
    ctx->offset += len;
  }
  static uint16_t ctx_fin_(struct ctx *ctx) {
    if (ctx->ctx == 0)
      return 0xFFFF;
    return (uint16_t)ctx->ctx;
  }

  static std::string fault_text(int fault) {
    switch (fault) {
      case 1 ... 23:
        return "Coded Error";
      case GW_ERROR_AUTO_TEST_FAIL:
        return "Auto Test Failed";
      case GW_ERROR_NO_AC_CONNECTION:
        return "No AC Connection";
      case GW_ERROR_PV_ISOLATION_LOW:
        return "PV Isolation Low";
      case GW_ERROR_RESIDUAL_I_HIGH:
        return "Residual I High";
      case GW_ERROR_OUTPUT_HIGH_DCI:
        return "High Output DCI";
      case GW_ERROR_PV_VOLTAGE_HIGH:
        return "PV Voltage High";
      case GW_ERROR_AC_V_OUTOFRANGE:
        return "AC Voltage Outrange";
      case GW_ERROR_AC_F_OUTOFRANGE:
        return "AC Frequency Outrange";
      case GW_ERROR_MODULE_HOT:
        return "Module Hot";
      default:
        return "Unknown Error";
    }
  }

  void GrowattMTLComponent::send_cmd(int command) {
    struct ctx ctx;
    gw_cmd_t buf = {0};

    buf.header1 = GW_CMD_HEADER1;
    buf.header2 = GW_CMD_HEADER2;
    buf.addr    = GW_ADDRESS_BROADCAST;
    buf.c0      = GW_CMD_CODE;
    buf.c1      = command;
    buf.size    = 0;

    ctx_ini_(&ctx);
    ctx_add_(&ctx, &buf, sizeof(buf) - sizeof(buf.checksum));
    buf.checksum = ctx_fin_(&ctx);

    flush();
    write_array((uint8_t *)&buf, sizeof(buf));
  }

  gw_data_t *GrowattMTLComponent::recv_data(int command) {
    gw_data_header_t header = {0};
    gw_data_footer_t footer = {0};
    static gw_data_t data   = {0};
    int s;
    size_t exs = 0;
    struct ctx ctx = {0};

    ctx_ini_(&ctx);

    read_array((uint8_t *)&header, sizeof(header));
    if (header.header1 != GW_DATA_HEADER1
     || header.header2 != GW_DATA_HEADER2
     || header.header3 != GW_DATA_HEADER3) {
      ESP_LOGW(TAG, "Error reading header");
      flush();
      return NULL;
    }

    if (header.type != command) {
      ESP_LOGW(TAG, "Received wrong command %d", header.type);
      flush();
      return NULL;
    }

    ctx_add_(&ctx, &header, sizeof(header));
    if (header.size > sizeof(data)) {
      ESP_LOGW(TAG, "Size error %d", header.size);
      flush();
      return NULL;
    }

    read_array((uint8_t *)&data, header.size);
    ctx_add_(&ctx, &data, header.size);

    read_array((uint8_t *)&footer, sizeof(footer));
    footer.checksum = ntohs(footer.checksum);

    if (footer.checksum != ctx_fin_(&ctx)){
      ESP_LOGW(TAG, "Checksum error");
      flush();
      return NULL;
    }
    return &data;
  }

  void GrowattMTLComponent::dump_config() {
    ESP_LOGCONFIG    (TAG,   "Growatt-MTL:");
    LOG_BINARY_SENSOR("   ", "Has Fault", has_fault_binary_sensor_);
    LOG_SENSOR       ("   ", "Voltage PV1", voltage_pv1_sensor_);
    LOG_SENSOR       ("   ", "Voltage PV2", voltage_pv2_sensor_);
    LOG_SENSOR       ("   ", "Power PV", power_pv_sensor_);
    LOG_SENSOR       ("   ", "Voltage AC", voltage_ac_sensor_);
    LOG_SENSOR       ("   ", "Freqiency AC", freq_ac_sensor_);
    LOG_SENSOR       ("   ", "Power AC", power_ac_sensor_);
    LOG_SENSOR       ("   ", "Temperature", temperature_sensor_);
    LOG_SENSOR       ("   ", "Energy Today", energy_today_sensor_);
    LOG_SENSOR       ("   ", "Energy Total", energy_total_sensor_);
    LOG_SENSOR       ("   ", "Total Time", total_time_sensor_);
    LOG_SENSOR       ("   ", "Power Max", power_max_sensor_);
    LOG_TEXT_SENSOR  ("   ", "Firmware Version", firmware_version_text_sensor_);
    LOG_TEXT_SENSOR  ("   ", "Manufacturer", manufacturer_text_sensor_);
    LOG_TEXT_SENSOR  ("   ", "Serial Number", serial_number_text_sensor_);
    LOG_PIN          ("   Comms LED: ", comms_pin_);
    LOG_PIN          ("   Fault LED: ", fault_pin_);

    check_uart_settings(9600, 1, esphome::uart::UART_CONFIG_PARITY_NONE, 8);
  }

  void GrowattMTLComponent::setup() {
    LOG_PIN("Comms LED: ", comms_pin_);
    comms_pin_->setup();
    comms_pin_->digital_write(0);
    LOG_PIN("Fault LED: ", fault_pin_);
    fault_pin_->setup();
    fault_pin_->digital_write(1);
    check_uart_settings(9600, 1, esphome::uart::UART_CONFIG_PARITY_NONE, 8);
  }

  void GrowattMTLComponent::update() {
    gw_data_t *d;

    comms_pin_->digital_write(0);

    send_cmd(GW_CMD_STATUS);
    if (!available()) {
      ESP_LOGE(TAG, "No status data available");
      return;
    }
    d = recv_data(GW_CMD_STATUS);
    if (!d) {
      ESP_LOGE(TAG, "Error getting status data");
      return;
    }
    has_fault_binary_sensor_->publish_state(gw_data_get_status(d) == GW_STATUS_FAULT);
    fault_pin_->digital_write(gw_data_get_status(d) == GW_STATUS_FAULT);

    status_sensor_->publish_state(gw_data_get_status(d));
    fault_code_sensor_->publish_state(gw_data_get_fault_code(d));
    fault_text_text_sensor_->publish_state(fault_text(gw_data_get_fault_code(d)));
    voltage_pv1_sensor_->publish_state(gw_data_get_voltage_pv1(d));
    voltage_pv2_sensor_->publish_state(gw_data_get_voltage_pv2(d));
    power_pv_sensor_->publish_state(gw_data_get_power_pv(d));
    voltage_ac_sensor_->publish_state(gw_data_get_voltage_ac(d));
    freq_ac_sensor_->publish_state(gw_data_get_freq_ac(d));
    power_ac_sensor_->publish_state(gw_data_get_power_ac(d));
    temperature_sensor_->publish_state(gw_data_get_temperature(d));

    send_cmd(GW_CMD_ENERGY);
    d = recv_data(GW_CMD_ENERGY);
    if (!d) {
      ESP_LOGE(TAG, "Error getting energy data");
      return;
    }

    energy_today_sensor_->publish_state(gw_data_get_energy_today(d));
    energy_total_sensor_->publish_state(gw_data_get_energy_total(d));
    total_time_sensor_->publish_state(gw_data_get_total_time(d));

    if (!firmware_version_text_sensor_->has_state()) {
      send_cmd(GW_CMD_INFO);
      d = recv_data(GW_CMD_INFO);
      if (!d) {
        ESP_LOGE(TAG, "Error getting info data");
        return;
      }
      power_max_sensor_->publish_state(gw_data_get_power_max(d));
      firmware_version_text_sensor_->publish_state(gw_data_get_firmware_version(d));
      manufacturer_text_sensor_->publish_state(gw_data_get_manufacturer(d));
    }

    if (!serial_number_text_sensor_->has_state()) {
      send_cmd(GW_CMD_SERIAL);
      d = recv_data(GW_CMD_SERIAL);
      if (!d) {
        ESP_LOGE(TAG, "Error getting info data");
        return;
      }

      serial_number_text_sensor_->publish_state(gw_data_get_serial(d));
    }
    comms_pin_->digital_write(1);
  }

}  // growatt-mtl
}  // esphome