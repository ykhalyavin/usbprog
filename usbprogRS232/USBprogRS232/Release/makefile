################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/subdir.mk
-include src/usbn2mc/tiny/subdir.mk
-include src/usbn2mc/class/CDC/subdir.mk
-include src/firmwarelib/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
USBprogRS232.lss \

FLASH_IMAGE += \
USBprogRS232.bin \

SIZEDUMMY += \
sizedummy \


# All Target
all: USBprogRS232.elf secondary-outputs

# Tool invocations
USBprogRS232.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,USBprogRS232.map -mmcu=atmega32 -o"USBprogRS232.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

USBprogRS232.lss: USBprogRS232.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -r -h -S USBprogRS232.elf  >"USBprogRS232.lss"
	@echo 'Finished building: $@'
	@echo ' '

USBprogRS232.bin: USBprogRS232.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -O binary USBprogRS232.elf  "USBprogRS232.bin"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: USBprogRS232.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=berkeley -t -x USBprogRS232.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) USBprogRS232.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
