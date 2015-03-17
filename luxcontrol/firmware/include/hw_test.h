/*
    LuxControl - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef HW_TEST_H_INCLUDED
#define HW_TEST_H_INCLUDED

#include "ch.h"
#include "hal.h"

enum hw_testDip 
{
  HW_TEST_DIP1,
  HW_TEST_DIP2,
  HW_TEST_DIP3,
  HW_TEST_DIP4,
  HW_TEST_DIP5,
  HW_TEST_DIP6,
  HW_TEST_DIP_NUM
};

void hw_testInit(void);
void hw_testCalc(void);

#endif