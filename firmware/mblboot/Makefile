target = mblboot

AS = arm-none-eabi-gcc
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
CPY = arm-none-eabi-objcopy

UC = $(shell echo '$1' | tr '[:lower:]' '[:upper:]')


OBJ_DIR_DEBUG   = obj_debug
OBJ_DIR_RELEASE = obj_release

CSRCS  = $(shell find * -maxdepth 7 -name '*.c')
ASRCS  = $(shell find * -maxdepth 7 \( -name '*.S' -o -name '*.s' \) )

OBJS_DEBUG  = $(filter %.o,$(CSRCS:%.c=$(OBJ_DIR_DEBUG)/%.o))
OBJS_DEBUG += $(filter %.o,$(ASRCS:%.s=$(OBJ_DIR_DEBUG)/%.o))
OBJS_DEBUG += $(filter %.o,$(ASRCS:%.S=$(OBJ_DIR_DEBUG)/%.o))

OBJS_RELEASE  = $(filter %.o,$(CSRCS:%.c=$(OBJ_DIR_RELEASE)/%.o))
OBJS_RELEASE += $(filter %.o,$(ASRCS:%.s=$(OBJ_DIR_RELEASE)/%.o))
OBJS_RELEASE += $(filter %.o,$(ASRCS:%.S=$(OBJ_DIR_RELEASE)/%.o))

CFLAGS_DEBUG 	= -g3 -O0 -DDEBUG
CFLAGS_RELEASE	= -g0 -Os

LDFLAGS_RELEASE	= -T"STM32G031G8UX_FLASH.ld" -Wl,-Map=$(target)_release.map
LDFLAGS_DEBUG 	= -T"STM32G031G8UX_FLASH.ld" -Wl,-Map=$(target)_debug.map

CFLAGS_DEFAULT = -mcpu=cortex-m0plus -std=gnu11 \
-DSTM32G031xx \
-DMODBULE_MCU=STM32G031xx \
-DUSER_VECT_TAB_ADDRESS \
-I./Drivers/STM32G0xx_HAL_Driver/Inc \
-I./Drivers/STM32G0xx_HAL_Driver/Inc/Legacy \
-I./Drivers/CMSIS/Device/ST/STM32G0xx/Include \
-I./Drivers/CMSIS/Include \
-I./Core/Inc \
-I../modbule/Core/Inc \
-I../modbule \
-ffunction-sections -fdata-sections -Wall -fanalyzer -fstack-usage \
-fcyclomatic-complexity \
--specs=nano.specs -mfloat-abi=soft -mthumb

ASFLAGS = -mcpu=cortex-m0plus -DMODBULE_MCU=STM32G031xx -x assembler-with-cpp \
--specs=nano.specs -mfloat-abi=soft -mthumb

LDFLAGS_DEFAULT = -mcpu=cortex-m0plus  --specs=nosys.specs \
-Wl,--gc-sections -static \
-Wl,--no-warn-rwx-segments --specs=nano.specs -mfloat-abi=soft -mthumb \
-Wl,--start-group -lc -lm -Wl,--end-group

all: release

payload:
	$(MAKE) bin_a bin_b -C ../modbule

debug: payload
debug:	export CFLAGS  := $(CFLAGS_DEBUG) $(CFLAGS_DEFAULT)
debug:	export LDFLAGS := $(LDFLAGS_DEBUG) $(LDFLAGS_DEFAULT)
debug:	export OBJS    := $(OBJS_DEBUG)
debug:	export OBJ_DIR := $(OBJ_DIR_DEBUG)
debug:	export target  := $(target)_debug.elf
debug:
	@printf "running recursively for %s\n" "$(target)"
	$(MAKE) $(target)
	@printf "went back from %s\n" "$(target)"

release: payload
release: export CFLAGS  := $(CFLAGS_RELEASE) $(CFLAGS_DEFAULT)
release: export LDFLAGS := $(LDFLAGS_RELEASE) $(LDFLAGS_DEFAULT)
release: export OBJS    := $(OBJS_RELEASE)
release: export OBJ_DIR := $(OBJ_DIR_RELEASE)
release: export target  := $(target).elf
release:
	@printf "running recursively for %s\n" "$(target)"
	$(MAKE) $(target)
	@printf "went back from %s\n" "$(target)"

%.bin: %.elf
	$(CPY) -O binary --remove-section=.wrflash $< $@
	@chmod a-x $@

%.elf: $(OBJS)
	echo $^
	$(LD) $(LDFLAGS) $^ -o $@

.SECONDARY: $(OBJS)

$(OBJ_DIR)/%.o: %.s
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.S
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -c $< -o  $@

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o  $@

clean:
	rm -rf $(OBJ_DIR_DEBUG) $(OBJ_DIR_RELEASE) *.elf *.bin *.map

.PHONY: clean all debug release payload
