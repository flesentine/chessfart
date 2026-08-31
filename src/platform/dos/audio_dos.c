#include <conio.h>
#include <stdlib.h>

#include "audio_platform.h"

static unsigned g_sb_base = 0x220U;
static int g_sb_ready;

static int hex_value(int ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return -1;
}

static void parse_blaster(void)
{
    const char *env = getenv("BLASTER");
    int value;
    int digit;
    if (env == 0) return;
    while (*env != '\0') {
        if (*env == 'A' || *env == 'a') {
            ++env;
            value = 0;
            digit = hex_value((unsigned char)*env);
            while (digit >= 0) {
                value = value * 16 + digit;
                ++env;
                digit = hex_value((unsigned char)*env);
            }
            if (value >= 0x200 && value <= 0x280)
                g_sb_base = (unsigned)value;
            return;
        }
        ++env;
    }
}

static void short_delay(void)
{
    volatile unsigned i;
    for (i = 0U; i < 6000U; ++i) { }
}

static int sb_wait_write(void)
{
    unsigned long guard;
    for (guard = 0UL; guard < 100000UL; ++guard)
        if ((inp(g_sb_base + 0x0CU) & 0x80) == 0) return 1;
    return 0;
}

static int sb_wait_read(void)
{
    unsigned long guard;
    for (guard = 0UL; guard < 100000UL; ++guard)
        if ((inp(g_sb_base + 0x0EU) & 0x80) != 0) return 1;
    return 0;
}

static int sb_reset(void)
{
    outp(g_sb_base + 0x06U, 1);
    short_delay();
    outp(g_sb_base + 0x06U, 0);
    if (!sb_wait_read()) return 0;
    return inp(g_sb_base + 0x0AU) == 0xAA;
}

static int sb_write(unsigned value)
{
    if (!sb_wait_write()) return 0;
    outp(g_sb_base + 0x0CU, value & 255U);
    return 1;
}

static void sample_pace(unsigned sample_rate)
{
    volatile unsigned i;
    unsigned loops = sample_rate >= 11025U ? 9U : 13U;
    for (i = 0U; i < loops; ++i) { }
}

static void speaker_off(void)
{
    outp(0x61, inp(0x61) & 0xFC);
}

static void speaker_tone(unsigned frequency, unsigned duration_units)
{
    unsigned divisor;
    unsigned old;
    volatile unsigned long i;
    if (frequency < 40U) frequency = 40U;
    divisor = (unsigned)(1193180UL / frequency);
    outp(0x43, 0xB6);
    outp(0x42, divisor & 255U);
    outp(0x42, (divisor >> 8) & 255U);
    old = (unsigned)inp(0x61);
    outp(0x61, old | 3U);
    for (i = 0UL; i < (unsigned long)duration_units * 5500UL; ++i) { }
    speaker_off();
}

CfAudioDevice audio_platform_init(CfAudioDevice requested)
{
    parse_blaster();
    g_sb_ready = 0;
    if (requested == CF_AUDIO_DEVICE_NONE) return CF_AUDIO_DEVICE_NONE;
    if (requested == CF_AUDIO_DEVICE_PC_SPEAKER)
        return CF_AUDIO_DEVICE_PC_SPEAKER;
    if (sb_reset()) {
        g_sb_ready = 1;
        return CF_AUDIO_DEVICE_SB;
    }
    if (requested == CF_AUDIO_DEVICE_AUTO)
        return CF_AUDIO_DEVICE_PC_SPEAKER;
    return CF_AUDIO_DEVICE_NONE;
}

void audio_platform_shutdown(void)
{
    speaker_off();
    g_sb_ready = 0;
}

void audio_platform_note_event(CfAudioEvent event, CfAudioSampleId sample,
                               unsigned length, CfAudioLevel level)
{
    (void)event;
    (void)sample;
    (void)length;
    (void)level;
}

void audio_platform_play_pcm(const cf_u8 *samples, unsigned length,
                             unsigned sample_rate, CfAudioLevel level,
                             CfAudioEvent event, CfAudioSampleId sample)
{
    unsigned i;
    (void)level;
    (void)event;
    (void)sample;
    if (!g_sb_ready || samples == 0) return;
    for (i = 0U; i < length; ++i) {
        if (!sb_write(0x10U)) break;
        if (!sb_write(samples[i])) break;
        sample_pace(sample_rate);
    }
}

void audio_platform_play_speaker(CfAudioEvent event, CfAudioLevel level)
{
    unsigned duration = level == CF_AUDIO_LEVEL_HIGH ? 3U : 2U;
    switch (event) {
    case CF_AUDIO_CURSOR:
        speaker_tone(1350U, 1U); break;
    case CF_AUDIO_SELECT:
    case CF_AUDIO_MENU_CONFIRM:
        speaker_tone(980U, 1U); break;
    case CF_AUDIO_INVALID:
        speaker_tone(180U, duration); break;
    case CF_AUDIO_MOVE:
        speaker_tone(520U, 1U); break;
    case CF_AUDIO_CAPTURE:
        speaker_tone(320U, 1U); speaker_tone(210U, 1U); break;
    case CF_AUDIO_FART_READY:
        speaker_tone(760U, 1U); break;
    case CF_AUDIO_FART_PUFF:
    case CF_AUDIO_FART_PUSH:
    case CF_AUDIO_FART_BLOCKED:
        speaker_tone(190U, 1U); speaker_tone(150U, 1U);
        speaker_tone(115U, duration); break;
    case CF_AUDIO_CHECK:
        speaker_tone(880U, 1U); speaker_tone(660U, 1U);
        speaker_tone(880U, 1U); break;
    case CF_AUDIO_CHECKMATE:
        speaker_tone(523U, 1U); speaker_tone(659U, 1U);
        speaker_tone(784U, 1U); speaker_tone(1046U, duration); break;
    case CF_AUDIO_PROMOTION:
        speaker_tone(523U, 1U); speaker_tone(659U, 1U);
        speaker_tone(784U, 1U); break;
    default:
        break;
    }
}
