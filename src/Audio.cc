#include "utils.hpp"
#include "Audio.hpp"
#include "keycode.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>

extern "C" {
#include <sys/stat.h>
}

Audio::Audio(const std::string str, int flag): max_len(0), init_property(false) {
    init(str, flag);
}

Audio::Audio(const std::string str): max_len(0), init_property(false) {

    init(str);
}

Audio::Audio(const std::string str, uint16_t channels,
    uint32_t sample_rate, uint16_t bits_per_sample): max_len(0), init_property(true),
    channels(channels), sample_rate(sample_rate), bits_per_sample(bits_per_sample) {

    init(str);
}

void Audio::init(const std::string &str) {
    if (is_dir(str)) {
        from_dir(str);
    } else if (is_json(str)) {
        from_json(str);
    } else if (is_wav(str)) {
        from_file(str);
    } else {
        std::cout << "not json or wav or dir" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Audio::init(const std::string &str, int flag) {
    std::string err;
    switch (flag) {
        case 'd':
            if (is_dir(str)) from_dir(str);
            else {
                err = "please use directory";
                goto init_failed;
            }
            return;
        case 'f':
            if (is_wav(str)) from_file(str);
            else {
                err = "please write a wav file";
                goto init_failed;
            }
            return;
        case 'j':
            if (is_json(str)) from_json(str);
            else {
                err = "please use a json file";
                goto init_failed;
            }
            return;
        default:
            goto init_failed;
            break;
    }

init_failed:
    std::cout << err << std::endl;
    exit(EXIT_FAILURE);
}

void Audio::from_dir(const std::string &str) {
    bool has_file = false;
    // 目录
    for (int i = 0; i < 256; i++) {
        if (wav_datas[i].data) continue;
        if (read_wav(str + "/" + KEYS[i].first + ".wav", wav_datas[i])) {
            has_file = true;
            datas[KEYS[i].second] = i;
        }
        else {
            datas[KEYS[i].second] = 0;
        }
    }

    if (!has_file) {
        std::cout << "there's no file exist in " << str << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Audio::from_json(const std::string &str) {
    // json
    std::map<std::string, uint16_t> a_map;
    bool has_file = false;

    std::ifstream f(str);
    std::stringstream json_buf;
    json_buf << f.rdbuf();
    f.close();

    cJSON *root = cJSON_Parse(json_buf.str().c_str());

    if (!root) {
        std::cout << "parse json failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    cJSON *item;
    item = cJSON_GetObjectItem(root, "dir");
    if (item) {
        std::string wav_dir = std::string(item->valuestring);

        for (int i = 0; i < 256; i++) {
            item = cJSON_GetObjectItem(root, KEYS[i].first.c_str());

            if (!item) {
                datas[KEYS[i].second] = 0;
                continue;
            }

            std::string wav_name = std::string(item->valuestring);

            // 已经读取过该音频了
            if (a_map.count(wav_name) > 0) {
                datas[KEYS[i].second] = a_map[wav_name];
                continue;
            }

            // if (wav_datas[i].data) continue;

            if (read_wav(wav_dir + "/" + wav_name, wav_datas[i])) {
                datas[KEYS[i].second] = i;
                a_map[wav_name] = i;
                has_file = true;
            } else {
                datas[KEYS[i].second] = 0;
            }
        }
    } else {
        std::cout << "you must add a dir item in json" << std::endl;
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    if (!has_file) {
        std::cout << "no file read success" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Audio::from_file(const std::string &str) {
    // 单独的音乐文件
    if (read_wav(str, wav_datas[0])) {
        for (int i = 1; i < 256; i++) {
            datas[i] = 0;
        }
    } else {
        std::cout << "read " << str << "error" << std::endl;
        exit(EXIT_FAILURE);
    }
}

bool Audio::read_wav(const std::string &file, WAV_DATA& wav_data) {
    wav_data.data = nullptr;
    wav_data.len = 0;
    wav_data.bits_per_sample = 0;

    if (!file_exists(file)) return false;

    WAVE_HEADER header;
    std::ifstream f(file);
    if (f.is_open()) {
        f.read((char *)&header, sizeof(WAVE_HEADER));

        // 不是wav文件
        if (!tag_is_right(header.riff_id, header.riff_type)) goto failed;

        if (header.fmt_audio_format == 1) {
            if (!init_property) {
                channels = header.fmt_channels;
                sample_rate = header.fmt_sample_rate;
                bits_per_sample = header.fmt_bits_per_sample;
                init_property = true;
            } else if (header.fmt_channels != channels
                    || header.fmt_sample_rate != sample_rate
                    || header.fmt_bits_per_sample != bits_per_sample) {
                goto failed;
            }
        } else {
            std::cout << file << " has not pcm tag" << std::endl;
            goto failed;
        }

        wav_data.len = header.data_size;
        wav_data.bits_per_sample =  header.fmt_bits_per_sample;
        wav_data.data = new uint8_t[wav_data.len];

        f.read((char *)wav_data.data, wav_data.len);

        // 更新最大值
        if (wav_data.len > max_len) max_len = wav_data.len;

        f.close();
        return true;
    } // else goto failed;

failed:
    f.close();
    return false;
}

WAV_DATA Audio::get_wav_by_code(uint16_t code) {
    if (code > 255) {
        return {nullptr, 0, 0, 0};
    } else if (datas[code] > 255) {
        return {nullptr, 0, 0, 0};
    }
    return wav_datas[datas[code]];
}


Audio::~Audio() {
    for (int i = 0; i < 256; i++) {
        if (wav_datas[i].data) delete [] wav_datas[i].data;
    }
}

bool Audio::is_wav(const std::string &str) const {
    WAVE_HEADER header;
    std::ifstream f(str);

    if (f.is_open()) {
        f.read((char *)&header, sizeof(WAVE_HEADER));
        f.close();
        return tag_is_right(header.riff_id, header.riff_type);
    } else {
        f.close();
        return false;
    }
}

bool Audio::tag_is_right(const uint8_t *riff_id, const uint8_t *riff_type) const {
    return (riff_id[0] == 'R' && riff_id[1] == 'I' &&
           riff_id[2] == 'F' && riff_id[3] == 'F' &&
           riff_type[0] == 'W' && riff_type[1] == 'A' &&
           riff_type[2] == 'V' && riff_type[3] == 'E');
}
