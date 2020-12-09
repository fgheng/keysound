#include "utils.hpp"
#include "Audio.hpp"
#include "keycode.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <sys/stat.h>
}


Audio::Audio(const std::string str):max_len(0) {

    // 默认非dir即file
    struct stat s;
    stat(str.c_str(), &s);
    is_dir = S_ISDIR(s.st_mode);

    // 初始化头
    init_value(str);
    if (is_dir) {
        for (int i = 0; i < 256; i++) {
            if (read_wav(str + "/" + KEYS[i] + ".wav", wav_datas[i])) {
                datas[i] = i;
            }
            else {
                datas[i] = 0;
            }
        }
    } else {
        read_wav(str, wav_datas[0]);
        for (int i = 1; i < 256; i++) {
            datas[i] = 0;
        }
    }
}

Audio::Audio(const std::string str, uint16_t channels,
    uint32_t sample_rate, uint16_t bits_per_sample): max_len(0),
    channels(channels), sample_rate(sample_rate), bits_per_sample(bits_per_sample) {

    // 默认非dir即file
    struct stat s;
    stat(str.c_str(), &s);
    is_dir = S_ISDIR(s.st_mode);

    if (is_dir) {
        for (int i = 0; i < 256; i++) {
            if (read_wav(str + "/" + KEYS[i] + ".wav", wav_datas[i])) datas[i] = i;
            else datas[i] = 0;
        }
    } else {
        read_wav(str, wav_datas[0]);
        for (int i = 1; i < 256; i++) {
            datas[i] = 0;
        }
    }
}

void Audio::init_value(const std::string &str) {
    if (is_dir) {
        for (int i = 0; i < 256; i++) {
            std::string file = str + "/" + KEYS[i] + ".wav";
            if (file_exists(file)) {
                std::ifstream f(file);
                if (f.is_open()) {
                    f.read((char *)&wav_header, sizeof(WAVE_HEADER));

                    channels = wav_header.fmt_channels;
                    sample_rate = wav_header.fmt_sample_rate;
                    bits_per_sample = wav_header.fmt_bits_per_sample;
                }
                f.close();
                break;
            }
        }
    } else {
        if (file_exists(str)) {
            std::ifstream f(str);
            if (f.is_open()) {
                f.read((char *)&wav_header, sizeof(WAVE_HEADER));

                channels = wav_header.fmt_channels;
                sample_rate = wav_header.fmt_sample_rate;
                bits_per_sample = wav_header.fmt_bits_per_sample;
            }
            f.close();
        }
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

        if (header.fmt_audio_format == 1
            && header.fmt_channels == channels
            && header.fmt_sample_rate == sample_rate
            && header.fmt_bits_per_sample == bits_per_sample) {

            wav_data.len = header.data_size;
            wav_data.bits_per_sample =  header.fmt_bits_per_sample;
            wav_data.data = new uint8_t[wav_data.len];

            f.read((char *)wav_data.data, wav_data.len);

            // 更新最大值
            if (wav_data.len > max_len) max_len = wav_data.len;
        } else {
            f.close();
            return false;
        }
    }

    f.close();
    return true;
}

WAV_DATA Audio::get_wav_by_code(uint16_t code) {
    if (code > 255) {
        return {nullptr, 0};
    }
    return wav_datas[datas[code]];
}


Audio::~Audio() {
    for (int i = 0; i < 256; i++) {
        if (wav_datas[i].data) delete [] wav_datas[i].data;
    }
}
