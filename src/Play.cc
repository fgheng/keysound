#include "Play.hpp"
#include "Mixer.hpp"
#include <cstring>
#include <iostream>

extern "C" {
#include <unistd.h>
}

#ifdef __USE_SDL__
extern "C" {
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
}

#ifdef __SDL_CALL_BACK__
static void sdl_callback(void *userdata, uint8_t *stream, int len) {
    Mixer *mixer = (Mixer *)userdata;
    SDL_memset(stream, 0, len);
    mixer->get_mix(stream, len);
}
#endif

void play(Mixer &mixer, uint32_t fmt_size, uint16_t channels, uint32_t sample_rate, uint16_t bits_per_sample) {
    uint32_t sleep_time; // 播放一组buffer所需的时间，ms
    SDL_AudioDeviceID dev;
    SDL_AudioSpec spec;

    spec.freq = sample_rate; // 采样率
    switch (bits_per_sample) {
        case 8:
            spec.format = AUDIO_U8;
            break;
        case 16:
            // 小字端，AUDIO_S16LSB
            // 与系统一致，AUDIO_S16SYS
            spec.format = AUDIO_S16LSB;
            break;
        case 32:
            spec.format = AUDIO_S32LSB;
            break;
        default:
            spec.format = AUDIO_U8;
            break;
    }
    spec.silence = 0; // 静音值
    spec.channels = channels;
    // 缓冲区大小，单位帧
    // 帧frame的数量，每个帧的字节数量=通道*bits_per_sample/8
    spec.samples = 2048;
    spec.size = spec.samples * channels * bits_per_sample / 8; // 缓冲区大小，单位字节

#ifdef __SDL_CALL_BACK__
    spec.callback = sdl_callback;
    spec.userdata = (void *)&mixer;
#else
    spec.callback = 0;
    spec.userdata = 0;
    sleep_time = (spec.size * 1000) / (sample_rate * channels * bits_per_sample / 8);
#endif


    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        std::cout << "init sdl error" << std::endl;
        return;
    }

#ifdef __SDL_CALL_BACK__
    if (SDL_OpenAudio(&spec, NULL) != 0) {
        std::cout << "open device error" << std::endl;
        return;
    }
    SDL_PauseAudio(0);
#else
    dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (dev == 0) {
        std::cout << "open audio device error" << std::endl;
        return;
    }
    SDL_PauseAudioDevice(dev, 0);
    uint8_t buf[spec.size];
#endif

    while (true) {
#ifdef __SDL_CALL_BACK__
        SDL_Delay(1000);
#else
        SDL_memset(buf, 0, spec.size);
        mixer.get_mix(buf, spec.size);
        SDL_QueueAudio(dev, buf, spec.size);
        // SDL_Delay(sleep_time);
        usleep(sleep_time*1000);
#endif
    }

#ifndef __SDL_CALL_BACK__
    SDL_CloseAudioDevice(dev);
#endif
    SDL_Quit();
}

#endif

#ifdef __USE_ALSA__
extern "C" {
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
}

static snd_pcm_t * gp_handle;
static uint32_t buffer_size;
static snd_pcm_uframes_t frames;

/**
 * @brief 初始化硬件
 *
 * @param fmt_size
 * @param channels 通道数量
 * @param sample_rate 采样率
 * @param bits_per_sample 每个样本点所需的bit数量
 */
static int alsa_init(uint32_t fmt_size, uint16_t channels,
        uint32_t sample_rate, uint16_t bits_per_sample) {

    int rc;
    snd_pcm_hw_params_t *gp_params; // 设置流的硬件参数

    // 打开pcm设备
    rc = snd_pcm_open(&gp_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        std::cout << "unale to open pcm device" << std::endl;
        return -1;
    }

    // 为参数分配空间
    snd_pcm_hw_params_alloca(&gp_params);

    // 填充默认值
    rc = snd_pcm_hw_params_any(gp_handle, gp_params);
    if (rc < 0) {
        std::cout << "unable to fill it in with default values." << std::endl;
        return -1;
    }

    // 交错模式
    rc = snd_pcm_hw_params_set_access(gp_handle, gp_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        std::cout << "unable set interleaved mode" << std::endl;
        return -1;
    }

    // 设置格式
    snd_pcm_format_t format;
    switch (fmt_size) {
        case 8:
            format = SND_PCM_FORMAT_U8;
            break;
        case 16:
            format = SND_PCM_FORMAT_S16_LE;
            break;
        case 24:
            format = SND_PCM_FORMAT_U24_LE;
            break;
        case 32:
            format = SND_PCM_FORMAT_U32_LE;
            break;
        default:
            format = SND_PCM_FORMAT_UNKNOWN;
            break;
    }
    rc = snd_pcm_hw_params_set_format(gp_handle, gp_params, format);
    if (rc < 0) {
        std::cout << "unable to set format" << std::endl;
        return -1;
    }

    // 设置通道
    rc = snd_pcm_hw_params_set_channels(gp_handle, gp_params, channels);
    if (rc < 0) {
        std::cout << "unable set channels" << std::endl;
        return -1;
    }

    // 设置采样率
    int dir;
    rc = snd_pcm_hw_params_set_rate_near(gp_handle, gp_params, &sample_rate, &dir);
    if (rc < 0) {
        std::cout << "unable set samples" << std::endl;
        return -1;
    }

    // 将参数写入驱动
    rc = snd_pcm_hw_params(gp_handle, gp_params);
    if (rc < 0) {
        std::cout << "unable to set hw parameters" << std::endl;
        return -1;
    }

    // 一个DMA有几个frame
    snd_pcm_uframes_t buffer_size_tmp;
    snd_pcm_hw_params_get_period_size(gp_params, &frames, &dir);
    snd_pcm_hw_params_get_buffer_size(gp_params, &buffer_size_tmp);

    if (frames == buffer_size_tmp) {
        std::cout << "can not use period equal to buffer size" << std::endl;
        return -1;
    }

    buffer_size = frames * channels * bits_per_sample / 8;

    return 0;
}

// static void play_back(uint8_t *stream, uint32_t size) {
    // int ret = snd_pcm_writei(gp_handle, stream, size);
    // usleep(sleep_time);
// }

void play(Mixer &mixer, uint32_t fmt_size, uint16_t channels, uint32_t sample_rate, uint16_t bits_per_sample) {

    int ret = alsa_init(fmt_size, channels, sample_rate, bits_per_sample);
    if (ret < 0) {
        std::cout << "error in play" << std::endl;
        return;
    }

    uint32_t sleep_time = buffer_size * 1000 / (sample_rate * channels * bits_per_sample / 8);
    uint8_t buf[buffer_size];
    while (true) {
        std::memset(buf, 0, buffer_size);

        // mixer.get_mix(play_back, buffer_size);
        mixer.get_mix(buf, buffer_size);

        std::cout << "in while" << std::endl;
        ret = snd_pcm_writei(gp_handle, buf, frames);
        if (ret == -EPIPE || ret < 0) {
            std::cout << "underrun occured or ret < 0" << std::endl;
            continue;
        }
        usleep(sleep_time*1000);
    }

    // 这里需要重新写一下whie的判断条件
    // ctrl c的时候结束
    // ctrl c统一个函数，该函数结束所有各种各样的线程
    snd_pcm_drain(gp_handle);
    snd_pcm_close(gp_handle);
}

#endif

#ifdef __USE_PULSE__
extern "C" {
#include <pulse/simple.h>
}

void play(Mixer &mixer, uint32_t fmt_size, uint16_t channels,
        uint32_t sample_rate, uint16_t bits_per_sample) {

    pa_simple *s;
    pa_sample_spec ss;

    switch (fmt_size) {
        case 8:
            ss.format = PA_SAMPLE_U8;
            break;
        case 16:
            // wav的数据是小字端存储的
            ss.format = PA_SAMPLE_S16LE;
            break;
        case 24:
            // 有符号?
            ss.format = PA_SAMPLE_S24LE;
            break;
        case 32:
            // 有符号?
            ss.format = PA_SAMPLE_S32LE;
            break;
        default:
            ss.format = PA_SAMPLE_INVALID;
            break;
    }
    ss.channels = channels;
    ss.rate = sample_rate;

    s = pa_simple_new(NULL, "keysound", PA_STREAM_PLAYBACK,
                    NULL, "music", &ss, NULL, NULL, NULL);

    if (!s) {
        std::cout << "error occured while connect audio" << std::endl;
        return;
    }

    // frame的数量
    uint16_t frame_num = 2048;
    // 每个frame拥有的字节数
    uint16_t frame_bytes = channels * bits_per_sample / 8;
    // 一个buffer持续的时间
    uint32_t sleep_time = (frame_num * frame_bytes * 1000) /
        (sample_rate * channels * bits_per_sample / 8);

    int error, ret;
    uint8_t buf[frame_num * frame_bytes];
    while (true) {
        std::memset(buf, 0, frame_num * frame_bytes);
        mixer.get_mix(buf, frame_num * frame_bytes);
        ret = pa_simple_write(s, buf, frame_num * frame_bytes, &error);
        if (ret < 0) {
            std::cout << "pulse write error" << std::endl;
            continue;
        }

        // 等待播放完毕
        // ret = pa_simple_drain(s, &error);
        // if (ret < 0) {
            // std::cout << "error occured" << std::endl;
            // continue;
        // }
        usleep(sleep_time*1000);
    }

    pa_simple_free(s);
}
#endif
