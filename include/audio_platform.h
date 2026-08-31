#ifndef CF_AUDIO_PLATFORM_H
#define CF_AUDIO_PLATFORM_H

#include "audio.h"

CfAudioDevice audio_platform_init(CfAudioDevice requested);
void audio_platform_shutdown(void);
void audio_platform_play_pcm(const cf_u8 *samples, unsigned length,
                             unsigned sample_rate, CfAudioLevel level,
                             CfAudioEvent event, CfAudioSampleId sample);
void audio_platform_play_speaker(CfAudioEvent event, CfAudioLevel level);
void audio_platform_note_event(CfAudioEvent event, CfAudioSampleId sample,
                               unsigned length, CfAudioLevel level);

#endif
