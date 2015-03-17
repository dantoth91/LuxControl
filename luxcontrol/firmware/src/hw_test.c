/*
    LuxControl - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "hw_test.h"

static uint8_t i = 0;
static uint8_t dip_value[6] = {0};
static uint8_t dip_name[6] = {GPIOB_DIP_1, GPIOB_DIP_2, GPIOB_DIP_3, GPIOB_DIP_4, GPIOB_DIP_5, GPIOA_DIP_6};
static int pins[5] = {GPIOA_SPI_CS2, GPIOA_SPI_MISO, GPIOA_SPI_CLK, GPIOA_SPI_MOSI, GPIOA_SPI_CS1};


void hw_testInit(void){
	palClearPad(GPIOA, pins[0]);
	palClearPad(GPIOA, pins[1]);
	palClearPad(GPIOA, pins[2]);
	palClearPad(GPIOA, pins[3]);
	palClearPad(GPIOA, pins[4]);
}

void hw_testCalc(void){
	int ch;

	palClearPad(GPIOA, pins[i]);
    i++;
    if(i > 4){
      i = 0;
    }
    palSetPad(GPIOA, pins[i]);

/*    for(ch = 0; ch < 1; ch++) {
    	if(palReadPad(GPIOB, dip_name[ch]) == 0)
	        dip_value[ch] = 0;
	    else
	    	dip_value[ch] = 1;
    }

    for(ch = 0; ch < HW_TEST_DIP1_NUM; ch++){
    	if (dip_value[ch] == 1)
    	{
    		palClearPad(GPIOA, GPIOA_LED4);
    		ch = HW_TEST_DIP1_NUM;
    	}
    	else
    		palSetPad(GPIOA, GPIOA_LED4);
    }*/

    if(palReadPad(GPIOB, dip_name[0]) == 0)
        dip_value[0] = 0;
    else
        dip_value[0] = 1;

    if(palReadPad(GPIOB, dip_name[1]) == 0)
        dip_value[1] = 0;
    else
        dip_value[1] = 1;

   	if (dip_value[0] == 1){
    	palClearPad(GPIOA, GPIOA_LED4);
    	}
    else
    	palSetPad(GPIOA, GPIOA_LED4);

    if (dip_value[1] == 1){
    	palClearPad(GPIOA, GPIOA_LED4);
    	}
    else
    	palSetPad(GPIOA, GPIOA_LED4);

    /*if (dip_value[2] == 1){
    	palClearPad(GPIOA, GPIOA_LED4);
    	}
    else
    	palSetPad(GPIOA, GPIOA_LED4);

    if (dip_value[3] == 1){
    	palClearPad(GPIOA, GPIOA_LED4);
    	}
    else
    	palSetPad(GPIOA, GPIOA_LED4);

    if (dip_value[4] == 1){
    	palClearPad(GPIOA, GPIOA_LED4);
    	}
    else
    	palSetPad(GPIOA, GPIOA_LED4);

    if (dip_value[5] == 1){
    	palClearPad(GPIOA, GPIOA_LED4);
    	}
    else
    	palSetPad(GPIOA, GPIOA_LED4);*/
}