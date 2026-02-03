#---------------------------------------------------------------------------------
# Pokemon Tetris GBA - Simple Makefile
#---------------------------------------------------------------------------------

# Project name
TARGET := pokemon_tetris

# Directories
BUILD := build
SOURCES := source
INCLUDES := include

# Toolchain
PREFIX := arm-none-eabi-
CC := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
AR := $(PREFIX)ar

# Compiler flags
ARCH := -mthumb -mthumb-interwork
CFLAGS := -g -Wall -O2 \
          -mcpu=arm7tdmi -mtune=arm7tdmi \
          -fomit-frame-pointer -ffast-math \
          $(ARCH) \
          -I$(INCLUDES) \
          -I$(DEVKITPRO)/libgba/include

# Linker flags
LDFLAGS := $(ARCH) -specs=gba.specs
LIBS := -L$(DEVKITPRO)/libgba/lib -lgba

# Source files
CFILES := $(wildcard $(SOURCES)/*.c)
OFILES := $(CFILES:$(SOURCES)/%.c=$(BUILD)/%.o)

#---------------------------------------------------------------------------------
# Targets
#---------------------------------------------------------------------------------

.PHONY: all clean

all: $(TARGET).gba
	@echo "Build complete! ROM: $(TARGET).gba"

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD)
	@rm -f $(TARGET).elf $(TARGET).gba $(TARGET).map
	@echo "Clean complete!"

# Create build directory
$(BUILD):
	@mkdir -p $(BUILD)

# Create GBA ROM from ELF
$(TARGET).gba: $(TARGET).elf
	@echo "Creating GBA ROM..."
	@$(OBJCOPY) -O binary $< $@
	@-gbafix $@ 2>/dev/null || true
	@ls -lh $(TARGET).gba

# Link ELF executable
$(TARGET).elf: $(OFILES) | $(BUILD)
	@echo "Linking..."
	@$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

# Compile C files
$(BUILD)/%.o: $(SOURCES)/%.c | $(BUILD)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# Dependencies (optional but helpful)
-include $(OFILES:.o=.d)

$(BUILD)/%.d: $(SOURCES)/%.c | $(BUILD)
	@$(CC) -MM $(CFLAGS) $< > $@
