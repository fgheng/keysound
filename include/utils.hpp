#ifndef __UTILS__
#define __UTILS__

#include <string>

extern void add_event_id(std::string);
extern void del_event_id(std::string);
extern bool event_id_exists(std::string);

extern void add_event_id(int);
extern void del_event_id(int);
extern bool event_id_exists(int);
extern bool is_event_id_lager_than_FILE_NUMS(int);
extern void clear_all_key_devices();

// 判断是否为小端系统
extern bool little_end();

#endif
