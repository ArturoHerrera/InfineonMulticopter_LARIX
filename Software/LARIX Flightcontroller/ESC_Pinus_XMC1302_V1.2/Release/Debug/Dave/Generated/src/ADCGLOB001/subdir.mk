################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Dave/Generated/src/ADCGLOB001/ADCGlob001.c \
../Dave/Generated/src/ADCGLOB001/ADCGlob001_Conf.c 

OBJS += \
./Dave/Generated/src/ADCGLOB001/ADCGlob001.o \
./Dave/Generated/src/ADCGLOB001/ADCGlob001_Conf.o 

C_DEPS += \
./Dave/Generated/src/ADCGLOB001/ADCGlob001.d \
./Dave/Generated/src/ADCGLOB001/ADCGlob001_Conf.d 


# Each subdirectory must supply rules for building sources it contributes
Dave/Generated/src/ADCGLOB001/%.o: ../Dave/Generated/src/ADCGLOB001/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM-GCC C Compiler'
	"C:\DAVE-3.1.10\ARM-GCC/bin/arm-none-eabi-gcc" -DDAVE_CE -DUC_ID=13010211 -I"C:\DAVE-3.1.10\eclipse\/../CMSIS/Include" -I"C:\DAVE-3.1.10\eclipse\/../CMSIS/Infineon/Include" -I"C:\DAVE-3.1.10\ARM-GCC/arm-none-eabi/include" -I"C:\DAVE-3.1.10\eclipse\/../emWin/Start/GUI/inc" -I"C:\DAVE-3.1.10\eclipse\/../CMSIS/Infineon/XMC1300_series/Include" -I"C:\Dave_Backup_Workspace\ESC_Pinus_XMC1302_V1.2\Dave\Generated\inc\MATHLIBS" -I"C:\Dave_Backup_Workspace\ESC_Pinus_XMC1302_V1.2\Dave\Generated\inc\DAVESupport" -I"C:\Dave_Backup_Workspace\ESC_Pinus_XMC1302_V1.2\Dave\Generated\inc\MOTORLIBS" -I"C:\Dave_Backup_Workspace\ESC_Pinus_XMC1302_V1.2\Dave\Generated\inc\LIBS" -O0 -ffunction-sections -Wall -std=gnu99 -mfloat-abi=soft -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $@" -mcpu=cortex-m0 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


