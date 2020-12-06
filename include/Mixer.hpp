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
    std::mutex mtx;
    uint32_t buffer_len;
    uint8_t *buffer;
    uint8_t *buffer_start, *pos, *buffer_end, *end;
    uint32_t index_start, index_pos, index_end;

    // 缓存中是否还有数据
    bool has_data;

    void mix8(uint8_t *, uint32_t);
    void mix16(uint8_t *, uint32_t);
    void mix32(uint8_t *, uint32_t);
};

#endif
