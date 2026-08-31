TARGET   := firmware
LDSCRIPT := link.ld
BUILD    := build
SRC := src
SRCS     := $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/*.S)
SRCS_RAW  := $(SRCS:src/%=%)

CROSS   := arm-none-eabi-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
SIZE    := $(CROSS)size

SDK := C:/Users/Vasilis/pico-sdk/src
INCS := -Iinclude \
        -I$(SDK)/rp2350/hardware_structs/include \
        -I$(SDK)/rp2350/hardware_regs/include

CPUFLAGS := -mcpu=cortex-m33 -mthumb -mfloat-abi=soft
CFLAGS   := $(CPUFLAGS) -Og -g3 -Wall -Wextra -ffreestanding \
            -ffunction-sections -fdata-sections $(INCS)
LDFLAGS  := $(CPUFLAGS) -T$(LDSCRIPT) -nostdlib -Wl,--gc-sections \
            -Wl,-Map=$(BUILD)/$(TARGET).map

OBJS := $(addprefix $(BUILD)/,$(addsuffix .o,$(basename $(SRCS_RAW))))
DEPS := $(OBJS:.o=.d)
ELF  := $(BUILD)/$(TARGET).elf
UF2  := $(BUILD)/$(TARGET).uf2
BIN  := $(BUILD)/$(TARGET).bin

print-%:
	@echo '$* = $($*)'

all: $(UF2)

$(BUILD):
	mkdir -p $(BUILD)

$(ELF): $(OBJS) $(LDSCRIPT) | $(BUILD)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(UF2): $(ELF)
	picotool uf2 convert $< $@ --family rp2350-arm-s

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/%.o: $(SRC)/%.S | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

flash: $(UF2)
	picotool load -x $< -f

clean:
	rm -rf $(BUILD)

.PHONY: all flash clean
-include $(DEPS)
