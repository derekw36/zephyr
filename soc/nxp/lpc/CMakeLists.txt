#
# Copyright (c) 2017-2022 NXP
#
# SPDX-License-Identifier: Apache-2.0
#

add_subdirectory(${SOC_SERIES})

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)

if (CONFIG_LPC_PATCH_CHECKSUM)
  set_property(GLOBAL APPEND PROPERTY post_build_patch_elf_commands
    COMMAND ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_LIST_DIR}/lpc_checksum.py
    ${PROJECT_BINARY_DIR}/${KERNEL_ELF_NAME}
  )
endif()
