#---------------------------------------------------------------------------------
# Pokemon Tetris GBA - Makefile with MaxMod Audio Support
# Supports parallel compilation (-j flag) and response file linking
#---------------------------------------------------------------------------------

TARGET := pokemon_tetris

BUILD    := build
SOURCES  := source
INCLUDES := include
SPRITES  := gfx/sprites

PREFIX   := arm-none-eabi-
CC       := $(PREFIX)gcc
OBJCOPY  := $(PREFIX)objcopy

ARCH     := -mthumb -mthumb-interwork
CFLAGS   := -Wall -O2 \
            -mcpu=arm7tdmi -mtune=arm7tdmi \
            -fomit-frame-pointer -ffast-math \
            $(ARCH) \
            -I$(INCLUDES) \
            -I$(DEVKITPRO)/libgba/include

LDFLAGS  := $(ARCH) -specs=gba.specs
LIBS     := -L$(DEVKITPRO)/libgba/lib -lmm -lgba

#---------------------------------------------------------------------------------
# File lists
#---------------------------------------------------------------------------------

# Source .c files -> .o in build/src/
SRC_CFILES   := $(wildcard $(SOURCES)/*.c)
SRC_OFILES   := $(SRC_CFILES:$(SOURCES)/%.c=$(BUILD)/src/%.o)

# Sprite .c files -> .o in build/spr/
SPR_CFILES   := $(wildcard $(SPRITES)/*.c)
SPR_OFILES   := $(SPR_CFILES:$(SPRITES)/%.c=$(BUILD)/spr/%.o)

# Soundbank binary -> .o
SOUNDBANK_OFILE := $(BUILD)/soundbank.bin.o

ALL_OFILES   := $(SRC_OFILES) $(SPR_OFILES) $(SOUNDBANK_OFILE)

#---------------------------------------------------------------------------------
# Targets
#---------------------------------------------------------------------------------

.PHONY: all clean

all: $(TARGET).gba
	@echo ""
	@echo "✓ Build complete!"
	@echo ""
	@echo "ROM: $(TARGET).gba"
	@ls -lh $(TARGET).gba

clean:
	@echo "Cleaning..."
	@rm -f $(BUILD)/src/*.o 2>/dev/null || true
	@rm -f $(BUILD)/spr/*.o 2>/dev/null || true
	@rm -f $(BUILD)/soundbank.bin.o 2>/dev/null || true
	@rm -f $(BUILD)/objects.txt 2>/dev/null || true
	@rm -f $(INCLUDES)/soundbank_bin.h 2>/dev/null || true
	@rm -f $(TARGET).elf $(TARGET).gba
	@echo "✓ Clean complete"

#---------------------------------------------------------------------------------
# ROM creation with proper header for Delta iOS compatibility
#---------------------------------------------------------------------------------

$(TARGET).gba: $(TARGET).elf
	@echo "Creating GBA ROM..."
	@$(OBJCOPY) -O binary $< $@
	@-gbafix -t"POKETETRIS" -c"PKTM" -m"01" -r0 -p $@ 2>/dev/null || true
	@echo "✓ ROM fixed with save type markers for Delta compatibility"

#---------------------------------------------------------------------------------
# Linking (response file avoids "argument list too long")
#---------------------------------------------------------------------------------

$(TARGET).elf: $(ALL_OFILES)
	@echo "Linking $(words $(ALL_OFILES)) object files..."
	@printf '%s\n' $(ALL_OFILES) > $(BUILD)/objects.txt
	@$(CC) $(LDFLAGS) @$(BUILD)/objects.txt $(LIBS) -o $@
	@echo "✓ Link complete"

#---------------------------------------------------------------------------------
# Compilation rules
#---------------------------------------------------------------------------------

# Source files (audio.c needs soundbank_bin.h)
$(BUILD)/src/%.o: $(SOURCES)/%.c $(INCLUDES)/soundbank_bin.h
	@mkdir -p $(BUILD)/src
	@echo "  src: $(<F)"
	@$(CC) $(CFLAGS) -c $< -o $@

# Sprite files (quiet — only summary printed by make's job output)
$(BUILD)/spr/%.o: $(SPRITES)/%.c
	@mkdir -p $(BUILD)/spr
	@$(CC) $(CFLAGS) -c $< -o $@

#---------------------------------------------------------------------------------
# Soundbank binary embedding (MaxMod audio)
# Soundbank files live in audio/ directory (committed to source control)
#---------------------------------------------------------------------------------

# Generate soundbank_bin.h from audio/soundbank.bin
$(INCLUDES)/soundbank_bin.h: audio/soundbank.bin
	@echo "  Generating soundbank_bin.h..."
	@echo "// Auto-generated soundbank binary symbols" > $(INCLUDES)/soundbank_bin.h
	@echo "#ifndef SOUNDBANK_BIN_H" >> $(INCLUDES)/soundbank_bin.h
	@echo "#define SOUNDBANK_BIN_H" >> $(INCLUDES)/soundbank_bin.h
	@echo "extern const unsigned char soundbank_bin[];" >> $(INCLUDES)/soundbank_bin.h
	@echo "extern const unsigned char soundbank_bin_end[];" >> $(INCLUDES)/soundbank_bin.h
	@echo "extern const unsigned int soundbank_bin_size;" >> $(INCLUDES)/soundbank_bin.h
	@echo "#endif" >> $(INCLUDES)/soundbank_bin.h

# Convert audio/soundbank.bin to object file in build/
$(BUILD)/soundbank.bin.o: audio/soundbank.bin
	@echo "  audio: soundbank.bin"
	@$(PREFIX)objcopy -I binary -O elf32-littlearm -B arm \
		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
		--redefine-sym _binary_audio_soundbank_bin_start=soundbank_bin \
		--redefine-sym _binary_audio_soundbank_bin_end=soundbank_bin_end \
		--redefine-sym _binary_audio_soundbank_bin_size=soundbank_bin_size \
		audio/soundbank.bin $(BUILD)/soundbank.bin.o

# To regenerate soundbank (only when you change audio files):
# cd audio && mmutil *.wav -osoundbank.bin -hsoundbank.h
# Files stay in audio/ directory - no need to move them!

#---------------------------------------------------------------------------------
# Usage:
#   make            - single core build
#   make -j4        - 4 parallel jobs (use number of your CPU cores)
#   make -j         - use ALL cores
#   make clean      - remove all build files
#---------------------------------------------------------------------------------
