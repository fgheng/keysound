#include "Mixer.hpp"
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>

extern "C" {
#include <SDL2/SDL_audio.h>
#include <unistd.h>
}

Mixer::Mixer(uint32_t buffer_len) : buffer_len(buffer_len), has_data(false) {
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

    index_pos = 0;
    index_start = 0;
    index_end = index_start + buffer_len;
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
    // 有数据，stop_get为假
    if (buffer_end - pos < size) {
        // 传入的size要比pos 到 end大
        mtx.lock();
        for (int i = 0; i < buffer_end - pos; i++) {
            uint8_t A = *(pos + i);
            uint8_t B = *(buf + i);
            *(pos+i) = (A + B) - (A * B >> 16);
        }

        for (int i = 0; i < size - (buffer_end - pos); i++) {
            uint8_t A = *(buffer_start + i);
            uint8_t B = *(buf + (buffer_end - pos) + i);
            *(buffer_start+i) = (A + B) - (A * B >> 16);
        }

        auto left = size - (buffer_end - pos);
        end = buffer_start + left > end ? buffer_start + left : end;

        // 有新数据了，可以获取
        has_data = true;
        mtx.unlock();
    } else {
        mtx.lock();
        for (int i = 0; i < size; i++) {
            uint8_t A = *(pos + i);
            uint8_t B = *(buf + i);
            *(pos+i) = (A + B) - (A * B >> 16);
        }
        end = pos + size > end ? pos + size : end;

        // 有新数据了，可以获取
        has_data = true;
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
            uint16_t A = *((uint16_t *)pos + i);
            uint16_t B = *((uint16_t *)buf + i);
            *((uint16_t *)pos + i) = (A + B) - (A * B >> 16);
        }

        for (int i = 0; i < (size-(buffer_end-pos)) / 2; i++) {
            uint16_t A = *((uint16_t *)buffer_start + i);
            uint16_t B = *((uint16_t *)(buf + (buffer_end - pos)) + i);

            *((uint16_t *)buffer_start + i) = (A + B) - (A * B >> 16);
        }

        auto left = size - (buffer_end - pos);
        end = buffer_start + left > end ? buffer_start + left : end;

        has_data = true;

        mtx.unlock();
    } else {
        mtx.lock();
        for (int i = 0; i < size / 2; i++) {
            uint16_t A = *((uint16_t *)pos + i);
            uint16_t B = *((uint16_t *)buf + i);
            *((uint16_t *)pos+i) = (A + B) - (A * B >> 16);
        }

        end = pos + size > end ? pos + size : end;

        has_data = true;

        mtx.unlock();
    }
}

void Mixer::mix32(uint8_t *buf, uint32_t size) {
    // 传入的buffer比较大，
    // 超过了剩余的内存
    if (buffer_end - pos < size) {
        mtx.lock();
        for (int i = 0; i < (buffer_end-pos) / 4; i++) {
            uint32_t A = *((uint32_t *)pos + i);
            uint32_t B = *((uint32_t *)buf + i);
            *((uint32_t *)pos + i) = (A + B) - (A * B >> 32);
        }

        for (int i = 0; i < (size-(buffer_end-pos)) / 4; i++) {
            uint32_t A = *((uint32_t *)buffer_start + i);
            uint32_t B = *((uint32_t *)(buf + (buffer_end - pos)) + i);

            *((uint32_t *)buffer_start + i) = (A + B) - (A * B >> 32);
        }

        auto left = size - (buffer_end - pos);
        end = buffer_start + left > end ? buffer_start + left : end;

        has_data = true;

        mtx.unlock();
    } else {
        mtx.lock();
        for (int i = 0; i < size / 4; i++) {
            uint32_t A = *((uint32_t *)pos + i);
            uint32_t B = *((uint32_t *)buf + i);
            *((uint32_t *)pos+i) = (A + B) - (A * B >> 32);
        }

        end = pos + size > end ? pos + size : end;

        has_data = true;

        mtx.unlock();
    }
}

// 需要考虑一下
void Mixer::get_mix(call_back func, uint32_t size) {
    if (!has_data || buffer_len == 0) {
        return;
    }

    // 取数据，pos会移动
    mtx.lock();

    // 请求的数据块大于剩余的数据块，正常应该不会出现这种情况
    // 所以这里应该重写，不需要这个，即使出现了，应该设置大小为到最后的值即可
    if (size > buffer_end - pos) {
        func(pos, buffer_end - pos);
        // 归0
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

    // 判断是否结束，没有数据了，可以暂停
    if (pos >= end) {
        // 结束了，应该暂停播放
        // 不是暂停播放，而是停止向队列中增加数据
        has_data = false;
    }
    mtx.unlock();
}

// 向buf拷贝size个数据
void Mixer::get_mix(uint8_t *buf, uint32_t size) {
    if (!has_data || buffer_len == 0) {
        return;
    }

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

    // 判断是否结束，没有数据了，可以暂停
    if (pos >= end) {
        // 结束了，应该暂停播放
        // 不是暂停播放，而是停止向队列中增加数据
        has_data = false;
    }
    mtx.unlock();
}
