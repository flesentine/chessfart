#include <emscripten.h>
#include "mouse.h"

int mouse_init(void)
{
    EM_ASM({
        var canvas = Module['canvas'] || document.getElementById('canvas');
        Module.cfMouseX = 0;
        Module.cfMouseY = 0;
        Module.cfMouseButtons = 0;
        if (!canvas || Module.cfMouseInstalled) return;
        Module.cfMouseInstalled = true;
        function update(e) {
            var r = canvas.getBoundingClientRect();
            var x = (e.clientX - r.left) * 320 / r.width;
            var y = (e.clientY - r.top) * 200 / r.height;
            if (x < 0) x = 0; if (x > 319) x = 319;
            if (y < 0) y = 0; if (y > 199) y = 199;
            Module.cfMouseX = x | 0;
            Module.cfMouseY = y | 0;
        }
        canvas.addEventListener('pointermove', function(e) {
            update(e);
            Module.cfMouseButtons = e.buttons | 0;
        });
        canvas.addEventListener('pointerdown', function(e) {
            update(e);
            Module.cfMouseButtons = e.buttons | 0;
            try { canvas.setPointerCapture(e.pointerId); } catch (_) {}
            if (Module.cfAudioCtx && Module.cfAudioCtx.state === 'suspended') Module.cfAudioCtx.resume();
            e.preventDefault();
        }, { passive: false });
        canvas.addEventListener('pointerup', function(e) {
            update(e);
            Module.cfMouseButtons = e.buttons | 0;
            e.preventDefault();
        }, { passive: false });
        canvas.addEventListener('pointercancel', function() { Module.cfMouseButtons = 0; });
        canvas.addEventListener('contextmenu', function(e) { e.preventDefault(); });
    });
    return 1;
}

void mouse_shutdown(void)
{
}

int mouse_poll(CfMouseState *state)
{
    if (state == 0) return 0;
    state->present = 1;
    state->x = EM_ASM_INT({ return Module.cfMouseX | 0; });
    state->y = EM_ASM_INT({ return Module.cfMouseY | 0; });
    state->buttons = (unsigned)EM_ASM_INT({ return Module.cfMouseButtons | 0; });
    return 1;
}
