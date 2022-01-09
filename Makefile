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
MARCO =

ifeq ($(CFLAG), )
	MARCO+=-D USE_PULSE
	LIB+=-lpulse-simple
	LIB+=-lpulse
else ifeq ($(CFLAG), pulse)
	MARCO+=-D USE_PULSE
	LIB+=-lpulse-simple
else ifeq ($(CFLAG), alsa)
	MARCO+=-D USE_ALSA
	LIB+=-lasound
else ifeq ($(CFLAG), sdl)
	MARCO+=-D USE_SDL
	LIB+=-lSDL2
endif



.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -o $@ $^ $(CFLAGS) $(LIB)

# 由cc生成oo，编译的时候只需要头文件和源文件，不需要-l的，但是需要-D
# Generate oo from cc files, but only using the header and source files
#	for compiling. Don't use -I, instead use -D
$(BUILD_DIR)/%.o:$(SRC)/%.cc
	@ if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi;
	$(CC) -c $(INCLUDE) $(CFLAGS) $(MARCO) -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
