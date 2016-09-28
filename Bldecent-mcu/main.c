// Copyright (C) 2016 SlamBuilder LLC. All rights reserved.
#include "sam.h"
#include "board/pins.h"

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();

	board_init();

	PORT->Group[BOARD_PINS_DAC_PORT].DIRSET.reg	= 1 << BOARD_PINS_DAC_PIN;
	// PORT->Group[BOARD_PINS_DAC_PORT].PINCFG[PIN_PA02B_DAC_VOUT]

    /* Replace with your application code */
    while (1) 
    {
		PORT->Group[BOARD_PINS_DAC_PORT].OUTTGL.reg = 1 << BOARD_PINS_DAC_PIN;
    }
}
