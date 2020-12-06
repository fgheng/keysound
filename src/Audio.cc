#include "utils.hpp"
#include "Audio.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>


// type = VARIETY多样表示钢琴，也就是str是一个目录
// type = SOLE单一表示所有的按键使用一个声音，也就是str是一个文件
Audio::Audio(std::string str, int type):max_len(0), type(type) {
    init_value(str, type);
    if (type == VARIETY) {
        for (int i = 0; i < 128; i++) {
            read_wav(str + "/" + std::to_string(i) + ".wav", datas[i]);
        }
    } else if (type == SOLE) {
        read_wav(str, datas[0]);
        for (int i = 1; i < 128; i++) {
            datas[i].data = datas[0].data;
            datas[i].len = datas[0].len;
            datas[i].bits_per_sample = datas[0].bits_per_sample;
        }
    }
}

Audio::Audio(std::string str, int type, uint16_t channels,
    uint32_t sample_rate, uint16_t bits_per_sample): max_len(0), type(type),
    channels(channels), sample_rate(sample_rate), bits_per_sample(bits_per_sample) {
    if (type == VARIETY) {
        for (int i = 0; i < 128; i++) {
            read_wav(str + "/" + std::to_string(i) + ".wav", datas[i]);
        }
    } else if (type == SOLE) {
        read_wav(str, datas[0]);
        for (int i = 1; i < 128; i++) {
            datas[i].data = datas[0].data;
            datas[i].len = datas[0].len;
            datas[i].bits_per_sample = datas[0].bits_per_sample;
        }
    }
}

void Audio::init_value(const std::string &str, int type) {
    if (type == VARIETY) {
        for (int i = 0; i < 128; i++) {
            std::string file = str + "/" + std::to_string(i) + ".wav";
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

void Audio::read_wav(const std::string &file, WAV_DATA& wav_data) {
    wav_data.data = nullptr;
    wav_data.len = 0;
    wav_data.bits_per_sample = 0;

    if (!file_exists(file)) return;

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
        }
    }

    f.close();
}

WAV_DATA Audio::get_wav_by_code(uint16_t code) {
    if (code >= 128) {
        return {nullptr, 0};
    }
    return datas[code];
}


Audio::~Audio() {
    if (type == VARIETY)
        for (int i = 0; i < 128; i++) {
            if (datas[i].data) delete [] datas[i].data;
        }
    else
        delete [] datas[0].data;
}
