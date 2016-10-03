/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

// DAC on PA02
#define BOARD_PINS_DAC_PORT 0
#define BOARD_PINS_DAC_PIN 2

int main (void)
{
	system_init();

	PORT->Group[BOARD_PINS_DAC_PORT].DIRSET.reg	= 1 << BOARD_PINS_DAC_PIN;
	// PORT->Group[BOARD_PINS_DAC_PORT].PINCFG[PIN_PA02B_DAC_VOUT]

	/* Replace with your application code */
	while (1)
	{
		PORT->Group[BOARD_PINS_DAC_PORT].OUTTGL.reg = 1 << BOARD_PINS_DAC_PIN;
	}
}
