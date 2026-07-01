#ifndef _SOC_H_
#define _SOC_H_

#ifndef _ASMLANGUAGE

#if 0
#include <zephyr/arch/arm/cortex_m/nvic.h>

typedef enum {
	// Cortex-M3 common exceptions
	Reset_IRQn = -15,
	NonMaskableInt_IRQn = -14,
	HardFault_IRQn = -13,
	MemoryManagement_IRQn = -12,
	BusFault_IRQn = -11,
	UsageFault_IRQn = -10,
	SVCall_IRQn = -5,
	DebugMonitor_IRQn = -4,
	PendSV_IRQn = -2,
	SysTick_IRQn = -1,

	// IRQn >= 0 are declared by the SoC device tree
	Max_IRQn = CONFIG_NUM_IRQS,
} IRQn_Type;

// LPC17xx Cortex-M3 implementation
#define __CM3_REV 0x0200
#define __MPU_PRESENT CONFIG_CPU_HAS_ARM_MPU
#define __VTOR_PRESENT CONFIG_CPU_CORTEX_M_HAS_VTOR
#define __NVIC_PRIO_BITS NUM_IRQ_PRIO_BITS
#define __Vendor_SysTickConfig 0U

#include <core_cm3.h>

#else

// Redirect to LPCOpen
#include <chip.h>

#endif

#endif /* !_ASMLANGUAGE */

#endif
