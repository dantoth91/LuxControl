/*
    LuxControl - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef MEAS_H_INCLUDED
#define MEAS_H_INCLUDED

#include "ch.h"
#include "hal.h"

enum measChannels
{
  MEAS_NTC,
  MEAS_V_OUT,
  MEAS_IN_CURR,
  MEAS_OUT_CURR,
  MEAS_NUM_CH
};

void measInit(void);
void measCalc(void);
int16_t measGetValue(enum measChannels ch);
int16_t measInterpolateNTC(adcsample_t rawvalue);

#endif