target = payload
entry = 0x20000a90

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
CPY = arm-none-eabi-objcopy

SRCS = $(shell find . -maxdepth 1 -name '*.c')
OBJS  = $(patsubst %.c,%.o,$(SRCS))
INCDIR = /home/ladmanj/STM32CubeIDE/workspace_1.12.0/modbule-0.32/Core/Inc

CFLAGS = -mcpu=cortex-m0plus -std=gnu11 --specs=nano.specs -mfloat-abi=soft -mthumb -Os -fpie -nostdlib -nostartfiles --entry=$(entry) -I$(INCDIR)
LDFLAGS = -Wl,-nostdlib,-pie,-Ttext=$(entry)

all: $(target).bin

$(target).bin: $(target).elf
	$(CPY) -O binary --only-section=.text $< $@

$(target).elf: $(OBJS)
	$(LD) $(CFLAGS) $(LDFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY clean:
	rm -f $(OBJS) $(target).elf $(target).bin
