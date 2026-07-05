# pyocd works great for flashing and uses an incremental algorithm which is
# faster and results in less writes than openocd. but pyocd fails to stay
# connected during debugging, whereas openocd works great. so separate the
# duties here.
board_set_flasher_ifnset(pyocd)
board_set_debugger_ifnset(openocd)

# Increase WORKAREASIZE to use all of SRAM
dt_reg_size(WORKAREASIZE PATH "/memory@10000000")
board_runner_args(openocd --cmd-pre-init "set WORKAREASIZE ${WORKAREASIZE}")
board_runner_args(openocd --cmd-pre-init "adapter speed 12000")
board_runner_args(openocd --config "board/mbed-lpc1768.cfg")
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)

board_runner_args(pyocd "--target=lpc1768")
board_runner_args(pyocd "--frequency=12000000")
include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
