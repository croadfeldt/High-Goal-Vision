################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Goal.cpp \
../src/High\ Goal\ Vision.cpp 

OBJS += \
./src/Goal.o \
./src/High\ Goal\ Vision.o 

CPP_DEPS += \
./src/Goal.d \
./src/High\ Goal\ Vision.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/High\ Goal\ Vision.o: ../src/High\ Goal\ Vision.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/High Goal Vision.d" -MT"src/High\ Goal\ Vision.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


