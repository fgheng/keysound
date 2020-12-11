#ifndef __PLAY__
#define __PLAY__

#include "Mixer.hpp"

// #define USE_ALSA
// #define USE_PULSE
// #define USE_SDL
// #define SDL_CALL_BACK

#ifdef USE_ALSA
extern void play(Mixer *, uint16_t frame_size, uint16_t channels,
        uint32_t sample_rate, uint16_t bits_per_sample);
#endif
#ifdef USE_PULSE
extern void play(Mixer *, uint16_t frame_size, uint16_t channels,
        uint32_t sample_rate, uint16_t bits_per_sample);
#endif
#ifdef USE_SDL
extern void play(Mixer *, uint16_t frame_size, uint16_t channels,
        uint32_t sample_rate, uint16_t bits_per_sample);
#endif

extern void stop_play();

#endif
