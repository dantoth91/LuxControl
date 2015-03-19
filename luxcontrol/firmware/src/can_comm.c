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

#define MIN_CAN_EID         0x40
#define MAX_CAN_EID         0x5F


enum canComands
{
  CAN_TEST = 0x01234567,
  CAN_NUM_COMM
}canComands;

static uint8_t dip_name[6] = {GPIOB_DIP_1, 
                              GPIOB_DIP_2, 
                              GPIOB_DIP_3, 
                              GPIOB_DIP_4, 
                              GPIOB_DIP_5, 
                              GPIOA_DIP_6};

static CANRxFrame rxmsg;
static CANTxFrame txmsg;
static uint16_t id;

uint32_t canRxEID;

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
/* _____________________________________________________________________*/
/*                  				CAN_BTR_BRP Baud   					                */
/*	tPCLK = (1/fPCLK)													                          */
/*	tq = (BRP[9:0]+1) x tPCLK											                      */
/*																		                                  */
/*		 		    F1XX		    |		F2XX        |      F4XX        		        */
/*  fPCLK   	36 MHz		  |		30 MHz      |      42 MHz			            */
/*  tPCLK		  0,027777777	|		0,033333333 |      0,023809524		        */
/*  BRP[X]  	5			      |		4           |      6        		          */
/*  tq =		  0,166666662	|		0,166666667 |      0,166666667            */
/*----------------------------------------------------------------------*/

static const CANConfig cancfg = {
  CAN_MCR_ABOM,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(5)
};

/*
 * Receiver thread.
 */
static WORKING_AREA(can_rx_wa, 256);
static msg_t can_rx(void *p) {
  EventListener el;
  msg_t can_tx_msg=0;

  (void)p;
  chRegSetThreadName("receiver");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);
  while(!chThdShouldTerminate()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
      continue;
    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == RDY_OK) {

      switch(rxmsg.EID){
        case CAN_TEST:

          /* Message 1 */
          txmsg.EID = 0;
          txmsg.EID = 0x01;
          txmsg.EID += id << 8;

          txmsg.data8[0] = measGetValue(MEAS_NTC);
          txmsg.data8[1] = measGetValue(MEAS_IN_CURR);
          txmsg.data8[2] = measGetValue(MEAS_OUT_CURR);
          txmsg.data8[3] = 0;
          txmsg.data16[2] = (uint16_t)((SPV1020VIN() + 65383) / 1093);
          txmsg.data16[3] = measGetValue(MEAS_V_OUT);
          

          can_tx_msg = canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
          rxmsg.EID = 0;

          /* Message 2 */
          txmsg.EID = 0;
          txmsg.EID = 0x02;
          txmsg.EID += id << 8;
          txmsg.data16[0] = SPV1020STATUS();
          txmsg.data16[1] = SPV1020PWM();

          txmsg.data32[1] = 0;
          

          can_tx_msg = canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
          rxmsg.EID = 0;

          palTogglePad(GPIOA, GPIOA_LED4);

          chThdSleepMilliseconds(200);
          break;
        default:
          break;
      }
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
  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x00FF00FF;

  while (!chThdShouldTerminate()) {
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
    chThdSleepMilliseconds(100);
  }
  return 0;
}

void can_commInit(void){

  txmsg.EID = MIN_CAN_EID;
  id = MIN_CAN_EID;

  id += (palReadPad(GPIOB, dip_name[0]) == 0) ? 0x01 : 0x00;
  id += (palReadPad(GPIOB, dip_name[1]) == 0) ? 0x02 : 0x00;
  id += (palReadPad(GPIOB, dip_name[2]) == 0) ? 0x04 : 0x00;
  id += (palReadPad(GPIOB, dip_name[3]) == 0) ? 0x08 : 0x00;
  id += (palReadPad(GPIOB, dip_name[4]) == 1) ? 0x10 : 0x00;
  id += (palReadPad(GPIOA, dip_name[5]) == 0) ? 0x20 : 0x00;

  id = id < MIN_CAN_EID ? MIN_CAN_EID : id;
  id = id > MAX_CAN_EID ? MAX_CAN_EID : id;

  txmsg.IDE = CAN_IDE_EXT;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;

  canStart(&CAND1, &cancfg);

  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);
  //chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
}

void can_commCalc(void){
}