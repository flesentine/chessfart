#ifndef CF_INPUT_BUILD5_H
#define CF_INPUT_BUILD5_H

typedef enum CfInputKey5 {
    CF5_KEY_NONE = 0,
    CF5_KEY_ESCAPE,
    CF5_KEY_UP,
    CF5_KEY_DOWN,
    CF5_KEY_LEFT,
    CF5_KEY_RIGHT,
    CF5_KEY_UP_LEFT,
    CF5_KEY_UP_RIGHT,
    CF5_KEY_DOWN_LEFT,
    CF5_KEY_DOWN_RIGHT,
    CF5_KEY_ENTER,
    CF5_KEY_FART,
    CF5_KEY_SAVE,
    CF5_KEY_LOAD,
    CF5_KEY_DIFFICULTY,
    CF5_KEY_HELP,
    CF5_KEY_HISTORY,
    CF5_KEY_CREDITS,
    CF5_KEY_REPLAY,
    CF5_KEY_UNDO,
    CF5_KEY_THEME
} CfInputKey5;

void input5_init(void);
CfInputKey5 input5_poll_key(void);

#endif
