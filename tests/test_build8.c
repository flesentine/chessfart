#include <stdio.h>
#include <string.h>

#include "audio.h"

static int failures;
#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static void test_defaults_and_devices(void)
{
    CfAudioConfig config;
    const CfAudioStats *stats;
    audio_default_config(&config);
    CHECK(config.device == CF_AUDIO_DEVICE_AUTO);
    CHECK(config.sfx_level == CF_AUDIO_LEVEL_MED);
    CHECK(config.music_level == CF_AUDIO_LEVEL_OFF);
    CHECK(audio_init(&config));
    stats = audio_get_stats();
    CHECK(stats->initialized);
    CHECK(stats->actual_device == CF_AUDIO_DEVICE_SB);
    audio_cycle_device();
    CHECK(audio_get_config()->device == CF_AUDIO_DEVICE_SB);
    audio_cycle_device();
    CHECK(audio_get_config()->device == CF_AUDIO_DEVICE_PC_SPEAKER);
    audio_cycle_device();
    CHECK(audio_get_config()->device == CF_AUDIO_DEVICE_NONE);
    audio_shutdown();
}

static void test_sample_bank(void)
{
    cf_u8 a[CF_AUDIO_PCM_CAPACITY];
    cf_u8 b[CF_AUDIO_PCM_CAPACITY];
    unsigned i;
    unsigned len_a;
    unsigned len_b;
    int different = 0;

    for (i = CF_SAMPLE_TOOT_SHORT; i <= CF_SAMPLE_TINY_SQUEAK; ++i) {
        len_a = audio_generate_sample((CfAudioSampleId)i, a, sizeof(a));
        CHECK(len_a > 500U);
        CHECK(len_a <= CF_AUDIO_PCM_CAPACITY);
    }
    len_a = audio_generate_sample(CF_SAMPLE_TOOT_SHORT, a, sizeof(a));
    len_b = audio_generate_sample(CF_SAMPLE_RASP_MEDIUM, b, sizeof(b));
    CHECK(len_a != len_b);
    for (i = 0U; i < len_a && i < len_b; ++i)
        if (a[i] != b[i]) { different = 1; break; }
    CHECK(different);
}

static void test_event_accounting(void)
{
    CfAudioConfig config;
    const CfAudioStats *stats;
    unsigned before;

    audio_default_config(&config);
    CHECK(audio_init(&config));
    audio_play_event(CF_AUDIO_MOVE);
    audio_play_event(CF_AUDIO_CAPTURE);
    audio_play_event(CF_AUDIO_CHECK);
    audio_play_event(CF_AUDIO_PROMOTION);
    audio_play_fart_result(CF_FART_PUFF);
    audio_play_fart_result(CF_FART_PUSH);
    audio_play_fart_result(CF_FART_BLOCKED);
    stats = audio_get_stats();
    CHECK(stats->event_count[CF_AUDIO_MOVE] == 1U);
    CHECK(stats->event_count[CF_AUDIO_CAPTURE] == 1U);
    CHECK(stats->event_count[CF_AUDIO_CHECK] == 1U);
    CHECK(stats->event_count[CF_AUDIO_PROMOTION] == 1U);
    CHECK(stats->event_count[CF_AUDIO_FART_PUFF] == 1U);
    CHECK(stats->event_count[CF_AUDIO_FART_PUSH] == 1U);
    CHECK(stats->event_count[CF_AUDIO_FART_BLOCKED] == 1U);
    CHECK(stats->fart_counter == 3U);

    before = stats->played_events;
    while (audio_get_config()->sfx_level != CF_AUDIO_LEVEL_OFF)
        audio_cycle_sfx_level();
    audio_play_event(CF_AUDIO_MOVE);
    CHECK(audio_get_stats()->played_events == before);
    audio_shutdown();
}

int main(void)
{
    test_defaults_and_devices();
    test_sample_bank();
    test_event_accounting();
    if (failures != 0) {
        printf("Build 8 audio tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 8 audio tests passed.\n");
    return 0;
}
