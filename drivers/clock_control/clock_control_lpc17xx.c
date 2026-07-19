/* LPC17xx clock control driver */

#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <zephyr/drivers/clock_control/lpc17xx_clock_control.h>

/* Symbols referenced by LPCOpen */
const uint32_t OscRateIn = LPC17XX_OSC_CLK_FREQ;
const uint32_t RTCOscRateIn = LPC17XX_RTC_CLK_FREQ;

const struct device *const IRC_CLK = DEVICE_DT_GET_OR_NULL(LPC17XX_IRC_OSC);
const struct device *const OSC_CLK = DEVICE_DT_GET_OR_NULL(LPC17XX_OSC_CLK);
const struct device *const RTC_CLK = DEVICE_DT_GET_OR_NULL(LPC17XX_RTC_CLK);

const struct device *const MAIN_PLL = DEVICE_DT_GET_OR_NULL(LPC17XX_MAIN_PLL);
const struct device *const USB_PLL = DEVICE_DT_GET_OR_NULL(LPC17XX_USB_PLL);

static void lpc17xx_clock_set_mux_sysclk(const struct device *clk)
{
	if (clk == NULL) {
		return;
	}

	if (clk == OSC_CLK) {
		Chip_Clock_SetMainPLLSource(SYSCTL_PLLCLKSRC_MAINOSC);
	} else if (clk == RTC_CLK) {
		Chip_Clock_SetMainPLLSource(SYSCTL_PLLCLKSRC_RTC);
	} else {
		/* Should always be default */
		Chip_Clock_SetMainPLLSource(SYSCTL_PLLCLKSRC_IRC);
	}
}

static void lpc17xx_clock_set_mux_cpuclk(const struct device *clk)
{
	if (clk == NULL) {
		return;
	}

	if (clk == MAIN_PLL) {
		Chip_Clock_SetCPUClockSource(SYSCTL_CCLKSRC_MAINPLL);
	} else {
		Chip_Clock_SetCPUClockSource(SYSCTL_CCLKSRC_SYSCLK);
	}
}

static void lpc17xx_clock_set_mux_usbclk(const struct device *clk)
{
	if (clk == NULL) {
		return;
	}

#ifndef CHIP_LPC175X_6X
	if (clk == USB_PLL) {
		Chip_Clock_SetUSBClockSource(SYSCTL_USBCLKSRC_USBPLL);
	} else if (clk == MAIN_PLL) {
		Chip_Clock_SetUSBClockSource(SYSCTL_USBCLKSRC_MAINPLL);
	} else {
		Chip_Clock_SetUSBClockSource(SYSCTL_USBCLKSRC_SYSCLK);
	}
#else
	/* No upper layer macro for USB clock source like CPU clock */
	if (clk == USB_PLL) {
		Chip_Clock_EnablePLL(SYSCTL_USB_PLL, SYSCTL_PLL_CONNECT);
	} else {
		Chip_Clock_DisablePLL(SYSCTL_USB_PLL, SYSCTL_PLL_CONNECT);
	}
#endif
}

static void lpc17xx_clock_set_flash_access_time(uint32_t cpuclk)
{
	const uint32_t LPC17XX_FLASH_ACCESS_RATE = MHZ(20);
	FMC_FLASHTIM_T clocks = (FMC_FLASHTIM_T)(cpuclk / LPC17XX_FLASH_ACCESS_RATE);
	Chip_SYSCTL_SetFLASHAccess(clocks);
}

struct lpc17xx_pll_config {
	CHIP_SYSCTL_PLL_T pll;
	const struct device *src;
	uint32_t mul;
	uint32_t div;
};

static int lpc17xx_pll_on(const struct device *dev, clock_control_subsys_t sys)
{
	const struct lpc17xx_pll_config *config = dev->config;

	Chip_Clock_EnablePLL(config->pll, SYSCTL_PLL_ENABLE);

	// TODO: use WAIT_FOR with a time limit?
	if (config->pll == SYSCTL_USB_PLL) {
		while (!Chip_Clock_IsUSBPLLLocked());
	} else {
		while (!Chip_Clock_IsMainPLLLocked());
	}

	return 0;
}

static int lpc17xx_pll_off(const struct device *dev, clock_control_subsys_t sys)
{
	const struct lpc17xx_pll_config *config = dev->config;

	Chip_Clock_DisablePLL(config->pll, SYSCTL_PLL_ENABLE);
	return 0;
}

static enum clock_control_status lpc17xx_pll_get_status(const struct device *dev, clock_control_subsys_t sys)
{
	const struct lpc17xx_pll_config *config = dev->config;

	bool enabled = (config->pll == SYSCTL_USB_PLL) ?
		Chip_Clock_IsUSBPLLEnabled() :
		Chip_Clock_IsMainPLLEnabled();
	bool locked = (config->pll == SYSCTL_USB_PLL) ?
		Chip_Clock_IsUSBPLLLocked() :
		Chip_Clock_IsMainPLLLocked();

	enum clock_control_status status = CLOCK_CONTROL_STATUS_UNKNOWN;
	if (locked) status = CLOCK_CONTROL_STATUS_ON;
	else if (enabled) status = CLOCK_CONTROL_STATUS_STARTING;
	else status = CLOCK_CONTROL_STATUS_OFF;

	return status;
}

static int lpc17xx_pll_get_rate(const struct device *dev, clock_control_subsys_t sys, uint32_t *rate)
{
	const struct lpc17xx_pll_config *config = dev->config;

	*rate = (config->pll == SYSCTL_USB_PLL) ?
		Chip_Clock_GetUSBPLLOutClockRate() :
		Chip_Clock_GetMainPLLOutClockRate();

	return 0;
}

static DEVICE_API(clock_control, lpc17xx_pll_api) = {
	.on = lpc17xx_pll_on,
	.off = lpc17xx_pll_off,
	.get_status = lpc17xx_pll_get_status,
	.get_rate = lpc17xx_pll_get_rate,
};

static int __unused lpc17xx_pll_init(const struct device *dev)
{
	const struct lpc17xx_pll_config *config = dev->config;

	/* Enable source clock */
	int ret = clock_control_on(config->src, CLOCK_CONTROL_SUBSYS_ALL);
	if (ret) {
		return ret;
	}

	/* Set sysclk mux if we are main PLL */
	if (config->pll == SYSCTL_MAIN_PLL) {
		lpc17xx_clock_set_mux_sysclk(config->src);
	}

	// TODO: support runtime configure with mul and div
	uint32_t mul = config->mul - 1;
	uint32_t div = (config->pll == SYSCTL_USB_PLL) ?
		LOG2(config->div) : config->div - 1;
	Chip_Clock_SetupPLL(config->pll, mul, div);

	return 0;
}

#if LPC17XX_MAIN_PLL_ENABLED
const struct lpc17xx_pll_config main_pll_config = {
	.pll = SYSCTL_MAIN_PLL,
	.src = DEVICE_DT_GET(DT_CLOCKS_CTLR(LPC17XX_MAIN_PLL)),
	.mul = DT_PROP(LPC17XX_MAIN_PLL, mul_m),
	.div = DT_PROP(LPC17XX_MAIN_PLL, div_n),
};

DEVICE_DT_DEFINE(LPC17XX_MAIN_PLL, lpc17xx_pll_init, NULL,
		 NULL, &main_pll_config,
		 PRE_KERNEL_1, CONFIG_CLOCK_CONTROL_INIT_PRIORITY,
		 &lpc17xx_pll_api);
#endif

#if LPC17XX_USB_PLL_ENABLED
const struct lpc17xx_pll_config usb_pll_config = {
	.pll = SYSCTL_USB_PLL,
	.src = DEVICE_DT_GET(DT_CLOCKS_CTLR(LPC17XX_USB_PLL)),
	.mul = DT_PROP(LPC17XX_USB_PLL, mul_m),
	.div = DT_PROP(LPC17XX_USB_PLL, div_p),
};

DEVICE_DT_DEFINE(LPC17XX_USB_PLL, lpc17xx_pll_init, NULL,
		 NULL, &usb_pll_config,
		 PRE_KERNEL_1, CONFIG_CLOCK_CONTROL_INIT_PRIORITY,
		 &lpc17xx_pll_api);
#endif

struct lpc17xx_clkc_config {
	const struct device *src;
	uint32_t div_cpu;
	uint32_t div_usb;
};

static int lpc17xx_clkc_init(const struct device *dev)
{
	const struct lpc17xx_clkc_config *config = dev->config;
	int ret;

	/* Enable source clock */
	ret = clock_control_on(config->src, CLOCK_CONTROL_SUBSYS_ALL);
	if (ret) {
		return ret;
	}

	if (config->src != MAIN_PLL) {
		/* We are responsible for setting up sysclk */
		lpc17xx_clock_set_mux_sysclk(config->src);
	}

	Chip_Clock_SetUSBClockDiv(config->div_usb - 1);
	Chip_Clock_SetCPUClockDiv(config->div_cpu - 1);

	/*
	 * We treat CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC as the max CPU clock, and
	 * we always initialize the CPU clock to the max, so update flash timing to
	 * match.
	 */
	lpc17xx_clock_set_flash_access_time(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC);

	/* Set CPU clock mux according to source clock */
	lpc17xx_clock_set_mux_cpuclk(config->src);

	SystemCoreClockUpdate();
	return 0;
}

const struct lpc17xx_clkc_config clkc_config = {
	.src = DEVICE_DT_GET(DT_CLOCKS_CTLR(LPC17XX_CLOCK_CONTROL)),
	.div_cpu = DT_PROP(LPC17XX_CLOCK_CONTROL, div_cpu),
	.div_usb = DT_PROP(LPC17XX_CLOCK_CONTROL, div_usb),
};

DEVICE_DT_DEFINE(LPC17XX_CLOCK_CONTROL, lpc17xx_clkc_init, NULL,
		 NULL, &clkc_config,
		 PRE_KERNEL_1, CONFIG_CLOCK_CONTROL_INIT_PRIORITY,
		 NULL);
