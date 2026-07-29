/* Public header for the LPC17xx clock controller driver */

#ifndef ZEPHYR_INCLUDE_DRIVERS_CLOCK_CONTROL_LPC17XX_CLOCK_CONTROL_H_
#define ZEPHYR_INCLUDE_DRIVERS_CLOCK_CONTROL_LPC17XX_CLOCK_CONTROL_H_

#include <zephyr/devicetree.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/dt-bindings/clock/lpc17xx_clock.h>

#include <soc.h>

/* Fixed clocks */
#define LPC17XX_IRC_OSC DT_NODELABEL(irc_osc)
#define LPC17XX_IRC_OSC_ENABLED DT_NODE_HAS_STATUS_OKAY(LPC17XX_IRC_OSC)
#define LPC17XX_IRC_OSC_FREQ DT_PROP_OR(LPC17XX_IRC_OSC, clock_frequency, 0)
#define LPC17XX_OSC_CLK DT_NODELABEL(osc_clk)
#define LPC17XX_OSC_CLK_ENABLED DT_NODE_HAS_STATUS_OKAY(LPC17XX_OSC_CLK)
#define LPC17XX_OSC_CLK_FREQ DT_PROP_OR(LPC17XX_OSC_CLK, clock_frequency, 0)
#define LPC17XX_RTC_CLK DT_NODELABEL(rtc_clk)
#define LPC17XX_RTC_CLK_ENABLED DT_NODE_HAS_STATUS_OKAY(LPC17XX_RTC_CLK)
#define LPC17XX_RTC_CLK_FREQ DT_PROP_OR(LPC17XX_RTC_CLK, clock_frequency, 0)

/* PLLs */
#define LPC17XX_MAIN_PLL DT_INST(0, nxp_lpc17xx_main_pll)
#define LPC17XX_MAIN_PLL_ENABLED DT_NODE_HAS_STATUS_OKAY(LPC17XX_MAIN_PLL)
#define LPC17XX_USB_PLL DT_INST(0, nxp_lpc17xx_usb_pll)
#define LPC17XX_USB_PLL_ENABLED DT_NODE_HAS_STATUS_OKAY(LPC17XX_USB_PLL)

/* Clock controller */
#define LPC17XX_CLOCK_CONTROL DT_INST(0, nxp_lpc17xx_clock)

/* Peripheral clock helpers */
#define LPC17XX_PCLK_GET_ENABLE(node) (CHIP_SYSCTL_CLOCK_T)DT_CLOCKS_CELL(node, enable)
#define LPC17XX_PCLK_GET_SELECT(node) (CHIP_SYSCTL_PCLK_T)DT_CLOCKS_CELL(node, select)
#define LPC17XX_PCLK_GET_DIV(node) (CHIP_SYSCTL_CLKDIV_T)DT_CLOCKS_CELL(node, div)

struct lpc17xx_pclk {
	CHIP_SYSCTL_CLOCK_T enable;
	CHIP_SYSCTL_PCLK_T select;
};

#define LPC17XX_CLOCK_INFO(node) \
	{ \
		.enable = LPC17XX_PCLK_GET_ENABLE(node), \
		.select = LPC17XX_PCLK_GET_SELECT(node), \
	}

#endif /* ZEPHYR_INCLUDE_DRIVERS_CLOCK_CONTROL_LPC17XX_CLOCK_CONTROL_H_ */
