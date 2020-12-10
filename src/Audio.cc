#include "utils.hpp"
#include "Audio.hpp"
#include "keycode.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>

extern "C" {
#include <sys/stat.h>
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
        // 目录
        for (int i = 0; i < 256; i++) {
            if (wav_datas[i].data) continue;
            if (read_wav(str + "/" + KEYS[i].first + ".wav", wav_datas[i])) {
                datas[KEYS[i].second] = i;
            }
            else {
                datas[KEYS[i].second] = 0;
            }
        }
    } else if (is_json(str)) {
        // json
        std::ifstream f(str);
        std::stringstream json_buf;
        json_buf << f.rdbuf();
        f.close();

        cJSON *root = cJSON_Parse(json_buf.str().c_str());

        if (!root) {
            std::cout << "read json file error" << std::endl;
            return;
        }

        cJSON *item;
        item = cJSON_GetObjectItem(root, "dir");
        if (item) {
            std::string wav_dir = std::string(item->valuestring);

            for (int i = 0; i < 256; i++) {
                item = cJSON_GetObjectItem(root, KEYS[i].first.c_str());

                if (!item) continue;

                std::string wav_name = std::string(item->valuestring);

                if (wav_datas[i].data) continue;

                if (read_wav(wav_dir + "/" + wav_name, wav_datas[i])) {
                    datas[KEYS[i].second] = i;
                } else {
                    datas[KEYS[i].second] = 0;
                }
            }
        }
        cJSON_Delete(root);
    } else if (is_wav(str)) {
        // 单独的音乐文件
        read_wav(str, wav_datas[0]);
        for (int i = 1; i < 256; i++) {
            datas[i] = 0;
        }
    }
}

bool Audio::read_wav(const std::string &file, WAV_DATA& wav_data) {
    wav_data.data = nullptr;
    wav_data.len = 0;
    wav_data.bits_per_sample = 0;

    if (!file_exists(file)) {
        return false;
    }

    WAVE_HEADER header;
    std::ifstream f(file);
    if (f.is_open()) {
        f.read((char *)&header, sizeof(WAVE_HEADER));

        if (header.fmt_audio_format == 1) {
            if (!init_property) {
                channels = header.fmt_channels;
                sample_rate = header.fmt_sample_rate;
                bits_per_sample = header.fmt_bits_per_sample;
                init_property = true;
            } else if (header.fmt_channels != channels
                    || header.fmt_sample_rate != sample_rate
                    || header.fmt_bits_per_sample != bits_per_sample) {
                f.close();
                return false;
            }
        } else {
            f.close();
            return false;
        }

        wav_data.len = header.data_size;
        wav_data.bits_per_sample =  header.fmt_bits_per_sample;
        wav_data.data = new uint8_t[wav_data.len];

        f.read((char *)wav_data.data, wav_data.len);

        // 更新最大值
        if (wav_data.len > max_len) max_len = wav_data.len;
    }

    f.close();
    return true;
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
