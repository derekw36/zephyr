#include <zephyr/drivers/clock_control/lpc17xx_clock_control.h>

#include "soc.h"

void soc_early_init_hook(void)
{
	/* UM10360 Chapter 4.5.1.1:
	 * When there is no valid user code (determined by the checksum word)
	 * in the user flash or the ISP enable pin (P2.10) is pulled low on
	 * startup, the ISP mode will be entered and the boot code will setup
	 * the PLL with the IRC. Therefore it can not be assumed that the PLL
	 * is disabled when the user opens a debug session to debug the
	 * application code. The user startup code must follow the steps
	 * described in this chapter to disconnect the PLL.
	 */
	if (Chip_Clock_IsMainPLLConnected()) {
		Chip_Clock_DisablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_CONNECT);
	}
	if (Chip_Clock_IsMainPLLEnabled()) {
		Chip_Clock_DisablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_ENABLE);
	}

	/* Setup the main oscillator before the clock controller and PLLs init */
	if (IS_ENABLED(LPC17XX_OSC_CLK_ENABLED)) {
		/*
		 * Select this for the user based on the osc_clk frequency.
		 * 16 MHz is chosen as the crossover point.
		 */
		if (LPC17XX_OSC_CLK_FREQ >= MHZ(16)) {
			Chip_Clock_SetCrystalRangeHi();
		} else {
			Chip_Clock_SetCrystalRangeLo();
		}

		/* Enable main oscillator */
		Chip_Clock_EnableCrystal();
		while (!Chip_Clock_IsCrystalEnabled());
	}
}
