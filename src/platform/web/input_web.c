#include <emscripten.h>
#include "input_build5.h"

void input5_init(void)
{
    EM_ASM({
        if (!Module.cfKeyQueue) Module.cfKeyQueue = [];
        if (Module.cfKeyboardInstalled) return;
        Module.cfKeyboardInstalled = true;
        Module.cfPushKey = Module.cfPushKey || function(code) {
            if (Module.cfKeyQueue.length < 64) Module.cfKeyQueue.push(code|0);
            if (Module.cfAudioCtx && Module.cfAudioCtx.state === 'suspended') {
                Module.cfAudioCtx.resume();
            }
        };
        window.addEventListener('keydown', function(e) {
            var code = 0;
            switch (e.key) {
            case 'Escape': code = 1; break;
            case 'ArrowUp': code = 2; break;
            case 'ArrowDown': code = 3; break;
            case 'ArrowLeft': code = 4; break;
            case 'ArrowRight': code = 5; break;
            case 'Home': code = 6; break;
            case 'PageUp': code = 7; break;
            case 'End': code = 8; break;
            case 'PageDown': code = 9; break;
            case 'Enter': case ' ': code = 10; break;
            case 'f': case 'F': code = 11; break;
            case 's': case 'S': code = 12; break;
            case 'l': case 'L': code = 13; break;
            case 'd': case 'D': code = 14; break;
            case 'h': case 'H': case '?': code = 15; break;
            case 'Tab': case 'm': case 'M': code = 16; break;
            case 'c': case 'C': code = 17; break;
            case 'r': case 'R': code = 18; break;
            case 'u': case 'U': code = 19; break;
            case 't': case 'T': code = 20; break;
            default: break;
            }
            if (code) {
                e.preventDefault();
                Module.cfPushKey(code);
            }
        }, { passive: false });
    });
}

CfInputKey5 input5_poll_key(void)
{
    int code;
    code = EM_ASM_INT({
        if (!Module.cfKeyQueue || Module.cfKeyQueue.length === 0) return 0;
        return Module.cfKeyQueue.shift() | 0;
    });
    if (code == 0) {
        emscripten_sleep(8);
        return CF5_KEY_NONE;
    }
    return (CfInputKey5)code;
}
