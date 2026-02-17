#---------------------------------------------------------------------------------
# Pokemon Tetris GBA - Makefile
#---------------------------------------------------------------------------------

TARGET   := pokemon_tetris
BUILD    := build
SOURCES  := source
INCLUDES := include
SPRITES  := gfx/sprites
AUDIO    := audio_data

PREFIX   := arm-none-eabi-
CC       := $(PREFIX)gcc
AS       := $(PREFIX)as
OBJCOPY  := $(PREFIX)objcopy

ARCH     := -mthumb -mthumb-interwork

CFLAGS   := -Wall -O3 \
            -mcpu=arm7tdmi -mtune=arm7tdmi \
            -fomit-frame-pointer -ffast-math \
            -finline-functions -funroll-loops \
            $(ARCH) \
            -I$(INCLUDES) \
            -I$(DEVKITPRO)/libgba/include

ASFLAGS  := -mcpu=arm7tdmi -mthumb-interwork
LDFLAGS  := $(ARCH) -specs=gba.specs
LIBS     := -L$(DEVKITPRO)/libgba/lib -lgba

SRC_CFILES   := $(wildcard $(SOURCES)/*.c)
SRC_OFILES   := $(SRC_CFILES:$(SOURCES)/%.c=$(BUILD)/src/%.o)

SPR_CFILES   := $(wildcard $(SPRITES)/*.c)
SPR_OFILES   := $(SPR_CFILES:$(SPRITES)/%.c=$(BUILD)/spr/%.o)

AUDIO_SFILES := $(wildcard $(AUDIO)/*.s)
AUDIO_OFILES := $(AUDIO_SFILES:$(AUDIO)/%.s=$(BUILD)/audio/%.o)

ALL_OFILES   := $(SRC_OFILES) $(SPR_OFILES) $(AUDIO_OFILES)

.PHONY: all clean

all: $(TARGET).gba
	@echo ""
	@echo "✓ Build complete!"
	@ls -lh $(TARGET).gba

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD)
	@rm -f $(TARGET).elf $(TARGET).gba
	@echo "✓ Clean complete"

$(TARGET).gba: $(TARGET).elf
	@$(OBJCOPY) -O binary $< $@
	@-gbafix -t"POKETETRIS" -c"PKTM" -m"01" -r2 -p $@ 2>/dev/null || true
	@echo "✓ ROM fixed"

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

$(BUILD)/audio/%.o: $(AUDIO)/%.s
	@mkdir -p $(BUILD)/audio
	@$(AS) $(ASFLAGS) $< -o $@
