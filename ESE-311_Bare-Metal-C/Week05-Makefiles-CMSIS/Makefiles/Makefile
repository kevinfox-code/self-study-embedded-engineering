# Basic STM32 Makefile

# Target
TARGET = Makefile_project

final: $(TARGET).elf

# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# Source files
C_SOURCES = main.c
ASM_SOURCES = startup_stm32u575zitxq.s
LINKER_SCRIPT = STM32U575ZITXQ_FLASH.ld

# Object files
C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.s=.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# Compiler flags
CFLAGS = -mcpu=cortex-m33 -mthumb -c

# Assembler flags
ASFLAGS = -mcpu=cortex-m33 -mthumb

# Linker flags
LDFLAGS = -mcpu=cortex-m33 -mthumb -T$(LINKER_SCRIPT) --specs=nosys.specs

# Build rules
$(TARGET).elf: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

load: $(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32u5x.cfg -c "program $(TARGET).elf verify reset exit"

# Clean
clean:
	rm -f $(OBJECTS) $(TARGET).elf

.PHONY: clean load
