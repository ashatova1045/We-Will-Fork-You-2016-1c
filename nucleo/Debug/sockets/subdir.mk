################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/utnso/workspace/tp-2016-1c-We-Will-Fork-You/sockets/Sockets.c 

OBJS += \
./sockets/Sockets.o 

C_DEPS += \
./sockets/Sockets.d 


# Each subdirectory must supply rules for building sources it contributes
sockets/Sockets.o: /home/utnso/workspace/tp-2016-1c-We-Will-Fork-You/sockets/Sockets.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


