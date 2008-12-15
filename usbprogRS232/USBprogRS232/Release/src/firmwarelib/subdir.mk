################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/firmwarelib/avrupdate.c 

OBJS += \
./src/firmwarelib/avrupdate.o 

C_DEPS += \
./src/firmwarelib/avrupdate.d 


# Each subdirectory must supply rules for building sources it contributes
src/firmwarelib/%.o: ../src/firmwarelib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -DUSB_CDC_ACM -Wall -O2 -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega32 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


