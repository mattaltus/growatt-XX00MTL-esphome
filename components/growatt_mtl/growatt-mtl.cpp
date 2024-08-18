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
    switch (fault){
      case GW_ERROR_AUTO_TEST_FAIL:
        return "Auto test failure";
      case GW_ERROR_NO_AC_CONNECTION:
        return "No AC connection";
      case GW_ERROR_PV_ISOLATION_LOW:
        return "PV isolation too low";
      case GW_ERROR_RESIDUAL_I_HIGH:
        return "Residual too high";
      case GW_ERROR_OUTPUT_HIGH_DCI:
        return "DC output too high";
      case GW_ERROR_PV_VOLTAGE_HIGH:
        return "PV voltage too high";
      case GW_ERROR_AC_V_OUTOFRANGE:
        return "AC voltage out of range";
      case GW_ERROR_AC_F_OUTOFRANGE:
        return "AC frequency out of range";
      case GW_ERROR_MODULE_HOT:
        return "Module too hot";
      default:
        return "Unknown Error";
    }
  }

  void GrowattMTLComponent::send_cmd(int command) {
    struct ctx ctx;
    gw_cmd_t buf = {0};

    buf.header[0] = GW_CMD_HEADER[0];
    buf.header[1] = GW_CMD_HEADER[1];
    buf.addr   = GW_ADDRESS_BROADCAST;
    buf.c0     = GW_CMD_CODE;
    buf.c1     = command;
    buf.size   = 0;

    ctx_ini_(&ctx);
    ctx_add_(&ctx, &buf, sizeof(buf) - sizeof(buf.checksum));
    buf.checksum = ctx_fin_(&ctx);

    flush();
    write_array((uint8_t *)&buf, sizeof(buf));
  }

  gw_data_t *GrowattMTLComponent::recv_data(int command) {
    static gw_data_t d = {0};
    int s;
    size_t exs = 0;
    struct ctx ctx = {0};

    ctx_ini_(&ctx);

    read_array((uint8_t *)&d.header, sizeof(d.header));
    if (d.header.header[0] != GW_DATA_HEADER[0]
     || d.header.header[1] != GW_DATA_HEADER[1]
     || d.header.header_tail != GW_DATA_HEADER_TAIL) {
      ESP_LOGW(TAG, "Error reading header");
      flush();
      return NULL;
    }

    if (d.header.type != command) {
      ESP_LOGW(TAG, "Received wrong command %d", d.header.type);
      flush();
      return NULL;
    }

    ctx_add_(&ctx, &d.header, sizeof(d.header));
    if (d.header.size > sizeof(d.data)) {
      ESP_LOGW(TAG, "Size error %d", d.header.size);
      flush();
      return NULL;
    }

    read_array((uint8_t *)&d.data, d.header.size);
    ctx_add_(&ctx, &d.data, d.header.size);

    read_array((uint8_t *)&d.footer, sizeof(d.footer));
    d.footer.checksum = ntohs(d.footer.checksum);

    if (d.footer.checksum != ctx_fin_(&ctx)){
      ESP_LOGW(TAG, "Checksum error");
      flush();
      return NULL;
    }
    return &d;
  }

  void GrowattMTLComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "Growatt-MTL:");
    LOG_BINARY_SENSOR("   ", "Has Fault", has_fault_binary_sensor_);
    LOG_SENSOR("   ", "Voltage PV1", voltage_pv1_sensor_);
    LOG_SENSOR("   ", "Voltage PV2", voltage_pv2_sensor_);
    LOG_SENSOR("   ", "Power PV", power_pv_sensor_);
    LOG_SENSOR("   ", "Voltage AC", voltage_ac_sensor_);
    LOG_SENSOR("   ", "Freqiency AC", freq_ac_sensor_);
    LOG_SENSOR("   ", "Power AC", power_ac_sensor_);
    LOG_SENSOR("   ", "Temperature", temperature_sensor_);
    LOG_SENSOR("   ", "Energy Today", energy_today_sensor_);
    LOG_SENSOR("   ", "Energy Total", energy_total_sensor_);
    LOG_SENSOR("   ", "Total Time", total_time_sensor_);
    LOG_SENSOR("   ", "Power Max", power_max_sensor_);
    LOG_TEXT_SENSOR("   ", "Firmware Version", firmware_version_text_sensor_);
    LOG_TEXT_SENSOR("   ", "Manufacturer", manufacturer_text_sensor_);
    LOG_TEXT_SENSOR("   ", "Serial Number", serial_number_text_sensor_);

    check_uart_settings(9600, 1, esphome::uart::UART_CONFIG_PARITY_NONE, 8);
  }

  void GrowattMTLComponent::setup() {
    comms_pin_->pin_mode(esphome::gpio::FLAG_OUTPUT);
    fault_pin_->pin_mode(esphome::gpio::FLAG_OUTPUT);
    check_uart_settings(9600, 1, esphome::uart::UART_CONFIG_PARITY_NONE, 8);
  }

  void GrowattMTLComponent::update() {
    gw_data_t *d;

    comms_pin_->digital_write(0);

    send_cmd(GW_CMD_STATUS);
    d = recv_data(GW_CMD_STATUS);
    if (!d) {
      ESP_LOGE(TAG, "Error getting status data");
      return;
    }
    has_fault_binary_sensor_->publish_state(gw_data_get_status(d) == GW_STATUS_FAULT);
    fault_pin_->digital_write(gw_data_get_status(d) == GW_STATUS_FAULT);

    status_sensor_->publish_state(gw_data_get_status(d));
    fault_code_sensor_->publish_state(gw_data_get_fault_code(d));
    fault_text_sensor_->publish_state(fault_text(gw_data_get_fault_code(d)));
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




}  // growatt-mtk
}  // esphome