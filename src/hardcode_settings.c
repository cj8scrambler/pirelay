#include "ctrlr.h"

struct system_data sysdata = {
  .nodes = {
    {
      .name = "Fridge 1",
      .setting = {
        .mode = COOL,
        .type = COMPRESSOR,
        .setpoint = 40,
        .min_compressor_time = 180,
        .range = 2,
      },
      .temp = {
        .family_id = 0x28,
        .serial_no = 0x0000027b8abf,
//        .lowpass_reading = 19312,
      },
      .output = {
        .gpio = 8, /* SPI0_CE0 */
        .power = 0,
        .state = 0,
        .lasttime = 0,
      },
    },
    {
      .name = "Fridge 2",
      .setting = {
        .mode = COOL,
        .type = ON_OFF,
        .setpoint = 55,
        .range = 1,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .gpio = 7, /* SPI0_CE1 */
        .power = 0,
        .state = 0,
        .lasttime = 0,
      },
    },
    {
      .name = "Fermenter 2",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 60,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .gpio = 17,
        .power = 100,
        .state = 1,
        .lasttime = 0,
      },
    },
    {
      .name = "Fermenter 3",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 64,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .gpio = 18,
        .power = 80,
        .state = 0,
        .lasttime = 0,
      },
    },
    {
      .name = "Fermenter 4",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 69,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .gpio = 22,
        .power = 52,
        .state = 0,
        .lasttime = 0,
      },
    },
    {
      .name = "Fermenter 5",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 72,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .gpio = 23,
        .power = 17,
        .state = 0,
        .lasttime = 0,
      },
    },
    {
      .name = "Fermenter 6",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 76,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .gpio = 24,
        .power = 0,
        .state = 0,
        .lasttime = 0,
      },
    },
  },
};
