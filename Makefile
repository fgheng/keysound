CC = g++
LD = g++

BUILD_DIR = build
SRC = src

SRCS = $(wildcard ./src/*.cc)
OBJS = $(addprefix $(BUILD_DIR)/, $(patsubst %cc, %o, $(notdir $(SRCS))))

TARGET = keysound

INCLUDE = -I./include

LIB = -lpthread
CFLAGS = -O3
MARCO = -D

ifndef $(CFLAG)
	MARCO+=USE_PULSE
	LIB+=-lpulse-simple
endif

ifeq ($(CFLAG), pulse)
	MARCO+=USE_PULSE
	LIB+=-lpulse-simple
endif

ifeq ($(CFLAG), alsa)
	MARCO+=USE_ALSA
	LIB+=-lasound
endif

ifeq ($(CFLAG), sdl)
	MARCO+=USE_SDL
	LIB+=-lSDL2
endif



.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -o $@ $^ $(CFLAGS) $(LIB)

# 由cc生成oo，编译的时候只需要头文件和源文件，不需要-l的，但是需要-D
$(BUILD_DIR)/%.o:$(SRC)/%.cc
	@ if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi;
	$(CC) -c $(INCLUDE) $(CFLAGS) $(MARCO) -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
