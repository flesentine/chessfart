#include "audio.h"

static int clamp_byte(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

static unsigned lcg_next(unsigned *state)
{
    *state = (*state * 25173U + 13849U) & 65535U;
    return *state;
}

static unsigned make_fart(cf_u8 *buffer, unsigned capacity,
                          unsigned wanted, unsigned period_start,
                          unsigned period_end, unsigned noise_strength,
                          unsigned seed, int crackle)
{
    unsigned i;
    unsigned state = seed;
    unsigned length = wanted < capacity ? wanted : capacity;
    unsigned period;
    unsigned envelope;
    int tone;
    int noise;
    int value;

    if (length == 0U) return 0U;
    for (i = 0U; i < length; ++i) {
        period = period_start +
                 (period_end - period_start) * i / length;
        if (period < 2U) period = 2U;
        envelope = (length - i) * 112U / length;
        tone = ((i / period) & 1U) ? (int)envelope : -(int)envelope;
        noise = (int)((lcg_next(&state) >> 8) & 255U) - 128;
        noise = noise * (int)noise_strength / 128;
        if (crackle && (lcg_next(&state) & 127U) < 5U)
            noise += ((lcg_next(&state) & 1U) ? 70 : -70);
        value = 128 + tone * 3 / 5 + noise * (int)(length - i) / (int)length;
        buffer[i] = (cf_u8)clamp_byte(value);
    }
    return length;
}

static unsigned make_tone(cf_u8 *buffer, unsigned capacity,
                          unsigned wanted, unsigned period,
                          unsigned decay, int second_period)
{
    unsigned i;
    unsigned length = wanted < capacity ? wanted : capacity;
    unsigned env;
    unsigned p;
    int wave;
    int value;

    if (length == 0U) return 0U;
    for (i = 0U; i < length; ++i) {
        p = period;
        if (second_period > 0 && i > length / 2U)
            p = (unsigned)second_period;
        if (p < 2U) p = 2U;
        env = 110U;
        if (decay != 0U)
            env = 110U * (length - i) / length;
        wave = ((i / p) & 1U) ? (int)env : -(int)env;
        value = 128 + wave;
        buffer[i] = (cf_u8)clamp_byte(value);
    }
    return length;
}

static unsigned make_noise_click(cf_u8 *buffer, unsigned capacity,
                                 unsigned wanted, unsigned seed,
                                 unsigned strength)
{
    unsigned i;
    unsigned state = seed;
    unsigned length = wanted < capacity ? wanted : capacity;
    unsigned env;
    int noise;
    int value;

    for (i = 0U; i < length; ++i) {
        env = strength * (length - i) / (length == 0U ? 1U : length);
        noise = (int)((lcg_next(&state) >> 8) & 255U) - 128;
        value = 128 + noise * (int)env / 128;
        buffer[i] = (cf_u8)clamp_byte(value);
    }
    return length;
}

static unsigned make_fanfare(cf_u8 *buffer, unsigned capacity)
{
    unsigned i;
    unsigned length = 3300U < capacity ? 3300U : capacity;
    unsigned segment;
    unsigned period;
    unsigned env;
    int wave;

    for (i = 0U; i < length; ++i) {
        segment = i / 660U;
        switch (segment) {
        case 0U: period = 34U; break;
        case 1U: period = 28U; break;
        case 2U: period = 23U; break;
        case 3U: period = 19U; break;
        default: period = 25U; break;
        }
        env = 105U;
        if (i > 2700U) env = 105U * (length - i) / (length - 2700U);
        wave = ((i / period) & 1U) ? (int)env : -(int)env;
        buffer[i] = (cf_u8)clamp_byte(128 + wave);
    }
    return length;
}

unsigned audio_generate_sample(CfAudioSampleId sample,
                               cf_u8 *buffer, unsigned capacity)
{
    if (buffer == 0 || capacity == 0U) return 0U;
    switch (sample) {
    case CF_SAMPLE_TOOT_SHORT:
        return make_fart(buffer, capacity, 1450U, 34U, 49U, 25U, 0x1357U, 0);
    case CF_SAMPLE_RASP_MEDIUM:
        return make_fart(buffer, capacity, 2350U, 28U, 58U, 48U, 0x2468U, 1);
    case CF_SAMPLE_WET_LONG:
        return make_fart(buffer, capacity, 3500U, 39U, 72U, 60U, 0xBEEFU, 1);
    case CF_SAMPLE_ROYAL_BOOM:
        return make_fart(buffer, capacity, 3000U, 18U, 64U, 34U, 0xC0DEU, 0);
    case CF_SAMPLE_TINY_SQUEAK:
        return make_fart(buffer, capacity, 900U, 11U, 21U, 12U, 0x5151U, 0);
    case CF_SAMPLE_UI_TICK:
        return make_tone(buffer, capacity, 180U, 14U, 1U, 0);
    case CF_SAMPLE_SELECT_CLICK:
        return make_noise_click(buffer, capacity, 300U, 0x1111U, 72U);
    case CF_SAMPLE_INVALID_BONK:
        return make_tone(buffer, capacity, 720U, 52U, 1U, 66);
    case CF_SAMPLE_MOVE_TAP:
        return make_noise_click(buffer, capacity, 420U, 0x2222U, 96U);
    case CF_SAMPLE_CAPTURE_CLACK:
        return make_noise_click(buffer, capacity, 700U, 0x3333U, 122U);
    case CF_SAMPLE_PRESSURE_HISS:
        return make_noise_click(buffer, capacity, 650U, 0x4444U, 54U);
    case CF_SAMPLE_CHECK_ALERT:
        return make_tone(buffer, capacity, 1500U, 24U, 0U, 17);
    case CF_SAMPLE_CHECKMATE_FANFARE:
        return make_fanfare(buffer, capacity);
    case CF_SAMPLE_PROMOTION_CHIME:
        return make_tone(buffer, capacity, 1700U, 31U, 0U, 20);
    case CF_SAMPLE_MENU_CLICK:
        return make_tone(buffer, capacity, 260U, 10U, 1U, 0);
    default:
        break;
    }
    return 0U;
}
