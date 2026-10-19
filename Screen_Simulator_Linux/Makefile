CC ?= gcc
CPPFLAGS += $(shell pkg-config --cflags sdl2)
CFLAGS ?= -O2 -g
# The archived renderer shares tentative global definitions between source files.
CFLAGS += -fcommon
LDLIBS += $(shell pkg-config --libs sdl2) -lm
SOURCES := main.c airspaceParcer.c SDCard.c UI_Demo.c file_layer.c esp_timer.c Simulation.c \
           $(wildcard Screen/*.c Screen/E_Ink/*.c Screen/freertos/*.c Screen/assets/*.c)
HEADERS := $(wildcard *.h Screen/*.h Screen/E_Ink/*.h Screen/freertos/*.h Screen/assets/*.h)

.PHONY: all run clean
all: build/screen-simulator

build/screen-simulator: $(SOURCES) $(HEADERS)
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

run: all
	./build/screen-simulator

clean:
	rm -rf build
