/*
    LuxControl - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "spi_comm.h"

#define NOP                   0x01
#define SHUT_DOWN             0x02
#define TURN_ON               0x03
#define READ_CURRENT_10_BIT   0x04 
#define READ_VIN_10_BIT       0x05 
#define READ_PWM_9_BIT        0x06 
#define READ_STATUS           0x07 

/*
 * Low speed SPI configuration (281.250kHz, CPHA=0, CPOL=0, MSb first).
 */
static const SPIConfig ls_spicfg_cs1 = {
  NULL,
  GPIOA,
  GPIOA_RXD1,
  SPI_CR1_BR_2 | SPI_CR1_BR_1
};

static uint8_t txbuf;
static uint16_t rxbuf[2];
static uint8_t txbuf_nop[2];

uint16_t SPV1020VIN(void) {
  int rx, tx;

  tx = READ_VIN_10_BIT;

  SPISendData(&tx, &rx, 1);
  
  return (uint16_t)rx;
}

uint16_t SPV1020STATUS(void) {
  int rx, tx;

  tx = READ_STATUS;
  
  SPISendData(&tx, &rx, 1);
  
  return (uint16_t)rx;
}

uint16_t SPV1020PWM(void) {
  int rx, tx;

  tx = READ_PWM_9_BIT;
  
  SPISendData(&tx, &rx, 1);
  
  return (uint16_t)rx;
}

int SPISendData(uint8_t *tx, uint16_t *rxbuf, size_t txsize) {

  spiAcquireBus(&SPID1);
  spiStart(&SPID1, &ls_spicfg_cs1);
  spiSelect(&SPID1);

  spiSend(&SPID1, txsize, tx);

  spiUnselect(&SPID1);
  spiStop(&SPID1);
  spiReleaseBus(&SPID1);

  SPIExchangeData(&SPID1, txbuf_nop, rxbuf, 2);
  
  return 0;
}

int SPIExchangeData(SPIDriver *spip, uint8_t *tx, uint16_t *rx, size_t rxsize) {

  spiAcquireBus(spip);
  spiStart(spip, &ls_spicfg_cs1);
  spiSelect(spip);

  spiExchange(spip, rxsize, tx, rx);

  spiUnselect(spip);
  spiStop(spip);
  spiReleaseBus(spip);
  
  return 0;
}

void spi_commInit(void) {
  int tx, rx;
  
  tx = TURN_ON;

  txbuf_nop[0] = 0x01;
  txbuf_nop[1] = 0x01;

  SPISendData(&tx, &rx, 1);
}
