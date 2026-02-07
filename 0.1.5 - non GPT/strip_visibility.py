#!/usr/bin/env python3
"""
strip_visibility.py
-------------------
GRIT (v0.9.2) emits:

    const unsigned int p0001Tiles[…]
        __attribute__((aligned(4)))
        __attribute__((visibility("hidden")))  // <-- THIS kills the linker

visibility("hidden") stops the symbol from being exported out of the .o file,
so sprite_lookup.c can never link to it even though the extern is correct.

This script removes ONLY the visibility attribute, keeps aligned(4), and
leaves everything else untouched.

Usage
-----
    python3 strip_visibility.py                 # default: gfx/sprites/
    python3 strip_visibility.py path/to/sprites # custom directory

Run it once after every `grit` batch.  It is idempotent — safe to run again.
"""

import os, re, sys, glob

# Matches the two-attribute combo GRIT produces (with optional whitespace / newlines between)
PATTERN = re.compile(
    r'(__attribute__\(\(aligned\(4\)\)\))'   # keep this
    r'\s*'                                   # optional whitespace / newline
    r'__attribute__\(\(visibility\("hidden"\)\)\)'  # drop this
)

def main():
    sprite_dir = sys.argv[1] if len(sys.argv) > 1 else "gfx/sprites"

    if not os.path.isdir(sprite_dir):
        print(f"ERROR: directory not found: {sprite_dir}")
        sys.exit(1)

    files   = sorted(glob.glob(os.path.join(sprite_dir, "*.c")))
    changed = 0

    for path in files:
        with open(path, "r") as f:
            text = f.read()

        new_text = PATTERN.sub(r'\1', text)   # keep group 1 (aligned), drop the rest

        if new_text != text:
            with open(path, "w") as f:
                f.write(new_text)
            changed += 1

    print(f"Processed {len(files)} files in '{sprite_dir}' — {changed} updated.")

if __name__ == "__main__":
    main()
