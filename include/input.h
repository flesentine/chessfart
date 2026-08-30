#ifndef CF_INPUT_H
#define CF_INPUT_H

typedef enum CfInputKey {
    CF_KEY_NONE = 0,
    CF_KEY_ESCAPE,
    CF_KEY_UP,
    CF_KEY_DOWN,
    CF_KEY_LEFT,
    CF_KEY_RIGHT,
    CF_KEY_ENTER
} CfInputKey;

void input_init(void);
CfInputKey input_poll_key(void);

#endif
