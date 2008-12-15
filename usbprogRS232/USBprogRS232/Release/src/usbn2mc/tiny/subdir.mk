################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/usbn2mc/tiny/usbn960x.c \
../src/usbn2mc/tiny/usbnapi.c 

OBJS += \
./src/usbn2mc/tiny/usbn960x.o \
./src/usbn2mc/tiny/usbnapi.o 

C_DEPS += \
./src/usbn2mc/tiny/usbn960x.d \
./src/usbn2mc/tiny/usbnapi.d 


# Each subdirectory must supply rules for building sources it contributes
src/usbn2mc/tiny/%.o: ../src/usbn2mc/tiny/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -DUSB_CDC_ACM -Wall -O2 -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega32 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


