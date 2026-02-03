#!/usr/bin/env python3
"""
Regenerate sprite_lookup.c with CORRECT grit symbol names
Grit converts: 0001.png -> _001, 0015.png -> _015, 1017.png -> _017
"""

import json
import os

# Load Pokemon database
with open('pokedex.json', 'r', encoding='utf-8') as f:
    pokedex = json.load(f)

def get_grit_symbol(dex_num, form=''):
    """
    Convert Pokemon number to grit symbol name
    Grit strips leading digits to make 3-digit symbols:
    - 0001 -> _001
    - 0015 -> _015  
    - 1017 -> _017
    """
    # Take last 3 digits
    symbol_num = str(dex_num).zfill(4)[-3:]
    return f"_{symbol_num}{form}"

# Generate sprite_lookup.c
output = []
output.append("// Auto-generated sprite lookup table")
output.append("// Maps Pokemon ID -> sprite data pointers")
output.append("")
output.append('#include "sprite_manager.h"')
output.append("")

# Extern declarations
output.append("// Sprite data declarations")
for entry in pokedex:
    dex_num = entry['id']
    forms = entry.get('forms', [''])
    
    for form in forms:
        symbol = get_grit_symbol(dex_num, form)
        output.append(f"extern const unsigned int {symbol}Tiles[];")
        output.append(f"extern const unsigned short {symbol}Pal[];")
        output.append(f"extern const unsigned int {symbol}_shinyTiles[];")
        output.append(f"extern const unsigned short {symbol}_shinyPal[];")

output.append("")
output.append("// Sprite lookup table")
output.append("const SpriteEntry SPRITE_DATABASE[] = {")

for entry in pokedex:
    dex_num = entry['id']
    forms = entry.get('forms', [''])
    
    for form in forms:
        symbol = get_grit_symbol(dex_num, form)
        output.append(f"    {{{dex_num}, \"{form}\", 0, {symbol}Tiles, {symbol}Pal}},")

output.append("};")
output.append("")
output.append(f"const int TOTAL_SPRITES = {len(pokedex) * 2};  // Normal + shiny")

# Write file
with open('source/sprite_lookup.c', 'w') as f:
    f.write('\n'.join(output))

print(f"✓ Generated sprite_lookup.c with {len(pokedex)} Pokemon")
print(f"  Using grit symbol format: 0001->_001, 0015->_015, 1017->_017")
