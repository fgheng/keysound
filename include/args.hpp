#ifndef __ARGS__
#define __ARGS__

#include <string>
#include <iostream>

#include "usage.hpp"

extern "C" {
#include <unistd.h>
#include <getopt.h>
#include <bits/getopt_ext.h>
}

extern char *optarg;

static struct arguments {
    int flag = 0;
    bool daemon = false;
    std::string json = "";
    std::string wav_file = "";
    std::string dir = "";
    std::string log = "";
} args = {0};

static void process_command_line_arguments(int argc, char** argv) {
    if (argc <= 1) {
        usage();
        exit(EXIT_FAILURE);
    }

    int flag;
    struct option long_options[] = {
        // struct option
        // {
            // const char *name;
            // int         has_arg;
            // int        *flag;
            // int         val;
        // };
        // name: 选项名称
        // has_arg: 是否携带参数
        //     0: no_argument  无参数 --version --help
        //     1: required_argument 一个参数 --json piano.json or --json=piano.json
        //     2: optional_argument 只能按照指定格式输入 --json=piano.json
        // flag:
        //     空:
        //        当获得某个选项的时候，getopt_long返回val
        //        {"help", no_argument, 0, 'h'}, --help, getopt_long将会返回'h'
        //     非空:
        //        当获得某个长选项的时候，getopt_long将返回0，并将flag指向val。
        //        {"json", required_argument, &lopt, 'j'}，--json ./piano.json
        //        getopt_long将会返回0，同时lopt的值为j
        //
        // getopt_long返回值：
        // 1. 短选项找到，返回找到的字母
        // 2. 长选项找到：
        //    1. 如果flag为NULL，那么返回val
        //    2. 如果flag不为NULL，那么返回0，同时flag指向val
        // 3. 如果查询到的字符没有在短长中，返回?
        // 4. 结束，返回-1
        // {"file", required_argument, &flag, 'f'},
        // {"dir", required_argument, &flag, 'd'},
        // {"json", required_argument, &flag, 'j'},
        // {"log", required_argument, &flag, 'l'},

        {"file", required_argument, 0, 'f'},
        {"dir", required_argument, 0, 'd'},
        {"json", required_argument, 0, 'j'},
        {"log", required_argument, 0, 'l'},
        {"daemon", no_argument, 0, 'D'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "h?d:f:j:l:D", long_options, 0)) != -1) {
        switch (c) {
            case 'd':
                args.dir = std::string(optarg);
                if (args.flag != 0) goto failed;
                else args.flag = 'd';
                break;
            case 'f':
                args.wav_file = std::string(optarg);
                if (args.flag != 0) goto failed;
                else args.flag = 'f';
                break;
            case 'j':
                args.json = std::string(optarg);
                if (args.flag != 0) goto failed;
                else args.flag = 'j';
                break;
            case 'l':
                args.log = std::string(optarg);
                break;
            case 'D':
                args.daemon = true;
                break;
            case 'h':
            case '?':
            default:
                goto failed;
                break;
        }
    }

    if (args.flag == 0) goto failed;

    while (optind < argc)
        std::cout <<  "Non-option argument " << argv[optind++] << std::endl;

failed:
    usage();
    exit(EXIT_FAILURE);
}

#endif
