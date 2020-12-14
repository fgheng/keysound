#include "Audio.hpp"
#include "KeyDetect.hpp"
#include "utils.hpp"
#include <string>
#include <iostream>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>
}

static const std::string input_file_header = "/dev/input/event";


// 设置event_id，用于监控消息队列中是否有该类型消息有，
// 则表明该线程该退出了
void key_detect(std::string str_event_id, Audio *audio, Mixer *mixer) {
    input_event ie;
    // int tid = std::stoi(event_id);

    int fd;
    fd_set fds;
    struct timeval tv;

    fd = open((input_file_header + str_event_id).c_str(), O_RDONLY);

    if (fd == -1) {
        std::cout << "error occured while open event file, ";
        std::cout << "you maybe has no access to the file " << std::endl;
        return;
    }

    // 成功运行，添加到key_detect_threads中
    add_event_id(str_event_id);
    while(event_id_exists(str_event_id)) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;

        int ret = select(fd+1, &fds, NULL, NULL, &tv);

        if (ret < 0 ) continue;
        if (!(ret > 0 && FD_ISSET(fd, &fds))) continue;

        ssize_t num = read(fd, (void *)&ie, sizeof(struct input_event));

        if (num == -1 || num != sizeof(struct input_event)) continue;

        // 键盘事件
        if (ie.type == EV_KEY) {
            switch (ie.value) {
                case KEY_PRESS:
                {
                    WAV_DATA wd = audio->get_wav_by_code(ie.code);
                    std::cout << " [" << ie.code << "] is pressed " << std::endl;
                    mixer->mix(wd.data, wd.len, wd.bits_per_sample);
                    break;
                }
                case KEY_RELEASE:
                {
                    // std::cout << "some key release" << std::endl;
                    break;
                }
                case KEY_REPEAT:
                {
                    // std::cout << "some key repeat" << std::endl;
                    break;
                }
                default: break;
            }
        }
    }

    if (fd>0) close(fd);
}
