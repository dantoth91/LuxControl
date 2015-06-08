/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "meas.h"
#include "spi_comm.h"

#define ADC_GRP1_BUF_DEPTH      8

enum measStates
{
  MEAS_START,
  MEAS_RUNING
} measstate;

/*
 * Calibration of NTC temperature from -55 C to 155 C in 5 C steps
 */
#define MEAS_NTCCAL_NUM 43
#define MEAS_NTCCAL_START -55
#define MEAS_NTCCAL_STEP 5
const adcsample_t measNTCcalib[] = {
  4054, 4036, 4011, 3978, 3934, 3877, 3804, 3713, 3602, 3469,
  3313, 3136, 2939, 2726, 2503, 2275, 2048, 1828, 1618, 1424,
  1245, 1085,  942,  816,  706,  611,  528,  458,  397,  344,
   300,  261,  228,  199,  175,  153,  135,  120,  106,   94,
    83,   74,   66
} ;

static int32_t measValue[MEAS_NUM_CH +2];
static adcsample_t samples[MEAS_NUM_CH * ADC_GRP1_BUF_DEPTH];

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err);
int16_t measInterpolateNTC(adcsample_t rawvalue);

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 4 channel, SW triggered.
 * Channels:    IN1.
 * SMPR1 - CH 10...17,               |	SMPR2 - CH 0...9  |
 * SQR1 - 13...16 + sequence length, |	SQR2 - 7...12, 	  |	SQR3 - 1...6
 */
static const ADCConversionGroup adcgrpcfg = {
  FALSE,
  MEAS_NUM_CH,
  NULL,
  adcerrorcallback,
  0,                                                                            /* CR1 */
  0,                                                                            /* CR2 */
  0,									   								                                        /* SMPR1 | 				         |                | */
  ADC_SMPR2_SMP_AN9(ADC_SAMPLE_13P5)  |  ADC_SMPR2_SMP_AN8(ADC_SAMPLE_13P5) |   /* SMPR2 | AN9-PB1-OUT_CURR| AN8-PB0-IN_CURR| */
  ADC_SMPR2_SMP_AN2(ADC_SAMPLE_13P5)  |  ADC_SMPR2_SMP_AN3(ADC_SAMPLE_13P5 ),	  /* SMPR2 | AN2-PA2-V_OUT   | AN3-PA3-NTC    | */
  ADC_SQR1_NUM_CH(MEAS_NUM_CH),                                                 /* SQR1  --------- Number of sensors -------- */
  0,    									   							                                      /* SQR2  |					       |                | */
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN9) | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN8) |           /* SQR3  | 4. IN9-OUT_CURR | 3. IN8-IN_CURR | */
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN2) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN3)             /* SQR3  | 2. IN2-V_OUT    | 1. IN3-NTC     | */
};

void measInit(void){

  /*
  * Activates the ADC3 driver.
  */
  adcStart(&ADCD1, NULL);
  measstate = MEAS_START;
}

void measCalc(void){
  int ch;
  int avg, i, txbuf; 

  /*
  * Starts the ADC conversion.
  */
  switch(measstate){

    case MEAS_START:
      measstate = MEAS_RUNING;
      break;
    case MEAS_RUNING:
      for(ch = 0; ch < MEAS_NUM_CH; ch++) {
        avg = 0;
        for(i = 0; i < ADC_GRP1_BUF_DEPTH; i ++) {
          avg += samples[ch + MEAS_NUM_CH * i];
        }
        avg /= ADC_GRP1_BUF_DEPTH;
        switch(ch){
          case MEAS_NTC:
          	avg = measInterpolateNTC(avg);
            avg = avg < 0 ? 0 : avg;
            avg = avg > 255 ? 255 : avg;
            break;
          case MEAS_V_OUT:
            avg *= 2;
            avg /= 13;
            avg = avg > 65535 ? 65535 : avg;
            break;
          case MEAS_IN_CURR:
            avg = avg > 2048 ? 2048 : avg;
            avg = 2048 - avg;
            avg *= 1475;
            avg /= 10000;
            avg = avg > 255 ? 255 : avg;
            break;
          case MEAS_OUT_CURR:
            avg = avg < 2048 ? 2049 : avg;
            avg -= 2048;
            avg *= 1475;
            avg /= 10000;
            avg = avg > 255 ? 255 : avg;
            break;
          default:
            break;
        }
        chSysLock();
        measValue[ch] = (int16_t)avg;
        chSysUnlock();
      }
      break;
    default:
      break;
  }
  adcConvert(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);
}

int16_t measGetValue(enum measChannels ch){
	  return measValue[ch];
}

int16_t measInterpolateNTC(adcsample_t rawvalue){
  int16_t value = 0;
  adcsample_t left, right;
  int i;
  
  if(rawvalue >= measNTCcalib[0]){
    value = MEAS_NTCCAL_START;
  }
  else if(rawvalue < measNTCcalib[MEAS_NTCCAL_NUM - 1]){
    value = MEAS_NTCCAL_START + (MEAS_NTCCAL_NUM - 1) * MEAS_NTCCAL_STEP;
  }
  else {
    i = 0;
    left = measNTCcalib[i];
    right = measNTCcalib[i + 1];
    while(rawvalue < right && i < MEAS_NTCCAL_NUM - 1){
      i++;
      left = right;
      right = measNTCcalib[i + 1];
    }
    value = MEAS_NTCCAL_START + 
            i * MEAS_NTCCAL_STEP + 
            (MEAS_NTCCAL_STEP * (left - rawvalue)) / (left - right);
  }
  return value;
}

/*
 * ADC error callback. TODO: utilize
 */
static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;
}