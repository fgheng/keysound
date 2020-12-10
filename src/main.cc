#include "DeviceDetect.hpp"
#include "Audio.hpp"
#include "Play.hpp"
#include "args.hpp"
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
    process_command_line_arguments(argc, argv);

    std::string str;

    switch (args.flag) {
        case 'd':
            str = args.dir;
            break;
        case 'f':
            str = args.wav_file;
            break;
        case 'j':
            str = args.json;
            break;
        default:
            exit(EXIT_FAILURE);
    }

    Audio audio(str, args.flag);
    Mixer mixer(audio.get_max_len());

    std::thread th(device_detect, &audio, &mixer);
    th.detach();

    play(mixer, 2048, audio.get_channels(),
        audio.get_sample_rate(), audio.get_bits_per_sample());

    return 0;
}
