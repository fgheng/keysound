#ifndef __KEY_DETECT__
#define __KEY_DETECT__

#include "Audio.hpp"
#include "Mixer.hpp"
#include <string>

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_REPEAT 2

void key_detect(std::string, Audio *, Mixer *);

#endif
