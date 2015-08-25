/*
    LuxControl - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include <stdlib.h>
#include "ch.h"
#include "hal.h"

#include "can_comm.h"
#include "meas.h"
#include "spi_comm.h"

/* LuxControl ID */
#define CAN_EID             0x40

/* CAN BMS */
#define CAN_BMS_MIN         0x00
#define CAN_BMS_MAX         0x0F
#define CAN_BMS_MESSAGES_1  0x01
#define CAN_BMS_MESSAGES_2  0x02
#define CAN_BMS_MESSAGES_3  0x03
#define CAN_BMS_MESSAGES_4  0x04
#define CAN_BMS_MESSAGES_5  0x05

/* CAN Min-Max ID */
#define CAN_MIN_EID         0x10
#define CAN_MAX_EID         0x1FFFFFF

/* CAN Smarty messages */
#define CAN_SM_MIN          0x10
#define CAN_SM_MAX          0x1F
#define CAN_SM_MESSAGES_1   0x01

/* CAN Modulux messages */
#define CAN_ML_MIN          0x20
#define CAN_ML_MAX          0x2F

/* CAN Raspberry Pi messages */
#define CAN_RPY_MIN         0x30
#define CAN_RPY_MAX         0x3F

/* CAN LuxControl messages */
#define CAN_LC_MIN          0x40
#define CAN_LC_MAX          0x5F
#define CAN_LC_MESSAGES_1   0x01
#define CAN_LC_MESSAGES_2   0x02
#define CAN_LC_MESSAGES_3   0x03

enum canState
{
  CAN_BMS,
  CAN_SM,
  CAN_ML,
  CAN_RPY,
  CAN_LC,
  CAN_WAIT,
  CAN_NUM_CH
}canstate;

enum canMessages
{
  CAN_MESSAGES_1,
  CAN_MESSAGES_2,
  CAN_NUM_MESS
}canMessages;

static uint8_t dip_name[6] = {GPIOB_DIP_1, 
                              GPIOB_DIP_2, 
                              GPIOB_DIP_3, 
                              GPIOB_DIP_4, 
                              GPIOB_DIP_5, 
                              GPIOA_DIP_6};

static CANRxFrame rxmsg;
static CANTxFrame txmsg;

/*--------------------------------------------------*/
/* CAN_MCR_ABOM   ->  Automatic Bus-Off Management  */
/* CAN_MCR_AWUM   ->  Automatic Wakeup Mode         */
/* CAN_MCR_TXFP   ->  Transmit FIFO Priority        */
/* CAN_MSR_TXM    ->  Transmit Mode                 */
/* CAN_MSR_RXM    ->  Receive Mode                  */
/* CAN_BTR_LBKM   ->  Loop Back Mode (Debug)        */
/* CAN_BTR_SJW()  ->  Resynchronization Jump Width  */
/* CAN_BTR_TS2()  ->  Time Segment 2                */
/* CAN_BTR_TS1()  ->  Time Segment 1                */
/* CAN_BTR_BRP()  ->  Baud Rate Prescaler           */
/* CAN_IER_SLAKI  ->  Sleep acknowledge interrupt   */
/* _____________________________________________________________________*/
/*                  				CAN_BTR_BRP Baud 500kHz			                */
/*                                                                      */
/*		 		    F1XX		    |		F2XX        |      F4XX        		        */
/*  fPCLK   	36 MHz		  |		30 MHz      |      42 MHz			            */
/*  tPCLK		  0,027777777	|		0,033333333 |      0,023809524		        */
/*  BRP[X]  	5			      |		4           |      6        		          */
/*  tq =		  0,166666662	|		0,166666667 |      0,166666667            */
/*----------------------------------------------------------------------*/
/*
*/

static const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(11)
};

static uint16_t tx_id;
static uint16_t rx_id;
static uint16_t messages;
static bool_t switch_on;
static bool_t switch_off;

static uint32_t period; 

/*
 * Receiver thread.
 */
static WORKING_AREA(can_rx_wa, 256);
static msg_t can_rx(void *p) {
  EventListener el;

  (void)p;
  chRegSetThreadName("receiver");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);
  while(!chThdShouldTerminate()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
      continue;
    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == RDY_OK) {
      chSysLock();
      rx_id = 0;
      rx_id = rxmsg.EID >> 8;
      messages = (uint8_t)rxmsg.EID;
      period = 0;

      if(rx_id >= CAN_BMS_MIN && rx_id <= CAN_BMS_MAX){
        canstate = CAN_BMS;
        rxmsg.EID = 0;
      }
      else if(rx_id >= CAN_SM_MIN && rx_id <= CAN_SM_MAX){
        canstate = CAN_SM;
        rxmsg.EID = 0;
      }
      else if(rx_id >= CAN_ML_MIN && rx_id <= CAN_ML_MAX){
        canstate = CAN_ML;
        rxmsg.EID = 0;
      }
      else if(rx_id >= CAN_RPY_MIN && rx_id <= CAN_RPY_MAX){
        canstate = CAN_RPY;
        rxmsg.EID = 0;
      }
      else if(rx_id >= CAN_LC_MIN && rx_id <= CAN_LC_MAX){
        canstate = CAN_LC;
        rxmsg.EID = 0;
      }
      else{
        canstate = CAN_WAIT;
        rxmsg.EID = 0;
      }

      switch(canstate){

        case CAN_BMS:
          if(messages == CAN_BMS_MESSAGES_2){
            if((rxmsg.data8[0] >> 1) & 0x01)
            {
              if (switch_on)
              {
                SPV1020TURN_ON();
                palSetPad(GPIOA, GPIOA_LED4);
                switch_on = FALSE;
                switch_off = TRUE;
              }
            }
            else
            {
              if(switch_off)
              {
                SPV1020SHUT_DOWN();
                palClearPad(GPIOA, GPIOA_LED4);
                switch_off = FALSE;
                switch_on = TRUE;
              } 
            }
          }
          canstate = CAN_WAIT;
          break;
        
        case CAN_SM:
          canstate = CAN_WAIT;
          break;

        case CAN_ML:
          canstate = CAN_WAIT;
          break;

        case CAN_RPY:
          canstate = CAN_WAIT;
          break;

        case CAN_LC:
          canstate = CAN_WAIT;
          break;

        case CAN_WAIT:
          break;

        default:
          break;
      }
      chSysUnlock();
    }
  } 
  chEvtUnregister(&CAND1.rxfull_event, &el);
  return 0;
}

/*
 * Transmitter thread.
 */
static WORKING_AREA(can_tx_wa, 256);
static msg_t can_tx(void * p) {

  (void)p;
  chRegSetThreadName("transmitter");

  int tx_status;

  while (!chThdShouldTerminate()) {
    for(tx_status = 0; tx_status < CAN_NUM_MESS; tx_status ++){
      switch(tx_status){
        case CAN_MESSAGES_1:
          /* Message 1 */
          chSysLock();
          txmsg.EID = 0;
          txmsg.EID = CAN_LC_MESSAGES_1;
          txmsg.EID += tx_id << 8;

          txmsg.data8[0] = measGetValue(MEAS_NTC);
          txmsg.data8[1] = measGetValue(MEAS_IN_CURR);
          txmsg.data8[2] = measGetValue(MEAS_OUT_CURR);
          txmsg.data8[3] = 0;
          txmsg.data16[2] = SPV1020VIN();
          txmsg.data16[3] = measGetValue(MEAS_V_OUT);

          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          chSysUnlock();
          break;

        case CAN_MESSAGES_2:
          /* Message 2 */
          chSysLock();
          txmsg.EID = 0;
          txmsg.EID = CAN_LC_MESSAGES_2;
          txmsg.EID += tx_id << 8;

          txmsg.data16[0] = SPV1020STATUS();
          txmsg.data16[1] = SPV1020PWM();
          txmsg.data16[2] = SPV1020CURR_IN();

          canTransmit(&CAND1, CAN_ANY_MAILBOX ,&txmsg, MS2ST(100));
          chSysUnlock();
          break;

        default:      
          break;
      }
      chThdSleepMilliseconds(250);
    }

    chThdSleepMilliseconds(1000);
  }
  return 0;
}

/*
 * CAN safety thread.
 */
static WORKING_AREA(safety_wa, 256);
static msg_t safety(void * p) {

  (void)p;
  chRegSetThreadName("safety");
  while (TRUE) {
    period ++;
    if ((period > 50) && switch_off)
    {
      /*
       * SPV1020 Turn off.
       */
      SPV1020SHUT_DOWN();
      palClearPad(GPIOA, GPIOA_LED4);
      switch_off = FALSE;
      switch_on = TRUE;
    }
    chThdSleepMilliseconds(20);
  }
  return 0; /* Never executed.*/
}

void can_commInit(void){

  tx_id = CAN_EID;

  tx_id += (palReadPad(GPIOB, dip_name[0]) == 0) ? 0x01 : 0x00;
  tx_id += (palReadPad(GPIOB, dip_name[1]) == 0) ? 0x02 : 0x00;
  tx_id += (palReadPad(GPIOB, dip_name[2]) == 0) ? 0x04 : 0x00;
  tx_id += (palReadPad(GPIOB, dip_name[3]) == 0) ? 0x08 : 0x00;
  tx_id += (palReadPad(GPIOB, dip_name[4]) == 1) ? 0x10 : 0x00;
  tx_id += (palReadPad(GPIOA, dip_name[5]) == 0) ? 0x20 : 0x00;

  tx_id = tx_id < CAN_LC_MIN ? CAN_LC_MIN : tx_id;
  tx_id = tx_id > CAN_LC_MAX ? CAN_LC_MAX : tx_id;
  
  canStart(&CAND1, &cancfg);

  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);
  chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
  chThdCreateStatic(safety_wa, sizeof(safety_wa), NORMALPRIO + 7, safety, NULL);

  txmsg.IDE = CAN_IDE_EXT;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;

  rxmsg.IDE = CAN_IDE_EXT;
  rxmsg.RTR = CAN_RTR_DATA;
  rxmsg.DLC = 8;

  switch_on = TRUE;
  switch_off = TRUE;

  period = 0;
}