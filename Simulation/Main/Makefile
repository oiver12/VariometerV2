CC                    := gcc
BIN                   := posix_demo

BUILD_DIR             := ./build
BUILD_DIR_ABS         := $(abspath $(BUILD_DIR))

FREERTOS_DIR_REL      := ../FreeRTOS/FreeRTOS
FREERTOS_DIR          := $(abspath $(FREERTOS_DIR_REL))

FREERTOS_PLUS_DIR_REL := ../FreeRTOS/FreeRTOS-Plus
FREERTOS_PLUS_DIR     := $(abspath $(FREERTOS_PLUS_DIR_REL))

KERNEL_DIR            := ${FREERTOS_DIR}/Source

VARIOMETER_DIR_REL   := ./variometer-software/main
VARIOMETER_DIR       := $(abspath $(VARIOMETER_DIR_REL))

INCLUDE_DIRS          := -I.
INCLUDE_DIRS          += -I./Trace_Recorder_Configuration
INCLUDE_DIRS          += -I${KERNEL_DIR}/include
INCLUDE_DIRS          += -I${KERNEL_DIR}/portable/ThirdParty/GCC/Posix
INCLUDE_DIRS          += -I${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils
INCLUDE_DIRS          += -I${FREERTOS_DIR}/Demo/Common/include
INCLUDE_DIRS          += -I${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/Include
INCLUDE_DIRS          += -I${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/config
INCLUDE_DIRS          += -I${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/streamports/File/include
INCLUDE_DIRS          += -I${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/streamports/File/config

INCLUDE_DIRS          += -I${VARIOMETER_DIR}
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/AirspaceParcer
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/Bluetooth
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/Flash
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/IGC
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/Kalman
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/Kalman/Quaternion
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/Screen
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/Screen/assets
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/Screen/E_Ink
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/Sensor
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/TestTools
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/Watchdog
INCLUDE_DIRS          += -I${VARIOMETER_DIR}/crc
INCLUDE_DIRS          += -I./Mock

SOURCE_FILES          := $(wildcard *.c)
SOURCE_FILES          += $(wildcard ${FREERTOS_DIR}/Source/*.c)
# Memory manager (use malloc() / free() )
SOURCE_FILES          += ${KERNEL_DIR}/portable/MemMang/heap_3.c
# posix port
SOURCE_FILES          += ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
SOURCE_FILES          += ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/port.c

# Demo library.
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/AbortDelay.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/BlockQ.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/blocktim.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/countsem.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/death.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/dynamic.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/EventGroupsDemo.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/flop.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/GenQTest.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/integer.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/IntSemTest.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/MessageBufferAMP.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/MessageBufferDemo.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/PollQ.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/QPeek.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/QueueOverwrite.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/QueueSet.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/QueueSetPolling.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/recmutex.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/semtest.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/StaticAllocation.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/StreamBufferDemo.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/StreamBufferInterrupt.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/TaskNotify.c
SOURCE_FILES          += ${FREERTOS_DIR}/Demo/Common/Minimal/TimerDemo.c


SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/AirspaceParcer/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/Bluetooth/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/Flash/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/IGC/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/Kalman/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/Kalman/Quaternion/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/Screen/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/Screen/assets/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/Screen/E_Ink/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/Sensor/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/TestTools/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/Watchdog/*.c)
SOURCE_FILES          += $(wildcard ${VARIOMETER_DIR}/crc/*.c)
SOURCE_FILES 		  += $(wildcard ./Mock/*.c)
SOURCE_FILES          += $(wildcard ./Mock/driver/*.c)
SOURCE_FILES          += $(wildcard ./Mock/esp_adc/*.c)
SOURCE_FILES          += ${wildcard ./SDLHelper/*.c}

CFLAGS                :=    -ggdb3
LDFLAGS               :=    -ggdb3 -pthread -lm -lSDL2
CPPFLAGS              :=    $(INCLUDE_DIRS) -DBUILD_DIR=\"$(BUILD_DIR_ABS)\"
CPPFLAGS              +=    -D_WINDOWS_ -D ISSIMULATION

ifeq ($(TRACE_ON_ENTER),1)
  CPPFLAGS              += -DTRACE_ON_ENTER=1
else
  CPPFLAGS              += -DTRACE_ON_ENTER=0
endif

ifeq ($(COVERAGE_TEST),1)
  CPPFLAGS              += -DprojCOVERAGE_TEST=1
else
  CPPFLAGS              += -DprojCOVERAGE_TEST=0
# Trace library.
  SOURCE_FILES          += ${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/trcKernelPort.c
  SOURCE_FILES          += $(wildcard ${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/*.c )
endif

ifdef PROFILE
  CFLAGS              +=   -pg  -O0 -g
  LDFLAGS             +=   -pg  -O0 -g
else
  CFLAGS              +=   -O3
  LDFLAGS             +=   -O3
endif

ifdef SANITIZE_ADDRESS
  CFLAGS              +=   -fsanitize=address -fsanitize=alignment
  LDFLAGS             +=   -fsanitize=address -fsanitize=alignment
endif

ifdef SANITIZE_LEAK
  LDFLAGS             +=   -fsanitize=leak
endif

ifeq ($(USER_DEMO),BLINKY_DEMO)
  CPPFLAGS            +=   -DUSER_DEMO=0
endif

ifeq ($(USER_DEMO),FULL_DEMO)
  CPPFLAGS            +=   -DUSER_DEMO=1
endif


OBJ_FILES = $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.o)

DEP_FILE = $(OBJ_FILES:%.o=%.d)

${BIN} : $(BUILD_DIR)/$(BIN)

${BUILD_DIR}/${BIN} : ${OBJ_FILES}
	-mkdir -p ${@D}
	$(CC) $^ ${LDFLAGS} -o $@

-include ${DEP_FILE}

${BUILD_DIR}/%.o : %.c Makefile
	-mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c $< -o $@

.PHONY: clean

build:
	${BUILD_DIR}/${BIN} : ${OBJ_FILES}
		-mkdir -p ${@D}
		$(CC) $^ ${LDFLAGS} -o $@

	-include ${DEP_FILE}

	${BUILD_DIR}/%.o : %.c Makefile
		-mkdir -p $(@D)
		$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c $< -o $@

run: build
	./${BUILD_DIR}/${BIN}

clean:
	-rm -rf $(BUILD_DIR)


GPROF_OPTIONS := --directory-path=$(INCLUDE_DIRS)
profile:
	gprof -a -p --all-lines $(GPROF_OPTIONS) $(BUILD_DIR)/$(BIN) $(BUILD_DIR)/gmon.out > $(BUILD_DIR)/prof_flat.txt
	gprof -a --graph $(GPROF_OPTIONS) $(BUILD_DIR)/$(BIN) $(BUILD_DIR)/gmon.out > $(BUILD_DIR)/prof_call_graph.txt

