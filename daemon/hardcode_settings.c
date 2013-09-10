#include "ctrlr.h"

struct system_data sysdata = {
  .profile = {
      .id = 0,
  },
  .nodes = {
    {
      .id = 1,
      .name = "Fridge 1",
      .setting = {
        .mode = COOL,
        .type = COMPRESSOR,
        .setpoint = -10500,
        .min_compressor_time = 180,
        .range = 2,
      },
      .temp = {
        .family_id = 0x28,
/*        .serial_no = 0x0000027b8abf, */
        .serial_no = 0,
        .lowpass_reading = 19312,
      },
      .output = {
        .power = 0,
        .state = 0,
        .lasttime = 0,
      },
    },
    {
      .id = 2,
      .name = "Fridge 2",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 16500,
        .range = 1,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .power = 50,
        .state = 1,
        .lasttime = 0,
      },
    },
    {
      .id = 3,
      .name = "Fermenter 2",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 16500,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .power = 80,
        .state = 1,
        .lasttime = 0,
      },
    },
    {
      .id = 4,
      .name = "Fermenter 3",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 17200,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .power = 20,
        .state = 0,
        .lasttime = 0,
      },
    },
    {
      .id = 5,
      .name = "Fermenter 4",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 18600,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .power = 52,
        .state = 0,
        .lasttime = 0,
      },
    },
    {
      .id = 6,
      .name = "Fermenter 5",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 19300,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .power = 17,
        .state = 0,
        .lasttime = 0,
      },
    },
    {
      .id = 7,
      .name = "Fermenter 6",
      .setting = {
        .mode = HEAT,
        .type = PID,
        .setpoint = 20400,
      },
      .temp = {
        .family_id = 0,
        .serial_no = 0,
      },
      .output = {
        .power = 0,
        .state = 0,
        .lasttime = 0,
      },
    },
  },
};
