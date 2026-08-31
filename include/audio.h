#ifndef CF_AUDIO_H
#define CF_AUDIO_H

#include "cf_types.h"
#include "gas.h"

#define CF_AUDIO_SAMPLE_RATE 11025U
#define CF_AUDIO_PCM_CAPACITY 4096U

typedef enum CfAudioDevice {
    CF_AUDIO_DEVICE_AUTO = 0,
    CF_AUDIO_DEVICE_SB,
    CF_AUDIO_DEVICE_PC_SPEAKER,
    CF_AUDIO_DEVICE_NONE
} CfAudioDevice;

typedef enum CfAudioLevel {
    CF_AUDIO_LEVEL_OFF = 0,
    CF_AUDIO_LEVEL_LOW,
    CF_AUDIO_LEVEL_MED,
    CF_AUDIO_LEVEL_HIGH
} CfAudioLevel;

typedef enum CfAudioEvent {
    CF_AUDIO_CURSOR = 0,
    CF_AUDIO_SELECT,
    CF_AUDIO_INVALID,
    CF_AUDIO_MOVE,
    CF_AUDIO_CAPTURE,
    CF_AUDIO_FART_READY,
    CF_AUDIO_FART_PUFF,
    CF_AUDIO_FART_PUSH,
    CF_AUDIO_FART_BLOCKED,
    CF_AUDIO_CHECK,
    CF_AUDIO_CHECKMATE,
    CF_AUDIO_PROMOTION,
    CF_AUDIO_MENU_CONFIRM,
    CF_AUDIO_EVENT_COUNT
} CfAudioEvent;

typedef enum CfAudioSampleId {
    CF_SAMPLE_TOOT_SHORT = 0,
    CF_SAMPLE_RASP_MEDIUM,
    CF_SAMPLE_WET_LONG,
    CF_SAMPLE_ROYAL_BOOM,
    CF_SAMPLE_TINY_SQUEAK,
    CF_SAMPLE_UI_TICK,
    CF_SAMPLE_SELECT_CLICK,
    CF_SAMPLE_INVALID_BONK,
    CF_SAMPLE_MOVE_TAP,
    CF_SAMPLE_CAPTURE_CLACK,
    CF_SAMPLE_PRESSURE_HISS,
    CF_SAMPLE_CHECK_ALERT,
    CF_SAMPLE_CHECKMATE_FANFARE,
    CF_SAMPLE_PROMOTION_CHIME,
    CF_SAMPLE_MENU_CLICK,
    CF_SAMPLE_COUNT
} CfAudioSampleId;

typedef struct CfAudioConfig {
    CfAudioDevice device;
    CfAudioLevel sfx_level;
    CfAudioLevel music_level;
} CfAudioConfig;

typedef struct CfAudioStats {
    int initialized;
    CfAudioDevice actual_device;
    CfAudioEvent last_event;
    unsigned played_events;
    unsigned event_count[CF_AUDIO_EVENT_COUNT];
    unsigned fart_counter;
} CfAudioStats;

void audio_default_config(CfAudioConfig *config);
int audio_init(const CfAudioConfig *config);
void audio_shutdown(void);
void audio_apply_config(const CfAudioConfig *config);
const CfAudioConfig *audio_get_config(void);
const CfAudioStats *audio_get_stats(void);

void audio_cycle_device(void);
void audio_cycle_sfx_level(void);

const char *audio_device_name(CfAudioDevice device);
const char *audio_level_name(CfAudioLevel level);
const char *audio_event_name(CfAudioEvent event);
const char *audio_sample_name(CfAudioSampleId sample);

void audio_play_event(CfAudioEvent event);
void audio_play_fart_result(CfFartPreview result);
unsigned audio_generate_sample(CfAudioSampleId sample,
                               cf_u8 *buffer, unsigned capacity);

#endif
