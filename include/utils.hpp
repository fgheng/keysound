#ifndef __UTILS__
#define __UTILS__

#include <map>
#include <memory>
#include <string>
#include <mutex>

// #define MSG_QUEUE_FILE "/tmp/keysound_queue"
// #define MSG_KEY 1024
// struct msgstru {
    // long type;
    // char data[sizeof(long)];
// };

#define FILE_NUMS 1024

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_REPEAT 2

// 全局锁
static std::mutex mtx;
static std::map<std::string, std::string> key_detect_threads;
static char key_detect_threads_bool[FILE_NUMS] = {0};

// extern void add_event_id(std::string);
// extern void del_event_id(std::string);
// extern bool event_id_exists(std::string);

extern void add_event_id(int);
extern void del_event_id(int);
extern bool event_id_exists(int);
extern bool is_event_id_lager_than_FILE_NUMS(int);
extern void clear_all_key_detect();


#endif
