################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../adc.cpp \
../dht.cpp \
../ds18b20.cpp \
../main.cpp \
../nrf24l01p.cpp \
../OneWire.cpp \
../spi.cpp


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
adc.o \
dht.o \
ds18b20.o \
main.o \
nrf24l01p.o \
OneWire.o \
spi.o

OBJS_AS_ARGS +=  \
adc.o \
dht.o \
ds18b20.o \
main.o \
nrf24l01p.o \
OneWire.o \
spi.o

C_DEPS +=  \
adc.d \
dht.d \
ds18b20.d \
main.d \
nrf24l01p.d \
OneWire.d \
spi.d

C_DEPS_AS_ARGS +=  \
adc.d \
dht.d \
ds18b20.d \
main.d \
nrf24l01p.d \
OneWire.d \
spi.d

OUTPUT_FILE_PATH +=small_weather_station.elf

OUTPUT_FILE_PATH_AS_ARGS +=small_weather_station.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler















./%.o: .././%.cpp
	@echo Building file: $<
	@echo Invoking: AVR8/GNU C Compiler : 4.9.2
	$(QUOTE)D:\Stuff\Atmel\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-g++.exe$(QUOTE) -funsigned-char -funsigned-bitfields -DDEBUG  -I"D:\Stuff\Atmel\7.0\Packs\atmel\ATmega_DFP\1.1.130\include"  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega328p -B "D:\Stuff\Atmel\7.0\Packs\atmel\ATmega_DFP\1.1.130\gcc\dev\atmega328p" -c -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: AVR8/GNU Linker : 4.9.2
	$(QUOTE)D:\Stuff\Atmel\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-g++.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="small_weather_station.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=atmega328p -B "D:\Stuff\Atmel\7.0\Packs\atmel\ATmega_DFP\1.1.130\gcc\dev\atmega328p"  
	@echo Finished building target: $@
	"D:\Stuff\Atmel\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "small_weather_station.elf" "small_weather_station.hex"
	"D:\Stuff\Atmel\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "small_weather_station.elf" "small_weather_station.eep" || exit 0
	"D:\Stuff\Atmel\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "small_weather_station.elf" > "small_weather_station.lss"
	"D:\Stuff\Atmel\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "small_weather_station.elf" "small_weather_station.srec"
	"D:\Stuff\Atmel\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "small_weather_station.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "small_weather_station.elf" "small_weather_station.a" "small_weather_station.hex" "small_weather_station.lss" "small_weather_station.eep" "small_weather_station.map" "small_weather_station.srec" "small_weather_station.usersignatures"
	