#ifndef __UTILS__
#define __UTILS__

#include <string>

/*代码注释 / Code Comments*/
/*CN 中文               English*/
/*Note: Translated using an online tool and some elbow grease,
 *so the translation may not be 100% accurate.
 */

extern void add_event_id(std::string);
extern void del_event_id(std::string);
extern bool event_id_exists(std::string);

extern void add_event_id(int);
extern void del_event_id(int);
extern bool event_id_exists(int);
extern bool is_event_id_lager_than_FILE_NUMS(int);
extern void clear_all_key_detect_threads();

// 判断是否为小端系统	Determine whether it is a little endian system
extern bool little_end();

#endif
