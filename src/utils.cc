#include "utils.hpp"
#include <iostream>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <mutex>

#define FILE_NUMS 1024

static std::mutex mtx;
static std::map<std::string, std::string> key_detect_threads;
static char key_detect_threads_bool[FILE_NUMS] = {0};

// 增加一个设备
void add_event_id(std::string event_id) {
    mtx.lock();
    key_detect_threads[event_id] = event_id;
    mtx.unlock();
}

// 删除一个设备
void del_event_id(std::string event_id) {
    mtx.lock();
    key_detect_threads.erase(event_id);
    mtx.unlock();

}

// 判断设备是否已经存在
bool event_id_exists(std::string event_id) {
    if (key_detect_threads.count(event_id) > 0) return true;
    else return false;
}

///////////////////////////////////////////////////////
// 增加一个设备
void add_event_id(int event_id) {
    if (event_id > FILE_NUMS) {
        std::cout << "add error file is lager" << std::endl;
        return;
    }
    mtx.lock();
    key_detect_threads_bool[event_id] = true;
    mtx.unlock();
}

// 删除一个设备
void del_event_id(int event_id) {
    if (event_id > FILE_NUMS) {
        std::cout << "delete error file is lager" << std::endl;
        return;
    }
    mtx.lock();
    key_detect_threads_bool[event_id] = false;
    mtx.unlock();

}

// 判断设备是否已经存在
bool event_id_exists(int event_id) {
    if (event_id > FILE_NUMS) {
        std::cout << "file is lager" << std::endl;
        return false;
    }
    return key_detect_threads_bool[event_id];
}

bool is_event_id_lager_than_FILE_NUMS(int event_id) {
    if (event_id > FILE_NUMS) return true;
    else return false;
}

void clear_all_key_devices() {
    mtx.lock();
    std::memset(key_detect_threads_bool, 0, FILE_NUMS);
    mtx.unlock();
}
