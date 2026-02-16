#!/usr/bin/env python3
"""
GBA ROM Header Patcher
Manually sets save type to FLASH1M (128KB)
"""

import sys
import struct

def patch_rom(filename):
    """Patch GBA ROM header with correct save type"""
    
    try:
        with open(filename, 'r+b') as f:
            # Read first 192 bytes (ROM header)
            f.seek(0)
            header = bytearray(f.read(192))
            
            # Offset 0xAC = Save type
            # 0x00 = None/EEPROM
            # 0x01 = EEPROM 8KB
            # 0x02 = SRAM 32KB
            # 0x03 = FLASH1M 128KB ← We want this!
            # 0x04 = FLASH 64KB
            
            print(f"Current save type at 0xAC: 0x{header[0xAC]:02X}")
            
            # Set to FLASH1M
            header[0xAC] = 0x03
            
            # Calculate header checksum (0xA0-0xBC)
            checksum = 0
            for i in range(0xA0, 0xBD):
                checksum = (checksum - header[i]) & 0xFF
            header[0xBD] = checksum
            
            print(f"Setting save type to 0x03 (FLASH1M 128KB)")
            print(f"Header checksum: 0x{checksum:02X}")
            
            # Write back
            f.seek(0)
            f.write(header)
            
            print(f"✓ ROM patched successfully!")
            print(f"Save type is now FLASH1M (128KB)")
            
            return True
            
    except FileNotFoundError:
        print(f"ERROR: File '{filename}' not found!")
        return False
    except Exception as e:
        print(f"ERROR: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python patch_rom_header.py <rom_file.gba>")
        sys.exit(1)
    
    rom_file = sys.argv[1]
    
    print(f"Patching: {rom_file}")
    print("-" * 50)
    
    if patch_rom(rom_file):
        sys.exit(0)
    else:
        sys.exit(1)
