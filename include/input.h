#ifndef CF_INPUT_H
#define CF_INPUT_H

typedef enum CfInputKey {
    CF_KEY_NONE = 0,
    CF_KEY_ESCAPE,
    CF_KEY_UP,
    CF_KEY_DOWN,
    CF_KEY_LEFT,
    CF_KEY_RIGHT,
    CF_KEY_UP_LEFT,
    CF_KEY_UP_RIGHT,
    CF_KEY_DOWN_LEFT,
    CF_KEY_DOWN_RIGHT,
    CF_KEY_ENTER,
    CF_KEY_FART,
    CF_KEY_SAVE,
    CF_KEY_LOAD,
    CF_KEY_DIFFICULTY,
    CF_KEY_HELP,
    CF_KEY_HISTORY,
    CF_KEY_CREDITS,
    CF_KEY_REPLAY,
    CF_KEY_UNDO,
    CF_KEY_THEME
} CfInputKey;

void input_init(void);
CfInputKey input_poll_key(void);

#endif
