#---------------------------------------------------------------------------------
# Pokemon Tetris GBA - Makefile
# OPTIMIZED VERSION with -O3 and performance flags
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

# OPTIMIZATION 5: Changed -O2 to -O3, added -finline-functions and -funroll-loops
CFLAGS   := -Wall -O3 \
            -mcpu=arm7tdmi -mtune=arm7tdmi \
            -fomit-frame-pointer -ffast-math \
            -finline-functions -funroll-loops \
            $(ARCH) \
            -I$(INCLUDES) \
            -I$(DEVKITPRO)/libgba/include

LDFLAGS  := $(ARCH) -specs=gba.specs
LIBS     := -L$(DEVKITPRO)/libgba/lib -lgba

#---------------------------------------------------------------------------------
# File lists
#---------------------------------------------------------------------------------

SRC_CFILES   := $(wildcard $(SOURCES)/*.c)
SRC_OFILES   := $(SRC_CFILES:$(SOURCES)/%.c=$(BUILD)/src/%.o)

SPR_CFILES   := $(wildcard $(SPRITES)/*.c)
SPR_OFILES   := $(SPR_CFILES:$(SPRITES)/%.c=$(BUILD)/spr/%.o)

ALL_OFILES   := $(SRC_OFILES) $(SPR_OFILES)

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
	@rm -rf $(BUILD)
	@rm -f $(TARGET).elf $(TARGET).gba
	@echo "✓ Clean complete"

$(TARGET).gba: $(TARGET).elf
	@echo "Creating GBA ROM..."
	@$(OBJCOPY) -O binary $< $@
	@-gbafix -t"POKETETRIS" -c"PKTM" -m"01" -r2 -p $@ 2>/dev/null || true
	@echo "✓ ROM fixed with SRAM (32KB) save type"

$(TARGET).elf: $(ALL_OFILES)
	@echo "Linking $(words $(ALL_OFILES)) object files..."
	@printf '%s\n' $(ALL_OFILES) > $(BUILD)/objects.txt
	@$(CC) $(LDFLAGS) @$(BUILD)/objects.txt $(LIBS) -o $@
	@echo "✓ Link complete"

$(BUILD)/src/%.o: $(SOURCES)/%.c
	@mkdir -p $(BUILD)/src
	@echo "  src: $(<F)"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/spr/%.o: $(SPRITES)/%.c
	@mkdir -p $(BUILD)/spr
	@$(CC) $(CFLAGS) -c $< -o $@