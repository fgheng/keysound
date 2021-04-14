#ifndef __MIXER__
#define __MIXER__

#include <memory>
#include <mutex>

/*代码注释 / Code Comments*/
/*CN 中文               English*/
/*Note: Translated using an online tool and some elbow grease,
 *so the translation may not be 100% accurate.
 */

class Mixer {
    using call_back = void (*)(uint8_t *, uint32_t);
public:
    Mixer(uint32_t);
    Mixer(const Mixer &) = delete;
    Mixer(Mixer &&) = delete;

    Mixer &operator=(const Mixer &) = delete;
    Mixer &operator=(Mixer &&) = delete;

    // 混音	Mix
    void mix(uint8_t *, uint32_t, uint16_t);

    // 获取	Get the mix
    void get_mix(call_back func, uint32_t size);
    void get_mix(uint8_t *, uint32_t);

private:
    std::mutex mtx;
    uint32_t buffer_len;
    uint8_t *buffer;
    uint8_t *buffer_start, *pos, *buffer_end;
    // uint32_t block_size;

    void mix8(uint8_t *, uint32_t);
    void mix16(uint8_t *, uint32_t);
    void mix32(uint8_t *, uint32_t);

    // 需要确认wav的data到底是uint还是int
    // Need to confirm whether the data of wav is uint or int
    inline uint8_t mix_uint8(uint8_t, uint8_t);
    inline int8_t mix_int8(int8_t, int8_t);
    inline int16_t mix_int16(int16_t, int16_t);
    inline int32_t mix_int32(int32_t, int32_t);
};

#endif
