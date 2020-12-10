#ifndef __PLAY__
#define __PLAY__

#include "Mixer.hpp"

// #define __USE_ALSA__
#define __USE_PULSE__
// #define __USE_SDL__
// #define __SDL_CALL_BACK__

#ifdef __USE_ALSA__
extern void play(Mixer &, uint16_t frame_size, uint16_t channels,
        uint32_t sample_rate, uint16_t bits_per_sample);
#endif
#ifdef __USE_PULSE__
extern void play(Mixer &, uint16_t frame_size, uint16_t channels,
        uint32_t sample_rate, uint16_t bits_per_sample);
#endif
#ifdef __USE_SDL__
extern void play(Mixer &, uint16_t frame_size, uint16_t channels,
        uint32_t sample_rate, uint16_t bits_per_sample);
#endif

#endif
