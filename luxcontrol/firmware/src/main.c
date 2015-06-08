/*
    LuxControl - Copyright (C) 2014
    GAMF MegaLux Team              
*/


#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "can_comm.h"
#include "hw_test.h"
#include "spi_comm.h"
#include "meas.h"


/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    systime_t time;
    time = 50;
    palSetPad(GPIOA, GPIOA_LED4);
    chThdSleepMilliseconds(time);

    time = 450;
    palClearPad(GPIOA, GPIOA_LED4);
    chThdSleepMilliseconds(time);
  }
  return 0; /* Never executed.*/
}
/*
 * 20ms Task
 */
static WORKING_AREA(watask20ms, 256);
static msg_t task20ms(void *arg) {
  systime_t time; 

  (void)arg;
  chRegSetThreadName("task20ms");
  time = chTimeNow();  
  while (TRUE) {
    time += MS2ST(20);

    //hw_testCalc();
    measCalc();

    chThdSleepUntil(time);
  }
  return 0; /* Never executed.*/
}
/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  
  /*
   * Hardware test initialization.
   */
  //hw_testInit();

  /*
   * Measurement initialization.
   */
  measInit();

  /*
   * CAN bus initialization.
   */
  can_commInit();

  /*
   * SPI bus initialization.
   */
  spi_commInit();

  /*
   * Creates the 20ms Task.
   */
  chThdCreateStatic(watask20ms, sizeof(watask20ms), NORMALPRIO, task20ms, NULL);
  
  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE) {
    chThdSleepMilliseconds(1000);
  }
}