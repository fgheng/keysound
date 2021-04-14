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

/*代码注释 / Code Comments*/
/*CN 中文               English*/
/*Note: Translated using an online tool and some elbow grease,
 *so the translation may not be 100% accurate.
 */


static const std::string input_file_header = "/dev/input/event";


// 设置event_id，用于监控消息队列中是否有该类型消息有，		Set event_id to monitor whether there are messages of this type in the message queue,
// 则表明该线程该退出了						Indicates that the thread should exit
void key_detect(std::string str_event_id, Audio *audio, Mixer *mixer) {
    input_event ie;
    // int tid = std::stoi(event_id);

    int fd;
    fd_set fds;
    struct timeval tv;

    // 如果使用suid的话，不需要sleep即可正常读取/dev/input/event文件				If you use suid, you can read /dev/input/event files normally without using sleep
    // 如果使用input组的方式，那么需要加上sleep，否则新插入的键盘会读取失败			If you use the "input" group method, you need to add sleep, otherwise the newly inserted keyboard will fail to read
    // 不清楚是为什么，是因为文件创建的慢了？还是权限不够了？					Not sure why, is it because the file creation is slow? Still not enough permissions?
    // 也不是文件不存在的问题，我尝试判断文件是否存在，文件是存在的				It's not because the file doesn't exist, I tried to see if it does and it does in fact exist.
    // 那是因为权限没有跟上？尝试了一下确实是不可读，这是为什么呢？因为权限管理的延迟？		Is that because the permissions didn't carry over? After one  try, it becomes  unreadable. Why is this? Because 
    // 使用open的阻塞方式 O_NONBLOCK 打开也不行							of the delay in rights management?
    // add_event_id(str_event_id);								Use O_NONBLOCK method when opening
    // while (event_id_exists(str_event_id) && access((input_file_header + str_event_id).c_str(), R_OK) == -1) {
        // usleep(100 * 1000);
        // std::cout << "判断是否有权限" << std::endl;
    // }
    // 经过搜索，我找到了原因，是因为程序读取太快，在udev正确设置权限之前就读取了		After searching, I found the reason, because the program reads too fast, it reads before udev sets the permissions correctly
    // 有两个解决方案，一个是等待一段时间，比如sleep(1)						There are two solutions, one is to wait for a period of time, such as with  sleep(1)
    // 第二个是使用inotify监控文件的权限更改							The second choice is to use "inotify" to monitor file permission changes
    sleep(1);
    fd = open((input_file_header + str_event_id).c_str(), O_RDONLY);

    if (fd == -1) {
        std::cout << "Error occured while attempting to open event file: " << input_file_header + str_event_id;
        std::cout << " you may not have access to the file " << std::endl;
        return;
    }

    // 成功运行，添加到key_detect_threads中		It has successfully run, add to key_detect_threads
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

        // 键盘事件	Keyboard events
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
