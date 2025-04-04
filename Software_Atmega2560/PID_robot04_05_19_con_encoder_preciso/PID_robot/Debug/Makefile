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
../Accessori.c \
../BNO055.c \
../Posizione.c \
../EEPROM.c \
../I2C.c \
../interrupt.c \
../millis.c \
../PID.c \
../PID_linea.c \
../PID_robot.c \
../PWM.c \
../Seriale.c \
../Stanza.c \
../Vl6180x.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
Accessori.o \
BNO055.o \
Posizione.o \
EEPROM.o \
I2C.o \
interrupt.o \
millis.o \
PID.o \
PID_linea.o \
PID_robot.o \
PWM.o \
Seriale.o \
Stanza.o \
Vl6180x.o

OBJS_AS_ARGS +=  \
Accessori.o \
BNO055.o \
Posizione.o \
EEPROM.o \
I2C.o \
interrupt.o \
millis.o \
PID.o \
PID_linea.o \
PID_robot.o \
PWM.o \
Seriale.o \
Stanza.o \
Vl6180x.o

C_DEPS +=  \
Accessori.d \
BNO055.d \
Posizione.d \
EEPROM.d \
I2C.d \
interrupt.d \
millis.d \
PID.d \
PID_linea.d \
PID_robot.d \
PWM.d \
Seriale.d \
Stanza.d \
Vl6180x.d

C_DEPS_AS_ARGS +=  \
Accessori.d \
BNO055.d \
Posizione.d \
EEPROM.d \
I2C.d \
interrupt.d \
millis.d \
PID.d \
PID_linea.d \
PID_robot.d \
PWM.d \
Seriale.d \
Stanza.d \
Vl6180x.d

OUTPUT_FILE_PATH +=PID_robot.elf

OUTPUT_FILE_PATH_AS_ARGS +=PID_robot.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler





























./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.8.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g2 -Wall -mmcu=atmega2560 -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
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
	@echo Invoking: AVR/GNU Linker : 4.8.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="PID_robot.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mrelax -mmcu=atmega2560  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "PID_robot.elf" "PID_robot.hex"
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "PID_robot.elf" "PID_robot.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "PID_robot.elf" > "PID_robot.lss"
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "PID_robot.elf" "PID_robot.srec"
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-size.exe" "PID_robot.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "PID_robot.elf" "PID_robot.a" "PID_robot.hex" "PID_robot.lss" "PID_robot.eep" "PID_robot.map" "PID_robot.srec" "PID_robot.usersignatures"
	