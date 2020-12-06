#ifndef __DEVICE_DETECT__
#define __DEVICE_DETECT__

#include "Audio.hpp"
#include "Mixer.hpp"

extern void device_detect(Audio *, Mixer *);
extern void stop_detect();

#endif
