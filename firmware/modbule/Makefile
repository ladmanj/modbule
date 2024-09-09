target = modbule

AS = arm-none-eabi-gcc
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
CPY = arm-none-eabi-objcopy

UC = $(shell echo '$1' | tr '[:lower:]' '[:upper:]')


OBJ_DIR_DEBUG = obj_debug
OBJ_DIR_REL_A = obj_rel_a
OBJ_DIR_REL_B = obj_rel_b

CSRCS  = $(shell find * -maxdepth 7 -name '*.c')
ASRCS  = $(shell find * -maxdepth 7 \( -name '*.S' -o -name '*.s' \) )

OBJS_DEBUG  = $(filter %.o,$(CSRCS:%.c=$(OBJ_DIR_DEBUG)/%.o))
OBJS_DEBUG += $(filter %.o,$(ASRCS:%.s=$(OBJ_DIR_DEBUG)/%.o))
OBJS_DEBUG += $(filter %.o,$(ASRCS:%.S=$(OBJ_DIR_DEBUG)/%.o))

OBJS_REL_A  = $(filter %.o,$(CSRCS:%.c=$(OBJ_DIR_REL_A)/%.o))
OBJS_REL_A += $(filter %.o,$(ASRCS:%.s=$(OBJ_DIR_REL_A)/%.o))
OBJS_REL_A += $(filter %.o,$(ASRCS:%.S=$(OBJ_DIR_REL_A)/%.o))

OBJS_REL_B  = $(filter %.o,$(CSRCS:%.c=$(OBJ_DIR_REL_B)/%.o))
OBJS_REL_B += $(filter %.o,$(ASRCS:%.s=$(OBJ_DIR_REL_B)/%.o))
OBJS_REL_B += $(filter %.o,$(ASRCS:%.S=$(OBJ_DIR_REL_B)/%.o))

CFLAGS_DEBUG 	= -g3 -O0 -DVECT_TAB_BASE_ADDRESS=0x8000000 -DDEBUG
CFLAGS_REL_A	= -g0 -Os -DVECT_TAB_BASE_ADDRESS=0x8000800
CFLAGS_REL_B	= -g0 -Os -DVECT_TAB_BASE_ADDRESS=0x8008000

LDFLAGS_REL_A	= -T"STM32G031G8UX_FLASH_A.ld"	-Wl,-Map=$(target)_release_a.map
LDFLAGS_REL_B	= -T"STM32G031G8UX_FLASH_B.ld"	-Wl,-Map=$(target)_release_b.map
LDFLAGS_DEBUG 	= -T"STM32G031G8UX_debug.ld"	-Wl,-Map=$(target)_debug.map

CFLAGS_DEFAULT = -mcpu=cortex-m0plus -std=gnu11 \
-DUSE_HAL_DRIVER \
-DSTM32G031xx \
-DMODBULE_MCU=STM32G031xx \
-DUSER_VECT_TAB_ADDRESS \
-I./Drivers/STM32G0xx_HAL_Driver/Inc \
-I./Drivers/STM32G0xx_HAL_Driver/Inc/Legacy \
-I./Drivers/CMSIS/Device/ST/STM32G0xx/Include \
-I./Drivers/CMSIS/Include \
-I./Core/Inc \
-ffunction-sections -fdata-sections -Wall -fanalyzer -fstack-usage \
-fcyclomatic-complexity \
--specs=nano.specs -mfloat-abi=soft -mthumb

ASFLAGS = -mcpu=cortex-m0plus -DMODBULE_MCU=STM32G031xx -x assembler-with-cpp \
--specs=nano.specs -mfloat-abi=soft -mthumb

LDFLAGS_DEFAULT = -mcpu=cortex-m0plus  --specs=nosys.specs \
-Wl,--gc-sections -static \
-Wl,--no-warn-rwx-segments --specs=nano.specs -mfloat-abi=soft -mthumb \
-Wl,--start-group -lc -lm -Wl,--end-group

all: $(target)

debug:	export CFLAGS  := $(CFLAGS_DEBUG) $(CFLAGS_DEFAULT)
debug:	export LDFLAGS := $(LDFLAGS_DEBUG) $(LDFLAGS_DEFAULT)
debug:	export OBJS    := $(OBJS_DEBUG)
debug:	export OBJ_DIR := $(OBJ_DIR_DEBUG)
debug:	export target  := $(target)_debug.elf
debug:
	@printf "running recursively %s\n" "$(OBJS)"
	$(MAKE) $(target)
	@printf "went back\n"

bin_a:	export CFLAGS  := $(CFLAGS_REL_A) $(CFLAGS_DEFAULT)
bin_a:	export LDFLAGS := $(LDFLAGS_REL_A) $(LDFLAGS_DEFAULT)
bin_a:	export OBJS    := $(OBJS_REL_A)
bin_a:	export OBJ_DIR := $(OBJ_DIR_REL_A)
bin_a:	export target  := $(target)_a.bin
bin_a:
	@printf "running recursively %s\n" "$(OBJS)"
	$(MAKE) $(target)
	@printf "went back\n"

bin_b:	export CFLAGS  := $(CFLAGS_REL_B) $(CFLAGS_DEFAULT)
bin_b:	export LDFLAGS := $(LDFLAGS_REL_B) $(LDFLAGS_DEFAULT)
bin_b:	export OBJS    := $(OBJS_REL_B)
bin_b:	export OBJ_DIR := $(OBJ_DIR_REL_B)
bin_b:	export target  := $(target)_b.bin
bin_b:
	@printf "running recursively %s\n" "$(OBJS)"
	$(MAKE) $(target)
	@printf "went back\n"

%.bin: %.elf
	$(CPY) -O binary --remove-section=.wrflash $< $@
	@chmod a-x $@
	@./crcit.py $@ 30720 $(call UC,$(basename $<)) > $(basename $@).h

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
	rm -rf $(OBJ_DIR_DEBUG) $(OBJ_DIR_REL_A) $(OBJ_DIR_REL_B) *.elf *.bin *.h *.map

.PHONY: clean all debug
