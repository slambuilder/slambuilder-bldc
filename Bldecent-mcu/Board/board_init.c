// Copyright (C) 2016 SlamBuilder LLC. All rights reserved.

#include "sam.h"
#include "pins.h"
#include "definitions.h"

// Constants for Clock generators
#define GENERIC_CLOCK_GENERATOR_MAIN_ID     (0u)
#define GENERIC_CLOCK_GENERATOR_XOSC32K_ID  (1u)
#define GENERIC_CLOCK_GENERATOR_OSCULP32K	(2u) /* Initialized at reset for WDT */
#define GENERIC_CLOCK_GENERATOR_OSC8M		(3u)
// Constants for Clock multiplexers
#define GENERIC_CLOCK_MULTIPLEXER_DFLL48M_ID (0u)

// Frequency of the main board oscillator 
#define BOARD_XOSC32K_FREQUENCY				(32768ul)

// Master clock frequency
#define BOARD_MASTER_CLOCK_FREQUENCY		CPU_FREQUENCY

void board_init(void)
{
	// NVM Read Wait States = 1
	// this is needed for flash to work at 48 MHz
	NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val;

	// Turn on the digital interface clock
	PM->APBAMASK.reg |= PM_APBAMASK_GCLK;

	// Enable XOSC32K clock
	SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_STARTUP( 0x5u ) // startup time 1,000,092 us
		| SYSCTRL_XOSC32K_XTALEN 
		| SYSCTRL_XOSC32K_EN32K;
	SYSCTRL->XOSC32K.bit.ENABLE = 1;

	// wait for XOSC32K clock to start
	while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY) == 0 ) 
	{
	}

	// Software reset the module to ensure it is re-initialized correctly 
	// Note: Due to synchronization, there is a delay from writing CTRL.SWRST until the reset is complete.
	// CTRL.SWRST and STATUS.SYNCBUSY will both be cleared when the reset is complete, as described in chapter 15.8.1
	GCLK->CTRL.reg = GCLK_CTRL_SWRST;

	// Wait for GCLK reset to complete
	while ( (GCLK->CTRL.reg & GCLK_CTRL_SWRST) && (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) )
	{
	}

	// set division factor for Generic Clock Generator 1 as no division
	GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_XOSC32K_ID ); // Generic Clock Generator 1

	// Wait for synchronization 
	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
	}

	// Put XOSC32K as source of Generic Clock Generator 1
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_XOSC32K_ID )	// Generic Clock Generator 1
		| GCLK_GENCTRL_SRC_XOSC32K	// Selected source is External 32KHz Oscillator
		| GCLK_GENCTRL_GENEN;

	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
	}

	// Put Generic Clock Generator 1 as source for Generic Clock Multiplexer 0 (DFLL48M reference)
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GENERIC_CLOCK_MULTIPLEXER_DFLL48M_ID ) 
		| GCLK_CLKCTRL_GEN_GCLK1 // Generic Clock Generator 1 is source
		| GCLK_CLKCTRL_CLKEN;

	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
	}

	// Enable DFLL48M clock

	// DFLL Configuration in Closed Loop mode
	// See http://atmel.force.com/support/articles/en_US/FAQ/How-to-configure-DFLL48M-oscillator-as-main-clock-in-ASF-project-for-SAMR21-SAMD21-SAMD20-SAMD10-SAMD11

	// Remove the OnDemand mode, Bug http://avr32.icgroup.norway.atmel.com/bugzilla/show_bug.cgi?id=9905
	SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;

	while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
	{
	}

	SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP( 7 )	// coarse step
		| SYSCTRL_DFLLMUL_FSTEP( 63 )					// fine step
		| SYSCTRL_DFLLMUL_MUL( (BOARD_MASTER_CLOCK_FREQUENCY/BOARD_XOSC32K_FREQUENCY) ); // External 32KHz is the reference

	while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
	{
	}

	// Write full configuration to DFLL control register 
	SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_MODE // Enable the closed loop mode
		| SYSCTRL_DFLLCTRL_WAITLOCK
		| SYSCTRL_DFLLCTRL_QLDIS; // Disable Quick lock 

	while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
	{
	}

	// Enable the DFLL 
	SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE;

	// Wait for locks flags
	while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0 || (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0 )
	{
	}

	// Wait for synchronization
	while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
	{
	}

	// Switch Generic Clock Generator 0 to DFLL48M
	GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_MAIN_ID );

	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
	}

	/* Write Generic Clock Generator 0 configuration */
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_MAIN_ID ) 
		| GCLK_GENCTRL_SRC_DFLL48M // Selected source is DFLL 48MHz
		| GCLK_GENCTRL_IDC // Set 50/50 duty cycle
		| GCLK_GENCTRL_GENEN;

	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
	}

	// Now that all system clocks are configured, we can set CPU and APBx BUS clocks.
	// These values are normally the ones present after Reset.
	PM->CPUSEL.reg  = PM_CPUSEL_CPUDIV_DIV1;
	PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1_Val;
	PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1_Val;
	PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1_Val;
}
