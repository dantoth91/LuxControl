/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Board identifier.
 */
#define BOARD_LUXCONTROL
#define BOARD_NAME              "LuxControl"

/*
 * Board supply voltage.
 */
#define STM32_VDD               330

/*
 * Board frequencies.
 */
#define STM32_LSECLK            0
#define STM32_HSECLK            8000000

/*
 * MCU type as defined in the ST header file stm32f4xx.h.
 */
#define STM32F10X_MD

/*
 * IO pins assignments.
 */

#define GPIOA_WKUP              0
#define GPIOA_LED4              1
#define GPIOA_V_OUT             2
#define GPIOA_NTC               3
#define GPIOA_SPI_CS2           4
#define GPIOA_SPI_CLK           5
#define GPIOA_SPI_MISO          6
#define GPIOA_SPI_MOSI          7
#define GPIOA_DIP_6             8
#define GPIOA_TXD1              9
#define GPIOA_RXD1             10
#define GPIOA_CAN_RX           11
#define GPIOA_CAN_TX           12
#define GPIOA_SWDIO            13
#define GPIOA_SWCLK            14
#define GPIOA_SPI_CS1          15

#define GPIOB_V_IN_CURR         0
#define GPIOB_V_OUT_CURR        1
#define GPIOB_DIP_5             3
#define GPIOB_DIP_4             4
#define GPIOB_DIP_3             5
#define GPIOB_DIP_2             6
#define GPIOB_DIP_1             7

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 *
 * The digits have the following meaning:
 *   0 - Analog input.
 *   1 - Push Pull output 10MHz.
 *   2 - Push Pull output 2MHz.
 *   3 - Push Pull output 50MHz.
 *   4 - Digital input.
 *   5 - Open Drain output 10MHz.
 *   6 - Open Drain output 2MHz.
 *   7 - Open Drain output 50MHz.
 *   8 - Digital input with PullUp or PullDown resistor depending on ODR.
 *   9 - Alternate Push Pull output 10MHz.
 *   A - Alternate Push Pull output 2MHz.
 *   B - Alternate Push Pull output 50MHz.
 *   C - Reserved.
 *   D - Alternate Open Drain output 10MHz.
 *   E - Alternate Open Drain output 2MHz.
 *   F - Alternate Open Drain output 50MHz.
 * Please refer to the STM32 Reference Manual for details.
 */

/*
 * Port A setup.
 * Everything input with pull-up except:
 * PA1  - Push-pull output, initially low state.    (LED4)
 * PA2  - Analog input.                             (Output Voltage)
 * PA3  - Analog input.                             (NTC)
 * PA4  - Push Pull output 50MHz.                   (SPI_CS2)
 * PA5  - Alternate Push Pull output 50MHz.         (SPI_CLK)
 * PA6  - Digital input.                            (SPI_MISO)
 * PA7  - Alternate Push Pull output 50MHz.         (SPI_MOSI)
 * PA8  - Digital input.                            (DIP_6)
 * PA10 - Push Pull output 50MHz                    (RXD1 - CS1)
 * PA11 - Alternate Push Pull output 10MHz.         (CAN_RX)
 * PA12 - Alternate Push Pull output 10MHz.         (CAN_TX)
 */
#define VAL_GPIOACRL            0xB4B30038      /*  PA7...PA0 */
#define VAL_GPIOACRH            0x88894384      /* PA15...PA8 */
#define VAL_GPIOAODR            0xFFFFFFFF

/*
 * Port B setup.
 * Everything input with pull-up except:
 * PB0  - Analog input.                             (Input Current)
 * PB1  - Analog input.                             (Output Current)
 * PB3  - Digital input.                            (DIP_5)
 * PB4  - Digital input.                            (DIP_4)
 * PB5  - Digital input.                            (DIP_3)
 * PB6  - Digital input.                            (DIP_2)
 * PB7  - Digital input.                            (DIP_1)
 */
#define VAL_GPIOBCRL            0x44444800      /*  PB7...PB0 */
#define VAL_GPIOBCRH            0x88888888      /* PB15...PB8 */
#define VAL_GPIOBODR            0xFFFFFFFF

/*
 * Port C setup.
 * Everything input with pull-up except:
 */
#define VAL_GPIOCCRL            0x88888888      /*  PC7...PC0 */
#define VAL_GPIOCCRH            0x88888888      /* PC15...PC8 */
#define VAL_GPIOCODR            0xFFFFFFFF

/*
 * Port D setup.
 * Everything input with pull-up except:
 * PD0  - Normal input (OSC_IN).
 * PD1  - Normal input (OSC_OUT).
 */
#define VAL_GPIODCRL            0x88888844      /*  PD7...PD0 */
#define VAL_GPIODCRH            0x88888888      /* PD15...PD8 */
#define VAL_GPIODODR            0xFFFFFFFF

/*
 * Port E setup.
 * Everything input with pull-up except:
 */
#define VAL_GPIOECRL            0x88888888      /*  PE7...PE0 */
#define VAL_GPIOECRH            0x88888888      /* PE15...PE8 */
#define VAL_GPIOEODR            0xFFFFFFFF

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
