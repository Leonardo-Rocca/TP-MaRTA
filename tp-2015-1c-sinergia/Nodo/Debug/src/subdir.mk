################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Apareo.c \
../src/Nodo.c \
../src/RedireccionarScript.c \
../src/mensajesNodo.c \
../src/redireccionarVersion2.c 

OBJS += \
./src/Apareo.o \
./src/Nodo.o \
./src/RedireccionarScript.o \
./src/mensajesNodo.o \
./src/redireccionarVersion2.o 

C_DEPS += \
./src/Apareo.d \
./src/Nodo.d \
./src/RedireccionarScript.d \
./src/mensajesNodo.d \
./src/redireccionarVersion2.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


