/*
    LuxControl - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#ifndef CAN_COMM_H_INCLUDED
#define CAN_COMM_H_INCLUDED

void can_commInit(void);
void can_commCalc(void);

void cmd_can_commvalues(BaseSequentialStream *chp, int argc, char *argv[]);

#endif