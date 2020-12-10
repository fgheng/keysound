#ifndef __MIXER__
#define __MIXER__

#include <memory>
#include <mutex>

class Mixer {
    using call_back = void (*)(uint8_t *, uint32_t);
public:
    Mixer(uint32_t);
    Mixer(const Mixer &) = delete;
    Mixer(Mixer &&) = delete;

    Mixer &operator=(const Mixer &) = delete;
    Mixer &operator=(Mixer &&) = delete;

    // 混音
    void mix(uint8_t *, uint32_t, uint16_t);

    // 获取
    void get_mix(call_back func, uint32_t size);
    void get_mix(uint8_t *, uint32_t);

private:
    // 是否是小端模式
    bool little_end;

    std::mutex mtx;
    uint32_t buffer_len;
    uint8_t *buffer;
    uint8_t *buffer_start, *pos, *buffer_end;

    void mix8(uint8_t *, uint32_t);
    void mix16(uint8_t *, uint32_t);
    void mix32(uint8_t *, uint32_t);

    // 需要确认wav的data到底是uint还是int
    inline uint8_t mix_uint8(uint8_t, uint8_t);
    inline int8_t mix_int8(int8_t, int8_t);
    inline int16_t mix_int16(int16_t, int16_t);
    inline int32_t mix_int32(int32_t, int32_t);

    // 系统大小端检测，暂时先不使用，假定系统都是小端字节序
    void detect_mode() {
        // 判断系统的大小端
        uint16_t num = 0x1122;
        uint8_t *c = (uint8_t *)&num;
        if (*c == 0x22) {
            little_end = true;
        } else {
            little_end = false;
        }
    }
};

#endif
