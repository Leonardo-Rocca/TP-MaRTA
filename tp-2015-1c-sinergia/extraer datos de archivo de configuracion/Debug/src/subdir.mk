################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/extraer\ datos\ de\ archivo\ de\ configuracion.c 

OBJS += \
./src/extraer\ datos\ de\ archivo\ de\ configuracion.o 

C_DEPS += \
./src/extraer\ datos\ de\ archivo\ de\ configuracion.d 


# Each subdirectory must supply rules for building sources it contributes
src/extraer\ datos\ de\ archivo\ de\ configuracion.o: ../src/extraer\ datos\ de\ archivo\ de\ configuracion.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/extraer datos de archivo de configuracion.d" -MT"src/extraer\ datos\ de\ archivo\ de\ configuracion.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


