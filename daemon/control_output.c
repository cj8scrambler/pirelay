#include <stdio.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <bcm2835.h>

#include "ctrlr.h"

enum power_state power_is_on(struct node_data *data)
{
  return (data->output.state);
}

int update_outputs(struct node_data *data) {

  int i;
  char spi_data[NUM_NODES];

  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256); /* 1MHz */
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
  DEBUG("outputs:");
  for (i=0; i< NUM_NODES; i++) {
    if (data[i].output.state == OFF)
      spi_data[i] = 0;
    else
      spi_data[i] = data[i].output.power;
    DEBUG_DUMP("  0x%02x", data[i].output.power);
 
  }
  DEBUG_DUMP("\n");
  bcm2835_spi_transfern(spi_data, NUM_NODES);
  bcm2835_spi_end();
  return 0;
}

void output_disable_all(struct node_data *data){
  int i;

  for (i=0; i< NUM_NODES; i++){
    data[i].output.state = OFF;
  }
  update_outputs(data);
}

