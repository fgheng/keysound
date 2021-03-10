#include "Mixer.hpp"
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>

extern "C" {
#include <unistd.h>
}


/*代码注释 / Code Comments*/
/*CN 中文               English*/
/*Note: Translated using an online tool and some elbow grease,
 *so the translation may not be 100% accurate.
 */



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

// 进行混音，从当前的pos位置开始即可			To mix, start from the current "pos" position
void Mixer::mix(uint8_t *buf, uint32_t size, uint16_t bits_per_sample) {
    // 如果长度大于buffer_len，那么后面的应该截取掉，	If the length is greater than buffer_len, then the latter should be intercepted,
    // 虽然不太可能出现这种情况，但还是加上判断吧	although this is unlikely  to happen, let's be sure.
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
    if (buffer_end - pos < size) {
        // 传入的size要比pos 到 end大		The incoming size is larger than "pos" to "end"
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

    // 传入的buffer比较大， 超过了剩余的内存		The incoming buffer is relatively large and exceeds the remaining memory
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
    // 传入的buffer比较大，		The incoming buffer is relatively large,
    // 超过了剩余的内存			Exceeded the remaining memory
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

void Mixer::get_mix(call_back func, uint32_t size) {
    // 取数据，pos会移动		Fetch data, pos will move
    mtx.lock();

    uint32_t buf_pos = 0;
    // uint32_t need = size;

    // 所需大于0		Need to be greater than 0
    while (size - buf_pos > 0) {
        if (size - buf_pos > buffer_end - pos) {
            // 所需大于mixer的buffer的当前位置到结尾	The current position to the end of the buffer that needs to be larger than the mixer
            func(pos, buffer_end - pos);
            // 拷贝过后的数据归0			Return the data after copying to 0
            std::memset(pos, 0, buffer_end-pos);
            buf_pos += buffer_end - pos;
            pos = buffer_start;
        } else {
            // 所需小于mixer的buffer当前位置到结尾	The current position of the buffer that needs to be smaller than the mixer to the end
            func(pos, size - buf_pos);
            std::memset(pos, 0, size - buf_pos);
            pos += size - buf_pos;

            // buf_pos += (size - buf_pos);
            break;
        }
    }

    // // 请求的数据块大于剩余的数据块，正常应该不会出现这种情况	The requested data block is larger than the remaining data blocks, normally this should not happen
    // if (size > buffer_end - pos) {
        // func(pos, buffer_end - pos);
        // std::memset(pos, 0, buffer_end-pos);

        // uint32_t left = size - (buffer_end - pos);
        // pos = buffer_start;
        // func(pos, left);
        // std::memset(pos, 0, left);
        // pos += left;

    // } else {
        // func(pos, size);
        // std::memset(pos, 0, size);
        // pos += size;
    // }
    mtx.unlock();
}

// 向buf拷贝size个数据		Copy size data to buf
void Mixer::get_mix(uint8_t *buf, uint32_t size) {
    // 取数据，pos会移动	Fetch data, pos will move
    mtx.lock();

    uint32_t buf_pos = 0;
    // uint32_t need = size;

    while (size - buf_pos > 0) {
        if (size - buf_pos > buffer_end - pos) {
            std::memcpy(buf + buf_pos, pos, buffer_end - pos);
            // 拷贝过后的数据归0	Return the data after copying to 0
            std::memset(pos, 0, buffer_end-pos);
            buf_pos += buffer_end - pos;
            pos = buffer_start;
        } else {
            std::memcpy(buf + buf_pos, pos, size - buf_pos);
            std::memset(pos, 0, size - buf_pos);
            pos += size - buf_pos;

            // buf_pos += (size - buf_pos);
            break;
        }
    }

    // // 请求的数据块大于剩余的数据块		The requested data block is larger than the remaining data block
    // if (size > buffer_end - pos) {
        // std::cout << "---------------------" << std::endl;
        // std::cout << "pos start " << pos - buffer_start << std::endl;
        // std::cout << "size " << size << std::endl;
        // std::cout << "buffer_end - pos " << buffer_end - pos << std::endl;
        // std::memcpy(buf, pos, buffer_end - pos);
        // // 拷贝过后的数据归0			Return the data after copying to 0
        // std::memset(pos, 0, buffer_end-pos);

        // uint32_t left = size - (buffer_end - pos);
        // std::cout << "left is " << size - (buffer_end - pos) << std::endl;
        // std::memcpy(buf + (buffer_end - pos), buffer_start, left);
        // std::cout << "buf + (bufend - pos) " << (buffer_end - pos) << std::endl;
        // std::memset(buffer_start, 0, left);

        // pos = buffer_start + left;
        // std::cout << "pos end " << pos - buffer_start << std::endl;

    // } else {
        // std::memcpy(buf, pos, size);
        // std::memset(pos, 0, size);
        // pos += size;
    // }
    mtx.unlock();
}

inline uint8_t Mixer::mix_uint8(uint8_t A, uint8_t B) {
    return 0;
}

inline int8_t Mixer::mix_int8(int8_t A, int8_t B) {
    int16_t A1 = static_cast<int16_t>(A);
    int16_t B1 = static_cast<int16_t>(B);
    int16_t C = A1 + B1 - (A1 * B1 >> 0x08);

    if (C > 127) C = 127;
    else if (C < -128) C = -128;

    return static_cast<int8_t>(C);
}

inline int16_t Mixer::mix_int16(int16_t A, int16_t B) {
    // 应该改为内联汇编，使用汇编判断溢出位				Should be changed to inline assembly, use assembly to determine the overflow bit
    // uint16_t C = A + B
    // 如果溢出，C = xxx						If it overflows, C = xxx
    // 该函数应该改为右值引用?						Should this function be changed to an rvalue reference?

    // 参考 http://blog.sina.com.cn/s/blog_4d61a7570101arsr.html	Reference: http://blog.sina.com.cn/s/blog_4d61a7570101arsr.html
    int32_t A1 = static_cast<int32_t>(A);
    int32_t B1 = static_cast<int32_t>(B);
    int32_t C = A1 + B1 - (A1 * B1 >> 0x10);

    if (C > 32767) C = 32750;
    else if (C < -32768) C = -32750;

    return static_cast<int16_t>(C);
}

inline int32_t Mixer::mix_int32(int32_t A, int32_t B) {
    int64_t A1 = static_cast<int64_t>(A);
    int64_t B1 = static_cast<int64_t>(B);
    int64_t C = A1 + B1 - (A1 * B1 >> 0x20);

    if (C > 2147483647) C = 2147483647;
    else if (C < 2147483648) C = -2147483648;

    return static_cast<int32_t>(C);
}
