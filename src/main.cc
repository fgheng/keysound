#include "DeviceDetect.hpp"
#include "Audio.hpp"
#include "Play.hpp"
#include "args.hpp"
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>
#include <iostream>
#include <thread>

extern "C" {
#include <unistd.h>
}

void signal_handler(int signal) {
    stop_detect();
    stop_play();
}

void signal_handling() {
    struct sigaction act = {{0}};
    act.sa_handler = signal_handler;

    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);

    // 避免成为僵尸进程?
    act.sa_handler = SIG_IGN;
    sigaction(SIGTERM, &act, NULL);
}

int main(int argc, char *argv[])
{
    signal_handling();
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

    if (args.daemon) {
        daemon(1, 0);
    }

    Audio audio(str, args.flag);
    Mixer mixer(audio.get_max_len());

    std::thread th1(device_detect, &audio, &mixer);
    std::thread th2(play, &mixer, 2048, audio.get_channels(),
        audio.get_sample_rate(), audio.get_bits_per_sample());

    // 等待子线程结束，避免主线程提前释放资源后子线程访问失败
    th1.join();
    th2.join();

    return 0;
}
