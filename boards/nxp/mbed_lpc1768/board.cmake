board_runner_args(pyocd "--target=lpc1768")
board_runner_args(pyocd "--tool-opt=-O reset_type=hw")

# ROM code requires the correct CPU clock speed for flash erase/write operations
math(EXPR CCLK "${CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC} / 1000")
board_runner_args(openocd "--cmd-pre-init=set CCLK ${CCLK}")

# Increase WORKAREASIZE to use all of SRAM
dt_reg_size(WORKAREASIZE PATH "/memory@10000000")
board_runner_args(openocd "--cmd-pre-init=set WORKAREASIZE ${WORKAREASIZE}")

include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
