/*
    LuxControl - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef SPI_COMM_H_INCLUDED
#define SPI_COMM_H_INCLUDED

#include "ch.h"
#include "hal.h"

uint16_t SPV1020VIN(void);
uint16_t SPV1020STATUS(void);
uint16_t SPV1020PWM(void);
uint16_t SPV1020CURR_IN(void);

void spi_commInit(void);
int SPISendData(uint8_t *tx, uint16_t *rxbuf, size_t txsize);
int SPIExchangeData(SPIDriver *spip, uint8_t *tx, uint16_t *rx, size_t rxsize);

#endif