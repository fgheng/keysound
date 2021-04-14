#include "utils.hpp"
#include <iostream>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <mutex>



/*代码注释 / Code Comments*/
/*CN 中文               English*/
/*Note: Translated using an online tool and some elbow grease,
 *so the translation may not be 100% accurate.
 */



#define FILE_NUMS 1024

static std::mutex mtx;
static std::map<std::string, std::string> key_detects;
static char key_detects_bool[FILE_NUMS] = {0};

// 增加一个设备		Add a device
void add_event_id(std::string event_id) {
    mtx.lock();
    key_detects[event_id] = event_id;
    mtx.unlock();
}

// 删除一个设备		Delete a device
void del_event_id(std::string event_id) {
    mtx.lock();
    key_detects.erase(event_id);
    mtx.unlock();

}

// 判断设备是否已经存在		Determine whether the device already exists
bool event_id_exists(std::string event_id) {
    if (key_detects.count(event_id) > 0) return true;
    else return false;
}

///////////////////////////////////////////////////////
// 增加一个设备		Add a device
void add_event_id(int event_id) {
    if (event_id > FILE_NUMS) {
        std::cout << "add error file is lager" << std::endl;
        return;
    }
    mtx.lock();
    key_detects_bool[event_id] = true;
    mtx.unlock();
}

// 删除一个设备		Delete a device
void del_event_id(int event_id) {
    if (event_id > FILE_NUMS) {
        std::cout << "delete error file is lager" << std::endl;
        return;
    }
    mtx.lock();
    key_detects_bool[event_id] = false;
    mtx.unlock();
}

// 判断设备是否已经存在		Determine whether the device already exists
bool event_id_exists(int event_id) {
    if (event_id > FILE_NUMS) {
        std::cout << "file is lager" << std::endl;
        return false;
    }
    return key_detects_bool[event_id];
}

bool is_event_id_lager_than_FILE_NUMS(int event_id) {
    if (event_id > FILE_NUMS) return true;
    else return false;
}

void clear_all_key_detect_threads() {
    mtx.lock();
    // std::memset(key_detects_bool, 0, FILE_NUMS);
    // key_detects.erase(key_detects.begin(), key_detects.end());
    key_detects.clear();
    mtx.unlock();
}

bool little_end() {
    uint16_t num = 0x1122;
    uint8_t *c = (uint8_t *)&num;
    if (*c == 0x22) {
        return true;
    } else {
        return false;
    }
}
