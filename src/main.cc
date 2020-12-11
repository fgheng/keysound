#include "DeviceDetect.hpp"
#include "Audio.hpp"
#include "Play.hpp"
#include "args.hpp"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <iostream>
#include <thread>

extern "C" {
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <error.h>
}

#define PID_FILE "/tmp/keysound_pid"

static void signal_handler(int signal) {
    stop_detect();
    stop_play();
}

static void signal_handling() {
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

static std::string execute(const std::string &cmd) {
    FILE *pipe = popen(cmd.c_str(), "r");
    std::string result = "";

    if (pipe) {
        char buf[64];
        while (!feof(pipe)) {
            if (fgets(buf, 64, pipe))
                result += buf;
        }
        pclose(pipe);
    }

    return result;
}

// // 主要是避免从文件中读取的时候，读取到一个其他进程的id
// // 避免的方式是进程结束的时候自动删除文件
// static bool pid_is_real(pid_t pid) {
    // static std::string cmd =
        // std::string("ps ax | grep ") + std::to_string(pid) +
        // " | grep " + program_invocation_short_name +
        // " | grep -v grep";

    // if (execute(cmd) == "")
        // return true;
    // else
        // return false;
// }

// 从指定文件读取已经存在的线程并结束
static void kill_exists_process() {
    static std::string cmd =
        std::string("ps ax | grep ") +
        program_invocation_short_name +
        " | grep -v grep";

    pid_t pid = 0;

    FILE *tmp = fopen(PID_FILE, "r");

    // 文件存在，读取文件
    if (tmp != NULL && fscanf(tmp, "%d", &pid) ==1) {
        fclose(tmp);
    } else {
        // 文件不存在，使用命令获取
        // 成功获取pid
        sscanf(execute(cmd).c_str(), "%d", &pid);
    }

    if (pid > 0 && pid != getpid() /*&& pid_is_real(pid)*/) {
        kill(pid, SIGINT);
        remove(PID_FILE);
    }
}

// 保存当前线程到指定的文件
static void create_pid_file() {
    // 文件存在返回-1
    int fd = open(PID_FILE, O_WRONLY | O_CREAT | O_EXCL, 0644);

    if (fd != -1) {
        char pid[16] = {0};
        sprintf(pid, "%d", getpid());

        if (write(fd, pid, strlen(pid)) == -1) {
            close(fd);
            remove(PID_FILE);

            std::cout << "write pid file error" << std::endl;
            // exit(EXIT_FAILURE);
        } else close(fd);
    } else {
        if (errno == EEXIST) {
            std::cout << "another process is running" << std::endl;
        } else {
            std::cout << "read file error" << std::endl;
        }
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    process_command_line_arguments(argc, argv);
    // 结束进程
    if (args.kill) {
        kill_exists_process();
        exit(EXIT_SUCCESS);
    }

    signal_handling();
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

    // 杀掉已经存在的线程
    kill_exists_process();
    sleep(1);
    // 创建当前进程的文件
    create_pid_file();

    std::thread th1(device_detect, &audio, &mixer);
    std::thread th2(play, &mixer, 2048, audio.get_channels(),
        audio.get_sample_rate(), audio.get_bits_per_sample());

    // 等待子线程结束，避免主线程提前释放资源后子线程访问失败
    th1.join();
    th2.join();

    // 移除临时文件，这样不可，新进程创建的速度要快于通知旧进程结束的信号的速度
    // 所以如果启动一个新进程，那么旧进程会将新进程的文件删除，那怎么办呢？
    // remove(PID_FILE);
    return 0;
}
