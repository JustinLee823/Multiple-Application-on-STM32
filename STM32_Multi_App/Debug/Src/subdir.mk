################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Src/math.s 

C_SRCS += \
../Src/adc.c \
../Src/alarm.c \
../Src/calc.c \
../Src/debug.c \
../Src/display.c \
../Src/enviro.c \
../Src/game.c \
../Src/gpio.c \
../Src/i2c.c \
../Src/main.c \
../Src/motor.c \
../Src/spi.c \
../Src/syscalls.c \
../Src/sysclk.c \
../Src/sysmem.c \
../Src/systick.c \
../Src/testFile.c \
../Src/timer.c \
../Src/touchpad.c 

OBJS += \
./Src/adc.o \
./Src/alarm.o \
./Src/calc.o \
./Src/debug.o \
./Src/display.o \
./Src/enviro.o \
./Src/game.o \
./Src/gpio.o \
./Src/i2c.o \
./Src/main.o \
./Src/math.o \
./Src/motor.o \
./Src/spi.o \
./Src/syscalls.o \
./Src/sysclk.o \
./Src/sysmem.o \
./Src/systick.o \
./Src/testFile.o \
./Src/timer.o \
./Src/touchpad.o 

S_DEPS += \
./Src/math.d 

C_DEPS += \
./Src/adc.d \
./Src/alarm.d \
./Src/calc.d \
./Src/debug.d \
./Src/display.d \
./Src/enviro.d \
./Src/game.d \
./Src/gpio.d \
./Src/i2c.d \
./Src/main.d \
./Src/motor.d \
./Src/spi.d \
./Src/syscalls.d \
./Src/sysclk.d \
./Src/sysmem.d \
./Src/systick.d \
./Src/testFile.d \
./Src/timer.d \
./Src/touchpad.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32L552xx -DSTM32 -DSTM32L5 -DSTM32L552ZCTxQ -c -I../Inc -I/Users/justinlee/Documents/CEG3136_STM32_directory/Lab4/Drivers/CMSIS/Device/ST/STM32L5xx/Include -I/Users/justinlee/Documents/CEG3136_STM32_directory/Lab3/Drivers/CMSIS/Include -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/%.o: ../Src/%.s Src/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m33 -g3 -DDEBUG -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/adc.cyclo ./Src/adc.d ./Src/adc.o ./Src/adc.su ./Src/alarm.cyclo ./Src/alarm.d ./Src/alarm.o ./Src/alarm.su ./Src/calc.cyclo ./Src/calc.d ./Src/calc.o ./Src/calc.su ./Src/debug.cyclo ./Src/debug.d ./Src/debug.o ./Src/debug.su ./Src/display.cyclo ./Src/display.d ./Src/display.o ./Src/display.su ./Src/enviro.cyclo ./Src/enviro.d ./Src/enviro.o ./Src/enviro.su ./Src/game.cyclo ./Src/game.d ./Src/game.o ./Src/game.su ./Src/gpio.cyclo ./Src/gpio.d ./Src/gpio.o ./Src/gpio.su ./Src/i2c.cyclo ./Src/i2c.d ./Src/i2c.o ./Src/i2c.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/math.d ./Src/math.o ./Src/motor.cyclo ./Src/motor.d ./Src/motor.o ./Src/motor.su ./Src/spi.cyclo ./Src/spi.d ./Src/spi.o ./Src/spi.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysclk.cyclo ./Src/sysclk.d ./Src/sysclk.o ./Src/sysclk.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/systick.cyclo ./Src/systick.d ./Src/systick.o ./Src/systick.su ./Src/testFile.cyclo ./Src/testFile.d ./Src/testFile.o ./Src/testFile.su ./Src/timer.cyclo ./Src/timer.d ./Src/timer.o ./Src/timer.su ./Src/touchpad.cyclo ./Src/touchpad.d ./Src/touchpad.o ./Src/touchpad.su

.PHONY: clean-Src

