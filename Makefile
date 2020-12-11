CC = /usr/bin/g++
LD = /usr/bin/g++

BUILD_DIR = build
SRC = src

SRCS = $(wildcard ./src/*.cc)
OBJS = $(addprefix $(BUILD_DIR)/, $(patsubst %cc, %o, $(notdir $(SRCS))))

TARGET = keysound

INCLUDE = -I./include

LIB = -lpthread
LIB_USE_ALSA = -lalsound -D USE_ALSA
LIB_USE_PULSE = -lpulse-simple -D USE_PULSE
LIB_USE_SDL = -lSDL2 -D USE_SDL

CFLAGS = -O3


.PHONY: all clean pulse alsa sdl

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -o $@ $^ $(LIB) $(LIB_USE_PULSE)

# 由cc生成oo，编译的时候只需要头文件和源文件，不需要-l的，但是需要-D
$(BUILD_DIR)/%.o:$(SRC)/%.cc
	@ if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi;
	$(CC) -c $(INCLUDE) $(CFLAGS) $(LIB_USE_PULSE) -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
