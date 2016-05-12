################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Conexiones_Umc.c \
../src/Config_Umc.c \
../src/Consola_Umc.c \
../src/Log_Umc.c \
../src/estructuras_umc.c \
../src/umc.c 

OBJS += \
./src/Conexiones_Umc.o \
./src/Config_Umc.o \
./src/Consola_Umc.o \
./src/Log_Umc.o \
./src/estructuras_umc.o \
./src/umc.o 

C_DEPS += \
./src/Conexiones_Umc.d \
./src/Config_Umc.d \
./src/Consola_Umc.d \
./src/Log_Umc.d \
./src/estructuras_umc.d \
./src/umc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


