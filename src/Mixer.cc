#include "Mixer.hpp"
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>

extern "C" {
#include <SDL2/SDL_audio.h>
#include <unistd.h>
}

Mixer::Mixer(uint32_t buffer_len) : buffer_len(buffer_len) {
    if (buffer_len == 0) {
        buffer = nullptr;
        pos = buffer;
        buffer_start = buffer;
        buffer_end = buffer;
        std::cout << "buffer_len is 0" << std::endl;
        return;
    }

    buffer = new uint8_t[buffer_len];
    pos = buffer;
    buffer_start = buffer;
    buffer_end = buffer_start + buffer_len;
}

// 进行混音，从当前的pos位置开始即可
void Mixer::mix(uint8_t *buf, uint32_t size, uint16_t bits_per_sample) {
    // 如果长度大于buffer_len，那么后面的应该截取掉，
    // 虽然不太可能出现这种情况，但还是加上判断吧
    if (buf == nullptr || buffer_len == 0) return;
    if (size > buffer_len) size = buffer_len;

    switch (bits_per_sample) {
        case 8:
            mix8(buf, size);
            break;
        case 16:
            mix16(buf, size);
            break;
        case 32:
            mix32(buf, size);
            break;
        default:
            break;
    }
}

void Mixer::mix8(uint8_t *buf, uint32_t size) {
    // 增加数据end会移动
    if (buffer_end - pos < size) {
        // 传入的size要比pos 到 end大
        mtx.lock();
        for (int i = 0; i < buffer_end - pos; i++) {
            uint8_t A = *(pos + i);
            uint8_t B = *(buf + i);
            *(pos+i) = mix_uint8(A, B);
        }

        for (int i = 0; i < size - (buffer_end - pos); i++) {
            uint8_t A = *(buffer_start + i);
            uint8_t B = *(buf + (buffer_end - pos) + i);
            *(buffer_start+i) = mix_uint8(A, B);
        }

        mtx.unlock();
    } else {
        mtx.lock();
        for (int i = 0; i < size; i++) {
            uint8_t A = *(pos + i);
            uint8_t B = *(buf + i);
            *(pos+i) = mix_uint8(A, B);
        }
        mtx.unlock();
    }
}

void Mixer::mix16(uint8_t *buf, uint32_t size) {
    // 16位的要变成shot进行混音
    // 保存的时候要拆开保存，还要交换下位置???

    // 传入的buffer比较大，
    // 超过了剩余的内存
    if (buffer_end - pos < size) {
        mtx.lock();
        for (int i = 0; i < (buffer_end-pos) / 2; i++) {
            int16_t A = *((int16_t *)pos + i);
            int16_t B = *((int16_t *)buf + i);
            *((int16_t *)pos + i) = mix_int16(A, B);
        }

        for (int i = 0; i < (size-(buffer_end-pos)) / 2; i++) {
            int16_t A = *((int16_t *)buffer_start + i);
            int16_t B = *((int16_t *)(buf + (buffer_end - pos)) + i);

            *((int16_t *)buffer_start + i) = mix_int16(A, B);
        }

        mtx.unlock();
    } else {
        mtx.lock();
        for (int i = 0; i < size / 2; i++) {
            int16_t A = *((int16_t *)pos + i);
            int16_t B = *((int16_t *)buf + i);
            *((int16_t *)pos + i) = mix_int16(A, B);
        }

        mtx.unlock();
    }
}

void Mixer::mix32(uint8_t *buf, uint32_t size) {
    // 传入的buffer比较大，
    // 超过了剩余的内存
    if (buffer_end - pos < size) {
        mtx.lock();
        for (int i = 0; i < (buffer_end-pos) / 4; i++) {
            int32_t A = *((int32_t *)pos + i);
            int32_t B = *((int32_t *)buf + i);
            *((int32_t *)pos + i) = mix_int32(A, B);
        }

        for (int i = 0; i < (size-(buffer_end-pos)) / 4; i++) {
            int32_t A = *((int32_t *)buffer_start + i);
            int32_t B = *((int32_t *)(buf + (buffer_end - pos)) + i);

            *((int32_t *)buffer_start + i) = mix_int32(A, B);
        }

        mtx.unlock();
    } else {
        mtx.lock();
        for (int i = 0; i < size / 4; i++) {
            int32_t A = *((int32_t *)pos + i);
            int32_t B = *((int32_t *)buf + i);
            *((int32_t *)pos + i) = mix_int32(A, B);
        }

        mtx.unlock();
    }
}

// 需要考虑一下
void Mixer::get_mix(call_back func, uint32_t size) {
    // 取数据，pos会移动
    mtx.lock();

    // 请求的数据块大于剩余的数据块，正常应该不会出现这种情况
    // 所以这里应该重写，不需要这个，即使出现了，应该设置大小为到最后的值即可
    if (size > buffer_end - pos) {
        func(pos, buffer_end - pos);
        std::memset(pos, 0, buffer_end-pos);

        uint32_t left = size - (buffer_end - pos);
        pos = buffer_start;
        func(pos, left);
        std::memset(pos, 0, left);
        pos += left;

    } else {
        func(pos, size);
        std::memset(pos, 0, size);
        pos += size;
    }
    mtx.unlock();
}

// 向buf拷贝size个数据
void Mixer::get_mix(uint8_t *buf, uint32_t size) {
    // 取数据，pos会移动
    mtx.lock();

    // 请求的数据块大于剩余的数据块
    if (size > buffer_end - pos) {
        std::memcpy(buf, pos, buffer_end - pos);
        // 归0
        std::memset(pos, 0, buffer_end-pos);

        uint32_t left = size - (buffer_end - pos);
        std::memcpy(buf + (buffer_end - pos), buffer_start, left);
        std::memset(buffer_start, 0, left);

        pos = buffer_start + left;

    } else {
        std::memcpy(buf, pos, size);
        std::memset(pos, 0, size);
        pos += size;
    }
    mtx.unlock();
}

uint8_t Mixer::mix_uint8(uint8_t A, uint8_t B) {
    return 0;
}

int8_t Mixer::mix_int8(int8_t A, int8_t B) {
    int16_t A1 = static_cast<int16_t>(A);
    int16_t B1 = static_cast<int16_t>(B);
    int16_t C = A1 + B1 - (A1 * B1 >> 0x08);

    if (C > 127) C = 127;
    else if (C < -128) C = -128;

    return static_cast<int8_t>(C);
}

int16_t Mixer::mix_int16(int16_t A, int16_t B) {

    // 参考http://blog.sina.com.cn/s/blog_4d61a7570101arsr.html
    int32_t A1 = static_cast<int32_t>(A);
    int32_t B1 = static_cast<int32_t>(B);
    int32_t C = A1 + B1 - (A1 * B1 >> 0x10);

    if (C > 32767) C = 32760;
    else if (C < -32768) C = -32760;

    return static_cast<int16_t>(C);
}

int32_t Mixer::mix_int32(int32_t A, int32_t B) {
    int64_t A1 = static_cast<int64_t>(A);
    int64_t B1 = static_cast<int64_t>(B);
    int64_t C = A1 + B1 - (A1 * B1 >> 0x20);

    if (C > 2147483647) C = 2147483647;
    else if (C < 2147483648) C = -2147483648;

    return static_cast<int32_t>(C);
}
