#ifndef _GROWATT_PROTO_H_
#define _GROWATT_PROTO_H_

#include <inttypes.h>
#include "lwip/ip.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"

/* Command data structure */
static uint8_t GW_CMD_HEADER[] = {0x3F, 0x23};

#define GROWATT_TIMEOUT 2

#define GW_ADDRESS_BROADCAST 0x7E

typedef struct gw_cmd_s gw_cmd_t;
struct gw_cmd_s {
  uint8_t header[2];
  uint8_t addr;
  uint8_t c0;
  uint8_t c1;
  uint8_t size;
  uint16_t checksum;
} __attribute__((packed));



/* Data response data structures */
static uint8_t GW_DATA_HEADER[] = {0x23, 0x3F};
static uint8_t GW_DATA_HEADER_TAIL[] = {0x32};

typedef struct gw_data_header_s gw_data_header_t;
struct gw_data_header_s {
  uint8_t header[2];
  uint8_t addr;
  uint8_t header_tail[1];
  uint8_t type;
  uint8_t size;
} __attribute__((packed));

typedef struct gw_data_footer_s gw_data_footer_t;
struct gw_data_footer_s {
  uint16_t checksum;
} __attribute__((packed));


/* Command Codes */
#define GW_OP_CODE 0x31
#define GW_OP_ADDR_RESET 0x44


#define GW_CMD_CODE 0x32
#define GW_CMD_STATUS     0x41
#define GW_CMD_ENERGY     0x42
#define GW_CMD_INFO       0x43
#define GW_CMD_SERIAL     0x53

/* Status Codes */
#define GW_STATUS_WAITING  0
#define GW_STATUS_NORMAL   1
#define GW_STATUS_FAULT    3

/* Error Codes */
#define GW_ERROR_AUTO_TEST_FAIL   24
#define GW_ERROR_NO_AC_CONNECTION 25
#define GW_ERROR_PV_ISOLATION_LOW 26
#define GW_ERROR_RESIDUAL_I_HIGH  27
#define GW_ERROR_OUTPUT_HIGH_DCI  28
#define GW_ERROR_PV_VOLTAGE_HIGH  29
#define GW_ERROR_AC_V_OUTOFRANGE  30
#define GW_ERROR_AC_F_OUTOFRANGE  31
#define GW_ERROR_MODULE_HOT       32

/* Multiplers for data values */
#define GW_MULTIPLY_VOLTAGE     10.0
#define GW_MULTIPLY_POWER       10.0
#define GW_MULTIPLY_ENERGY      10.0
#define GW_MULTIPLY_FREQUENCY   100.0
#define GW_MULTIPLY_TEMPERATURE 10.0
#define GW_MULTIPLY_HOURS       10.0

/* Status data structure */
typedef struct gw_data_status_s gw_data_status_t;
struct gw_data_status_s {
  uint8_t   status;
  uint16_t  voltage_pv1;
  uint16_t  voltage_pv2;
  uint16_t  power_pv;
  uint16_t  voltage_ac;
  uint16_t  _blank1;
  uint16_t  freq_ac;
  uint16_t  power_ac;
  uint8_t   _blank2[14];
  uint16_t  fault;
  uint16_t  temperature;
  uint16_t  _blank3;
} __attribute__((packed));


/* Energy data structure */
typedef struct gw_data_energy_s gw_data_energy_t;
struct gw_data_energy_s {
  uint8_t  _blank1[7];
  uint16_t energy_today;
  uint32_t energy_total;
  uint32_t total_time;
  uint32_t  _blank2;
} __attribute__((packed));


/* Info data structure */
#define GW_DATA_INFO_MAGIC 0x31
typedef struct gw_data_info_s gw_data_info_t;
struct gw_data_info_s {
  uint8_t  magic;
  uint32_t power_max;
  uint16_t voltage_dc_nor;
  uint16_t model;
  uint8_t  firmware_version[5];
  uint8_t  manufacturer[16];
  uint8_t _blank1[1];
} __attribute__((packed));


/* Serial data structure */
typedef struct gw_data_serial_s gw_data_serial_t;
struct gw_data_serial_s {
  uint8_t  serial[10];
} __attribute__((packed));

typedef struct gw_data_s gw_data_t;
struct gw_data_s {
  gw_data_header_t header;
  union {
    gw_data_status_t status;
    gw_data_energy_t energy;
    gw_data_info_t   info;
    gw_data_serial_t serial;
  } data;
  gw_data_footer_t footer;
};

#define gw_data_get_voltage_pv1(d) (ntohs(d->data.status.voltage_pv1) / GW_MULTIPLY_VOLTAGE)
#define gw_data_get_voltage_pv2(d) (ntohs(d->data.status.voltage_pv2) / GW_MULTIPLY_VOLTAGE)
#define gw_data_get_power_pv(d) (ntohs(d->data.status.power_pv) / GW_MULTIPLY_POWER)
#define gw_data_get_voltage_ac(d) (ntohs(d->data.status.voltage_ac) / GW_MULTIPLY_VOLTAGE)
#define gw_data_get_power_ac(d) (ntohs(d->data.status.power_ac) / GW_MULTIPLY_POWER)
#define gw_data_get_freq_ac(d) (ntohs(d->data.status.freq_ac) / GW_MULTIPLY_FREQUENCY)

#define gw_data_get_status(d) ntohs(d->data.status.status)
#define gw_data_get_fault_code(d) ntohs(d->data.status.fault)
#define gw_data_get_temperature(d) (ntohs(d->data.status.temperature) / GW_MULTIPLY_TEMPERATURE)

#define gw_data_get_energy_today(d) (ntohs(d->data.energy.energy_today) / GW_MULTIPLY_ENERGY)
#define gw_data_get_energy_total(d) (ntohl(d->data.energy.energy_total) / GW_MULTIPLY_ENERGY)
#define gw_data_get_total_time(d) (ntohl(d->data.energy.total_time) / GW_MULTIPLY_HOURS)

#define gw_data_get_power_max(d) (ntohl(d->data.info.power_max) / GW_MULTIPLY_POWER)

#define to_string(s) std::string(std::begin(s), std::find(std::begin(s), std::end(s), '\0'))

#define gw_data_get_firmware_version(d) (to_string(d->data.info.firmware_version))
#define gw_data_get_manufacturer(d) (to_string(d->data.info.manufacturer))

#define gw_data_get_serial(d) (to_string(d->data.serial.serial))


#endif