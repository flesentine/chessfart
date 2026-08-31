#include <string.h>

#include "audio.h"
#include "audio_platform.h"

static CfAudioConfig g_config;
static CfAudioStats g_stats;
static cf_u8 g_pcm[CF_AUDIO_PCM_CAPACITY];

void audio_default_config(CfAudioConfig *config)
{
    if (config == 0) return;
    config->device = CF_AUDIO_DEVICE_AUTO;
    config->sfx_level = CF_AUDIO_LEVEL_MED;
    config->music_level = CF_AUDIO_LEVEL_OFF;
}

static int event_valid(CfAudioEvent event)
{
    return (int)event >= 0 && event < CF_AUDIO_EVENT_COUNT;
}

const char *audio_device_name(CfAudioDevice device)
{
    switch (device) {
    case CF_AUDIO_DEVICE_AUTO: return "AUTO";
    case CF_AUDIO_DEVICE_SB: return "SB";
    case CF_AUDIO_DEVICE_PC_SPEAKER: return "PCSPK";
    case CF_AUDIO_DEVICE_NONE: return "NONE";
    default: return "?";
    }
}

const char *audio_level_name(CfAudioLevel level)
{
    switch (level) {
    case CF_AUDIO_LEVEL_OFF: return "OFF";
    case CF_AUDIO_LEVEL_LOW: return "LOW";
    case CF_AUDIO_LEVEL_MED: return "MED";
    case CF_AUDIO_LEVEL_HIGH: return "HIGH";
    default: return "?";
    }
}

const char *audio_event_name(CfAudioEvent event)
{
    static const char *names[CF_AUDIO_EVENT_COUNT] = {
        "CURSOR", "SELECT", "INVALID", "MOVE", "CAPTURE",
        "FART_READY", "FART_PUFF", "FART_PUSH", "FART_BLOCKED",
        "CHECK", "CHECKMATE", "PROMOTION", "MENU_CONFIRM"
    };
    if (!event_valid(event)) return "?";
    return names[(int)event];
}

const char *audio_sample_name(CfAudioSampleId sample)
{
    static const char *names[CF_SAMPLE_COUNT] = {
        "TOOT_SHORT", "RASP_MEDIUM", "WET_LONG", "ROYAL_BOOM",
        "TINY_SQUEAK", "UI_TICK", "SELECT_CLICK", "INVALID_BONK",
        "MOVE_TAP", "CAPTURE_CLACK", "PRESSURE_HISS", "CHECK_ALERT",
        "CHECKMATE_FANFARE", "PROMOTION_CHIME", "MENU_CLICK"
    };
    if ((int)sample < 0 || sample >= CF_SAMPLE_COUNT) return "?";
    return names[(int)sample];
}

static CfAudioSampleId sample_for_event(CfAudioEvent event)
{
    switch (event) {
    case CF_AUDIO_CURSOR: return CF_SAMPLE_UI_TICK;
    case CF_AUDIO_SELECT: return CF_SAMPLE_SELECT_CLICK;
    case CF_AUDIO_INVALID: return CF_SAMPLE_INVALID_BONK;
    case CF_AUDIO_MOVE: return CF_SAMPLE_MOVE_TAP;
    case CF_AUDIO_CAPTURE: return CF_SAMPLE_CAPTURE_CLACK;
    case CF_AUDIO_FART_READY: return CF_SAMPLE_PRESSURE_HISS;
    case CF_AUDIO_FART_PUFF: return CF_SAMPLE_TOOT_SHORT;
    case CF_AUDIO_FART_PUSH: return CF_SAMPLE_RASP_MEDIUM;
    case CF_AUDIO_FART_BLOCKED: return CF_SAMPLE_TINY_SQUEAK;
    case CF_AUDIO_CHECK: return CF_SAMPLE_CHECK_ALERT;
    case CF_AUDIO_CHECKMATE: return CF_SAMPLE_CHECKMATE_FANFARE;
    case CF_AUDIO_PROMOTION: return CF_SAMPLE_PROMOTION_CHIME;
    case CF_AUDIO_MENU_CONFIRM: return CF_SAMPLE_MENU_CLICK;
    default: return CF_SAMPLE_UI_TICK;
    }
}

static void scale_pcm(cf_u8 *buffer, unsigned length, CfAudioLevel level)
{
    unsigned i;
    int centered;
    int numerator;
    int denominator = 3;

    if (level == CF_AUDIO_LEVEL_HIGH) numerator = 3;
    else if (level == CF_AUDIO_LEVEL_MED) numerator = 2;
    else numerator = 1;

    for (i = 0U; i < length; ++i) {
        centered = (int)buffer[i] - 128;
        centered = centered * numerator / denominator;
        buffer[i] = (cf_u8)(128 + centered);
    }
}

int audio_init(const CfAudioConfig *config)
{
    CfAudioConfig local;
    if (config == 0) {
        audio_default_config(&local);
        config = &local;
    }
    g_config = *config;
    memset(&g_stats, 0, sizeof(g_stats));
    g_stats.last_event = CF_AUDIO_CURSOR;
    g_stats.actual_device = audio_platform_init(g_config.device);
    g_stats.initialized = 1;
    return 1;
}

void audio_shutdown(void)
{
    if (g_stats.initialized) audio_platform_shutdown();
    g_stats.initialized = 0;
}

void audio_apply_config(const CfAudioConfig *config)
{
    if (config == 0) return;
    if (!g_stats.initialized) {
        (void)audio_init(config);
        return;
    }
    audio_platform_shutdown();
    g_config = *config;
    g_stats.actual_device = audio_platform_init(g_config.device);
}

const CfAudioConfig *audio_get_config(void)
{
    return &g_config;
}

const CfAudioStats *audio_get_stats(void)
{
    return &g_stats;
}

void audio_cycle_device(void)
{
    CfAudioConfig next = g_config;
    next.device = (CfAudioDevice)(((int)next.device + 1) % 4);
    audio_apply_config(&next);
}

void audio_cycle_sfx_level(void)
{
    g_config.sfx_level = (CfAudioLevel)(((int)g_config.sfx_level + 1) % 4);
}

static CfAudioSampleId fart_sample_for_event(CfAudioEvent event)
{
    unsigned index;
    if (event == CF_AUDIO_FART_PUFF) {
        index = g_stats.fart_counter++ % 2U;
        return index == 0U ? CF_SAMPLE_TOOT_SHORT : CF_SAMPLE_TINY_SQUEAK;
    }
    if (event == CF_AUDIO_FART_BLOCKED) {
        ++g_stats.fart_counter;
        return CF_SAMPLE_TINY_SQUEAK;
    }
    index = g_stats.fart_counter++ % 3U;
    if (index == 0U) return CF_SAMPLE_RASP_MEDIUM;
    if (index == 1U) return CF_SAMPLE_WET_LONG;
    return CF_SAMPLE_ROYAL_BOOM;
}

void audio_play_event(CfAudioEvent event)
{
    CfAudioSampleId sample;
    unsigned length;

    if (!g_stats.initialized || !event_valid(event)) return;
    if (g_config.sfx_level == CF_AUDIO_LEVEL_OFF ||
        g_stats.actual_device == CF_AUDIO_DEVICE_NONE) return;

    if (event == CF_AUDIO_FART_PUFF || event == CF_AUDIO_FART_PUSH ||
        event == CF_AUDIO_FART_BLOCKED)
        sample = fart_sample_for_event(event);
    else
        sample = sample_for_event(event);

    ++g_stats.played_events;
    ++g_stats.event_count[(int)event];
    g_stats.last_event = event;

    if (g_stats.actual_device == CF_AUDIO_DEVICE_PC_SPEAKER) {
        audio_platform_note_event(event, sample, 0U, g_config.sfx_level);
        audio_platform_play_speaker(event, g_config.sfx_level);
        return;
    }

    length = audio_generate_sample(sample, g_pcm, CF_AUDIO_PCM_CAPACITY);
    scale_pcm(g_pcm, length, g_config.sfx_level);
    audio_platform_note_event(event, sample, length, g_config.sfx_level);
    audio_platform_play_pcm(g_pcm, length, CF_AUDIO_SAMPLE_RATE,
                            g_config.sfx_level, event, sample);
}

void audio_play_fart_result(CfFartPreview result)
{
    switch (result) {
    case CF_FART_PUFF:
        audio_play_event(CF_AUDIO_FART_PUFF);
        break;
    case CF_FART_PUSH:
    case CF_FART_PROMOTION:
        audio_play_event(CF_AUDIO_FART_PUSH);
        break;
    case CF_FART_BLOCKED:
        audio_play_event(CF_AUDIO_FART_BLOCKED);
        break;
    default:
        audio_play_event(CF_AUDIO_INVALID);
        break;
    }
}
