#include <stdio.h>

#include "audio_platform.h"

static void write_u16(FILE *fp, unsigned value)
{
    fputc((int)(value & 255U), fp);
    fputc((int)((value >> 8) & 255U), fp);
}
static void write_u32(FILE *fp, unsigned long value)
{
    fputc((int)(value & 255UL), fp);
    fputc((int)((value >> 8) & 255UL), fp);
    fputc((int)((value >> 16) & 255UL), fp);
    fputc((int)((value >> 24) & 255UL), fp);
}
static void write_wav(const cf_u8 *samples, unsigned length,
                      unsigned sample_rate)
{
    FILE *fp;
    unsigned long data_size = (unsigned long)length;
    fp = fopen("build/host/chessfart_build9_fart.wav", "wb");
    if (fp == 0) return;
    fwrite("RIFF", 1, 4, fp);
    write_u32(fp, 36UL + data_size);
    fwrite("WAVEfmt ", 1, 8, fp);
    write_u32(fp, 16UL);
    write_u16(fp, 1U);
    write_u16(fp, 1U);
    write_u32(fp, (unsigned long)sample_rate);
    write_u32(fp, (unsigned long)sample_rate);
    write_u16(fp, 1U);
    write_u16(fp, 8U);
    fwrite("data", 1, 4, fp);
    write_u32(fp, data_size);
    fwrite(samples, 1, length, fp);
    fclose(fp);
}
static void log_line(const char *text)
{
    FILE *fp = fopen("build/host/chessfart_build9_audio.log", "a");
    if (fp == 0) return;
    fputs(text, fp);
    fputc('\n', fp);
    fclose(fp);
}
CfAudioDevice audio_platform_init(CfAudioDevice requested)
{
    FILE *fp;
    CfAudioDevice actual = requested;
    if (requested == CF_AUDIO_DEVICE_AUTO) actual = CF_AUDIO_DEVICE_SB;
    fp = fopen("build/host/chessfart_build9_audio.log", "w");
    if (fp != 0) {
        fprintf(fp, "INIT requested=%s actual=%s\n",
                audio_device_name(requested), audio_device_name(actual));
        fclose(fp);
    }
    return actual;
}
void audio_platform_shutdown(void) { log_line("SHUTDOWN"); }
void audio_platform_note_event(CfAudioEvent event, CfAudioSampleId sample,
                               unsigned length, CfAudioLevel level)
{
    FILE *fp = fopen("build/host/chessfart_build9_audio.log", "a");
    if (fp == 0) return;
    fprintf(fp, "EVENT %s SAMPLE %s LEN %u LEVEL %s\n",
            audio_event_name(event), audio_sample_name(sample), length,
            audio_level_name(level));
    fclose(fp);
}
void audio_platform_play_pcm(const cf_u8 *samples, unsigned length,
                             unsigned sample_rate, CfAudioLevel level,
                             CfAudioEvent event, CfAudioSampleId sample)
{
    (void)level;
    (void)event;
    if (samples == 0 || length == 0U) return;
    if (sample >= CF_SAMPLE_TOOT_SHORT &&
        sample <= CF_SAMPLE_TINY_SQUEAK)
        write_wav(samples, length, sample_rate);
}
void audio_platform_play_speaker(CfAudioEvent event, CfAudioLevel level)
{
    FILE *fp = fopen("build/host/chessfart_build9_audio.log", "a");
    if (fp == 0) return;
    fprintf(fp, "PCSPK %s LEVEL %s\n",
            audio_event_name(event), audio_level_name(level));
    fclose(fp);
}
