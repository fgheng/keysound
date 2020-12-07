#include "DeviceDetect.hpp"
#include "Audio.hpp"
#include "Play.hpp"
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>
#include <iostream>
#include <thread>

extern "C" {
#include <unistd.h>
}

int main(int argc, char *argv[])
{
    Audio audio("./sources/piano");
    // Audio audio("./sources/keyany.wav", SOLE);
    Mixer mixer(audio.get_max_len()*2);

    std::thread th(device_detect, &audio, &mixer);
    th.detach();

    play(mixer, audio.get_fmt_size(), audio.get_channels(),
            audio.get_sample_rate(), audio.get_bits_per_sample());

    return 0;
}
