#!/usr/bin/env python3
"""
NXP UM10360 LPC17xx User Manual

32.3.1.1 Criterion for Valid User Code
The reserved Cortex-M3 exception vector location 7 (offset 0x1C in the vector table)
should contain the 2's complement of the check-sum of table entries 0 through 6. This
causes the checksum of the first 8 table entries to be 0. The boot loader code checksums
the first 8 locations in sector 0 of the flash. If the result is 0, then execution control is
transferred to the user code.
"""

import argparse
from typing import BinaryIO
import struct

from elftools.elf.elffile import ELFFile

LPC_CHECKSUM_OFFSET = 0x1C

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("elf_file", help="Path to Zephyr kernel ELF image")
    return parser.parse_args()

def get_rom_start_offset(elf_fp: BinaryIO) -> int:
    elf = ELFFile(elf_fp)
    rom_start = elf.get_section_by_name("rom_start")
    return rom_start["sh_offset"]

def patch_checksum(elf_fp: BinaryIO, rom_start_offset: int) -> None:
    elf_fp.seek(rom_start_offset)
    vectors = elf_fp.read(LPC_CHECKSUM_OFFSET)
    vectors: tuple[int] = struct.unpack("<IIIIIII", vectors)
    checksum = (~sum(vectors) + 1) & 0xFFFFFFFF
    checksum = struct.pack("<I", checksum)
    elf_fp.write(checksum)

def main():
    args = parse_args()
    with open(args.elf_file, "r+b") as elf_fp:
        rom_start_offset = get_rom_start_offset(elf_fp)
        patch_checksum(elf_fp, rom_start_offset)

if __name__ == "__main__":
    main()
