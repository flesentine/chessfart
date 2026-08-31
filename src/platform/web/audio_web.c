#include <emscripten.h>
#include "audio_platform.h"

CfAudioDevice audio_platform_init(CfAudioDevice requested)
{
    if (requested == CF_AUDIO_DEVICE_NONE) return CF_AUDIO_DEVICE_NONE;
    if (requested == CF_AUDIO_DEVICE_PC_SPEAKER) return CF_AUDIO_DEVICE_PC_SPEAKER;
    return CF_AUDIO_DEVICE_SB;
}

void audio_platform_shutdown(void)
{
}

void audio_platform_play_pcm(const cf_u8 *samples, unsigned length,
                             unsigned sample_rate, CfAudioLevel level,
                             CfAudioEvent event, CfAudioSampleId sample)
{
    (void)event;
    (void)sample;
    if (samples == 0 || length == 0U) return;
    EM_ASM({
        var AC = window.AudioContext || window.webkitAudioContext;
        if (!AC) return;
        if (!Module.cfAudioCtx) Module.cfAudioCtx = new AC();
        var ctx = Module.cfAudioCtx;
        if (ctx.state === 'suspended') ctx.resume();
        var len = $1 | 0;
        var rate = $2 | 0;
        var buffer = ctx.createBuffer(1, len, rate);
        var dst = buffer.getChannelData(0);
        var src = HEAPU8.subarray($0, $0 + len);
        for (var i = 0; i < len; ++i) dst[i] = (src[i] - 128) / 128.0;
        var source = ctx.createBufferSource();
        var gain = ctx.createGain();
        var level = $3 | 0;
        gain.gain.value = level === 1 ? 0.35 : (level === 2 ? 0.65 : 0.95);
        source.buffer = buffer;
        source.connect(gain);
        gain.connect(ctx.destination);
        source.start();
    }, samples, length, sample_rate, level);
}

void audio_platform_play_speaker(CfAudioEvent event, CfAudioLevel level)
{
    EM_ASM({
        var AC = window.AudioContext || window.webkitAudioContext;
        if (!AC) return;
        if (!Module.cfAudioCtx) Module.cfAudioCtx = new AC();
        var ctx = Module.cfAudioCtx;
        if (ctx.state === 'suspended') ctx.resume();
        var osc = ctx.createOscillator();
        var gain = ctx.createGain();
        var ev = $0 | 0;
        var lvl = $1 | 0;
        osc.type = 'square';
        osc.frequency.value = (ev === 2 ? 130 : (ev === 9 || ev === 10 ? 880 : 440));
        gain.gain.setValueAtTime(lvl === 1 ? 0.025 : (lvl === 2 ? 0.05 : 0.08), ctx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.07);
        osc.connect(gain);
        gain.connect(ctx.destination);
        osc.start();
        osc.stop(ctx.currentTime + 0.075);
    }, event, level);
}

void audio_platform_note_event(CfAudioEvent event, CfAudioSampleId sample,
                               unsigned length, CfAudioLevel level)
{
    (void)event;
    (void)sample;
    (void)length;
    (void)level;
}
