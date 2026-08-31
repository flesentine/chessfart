#ifndef CF_MOUSE_H
#define CF_MOUSE_H

typedef struct CfMouseState {
    int present;
    int x;
    int y;
    unsigned buttons;
} CfMouseState;

int mouse_init(void);
void mouse_shutdown(void);
int mouse_poll(CfMouseState *state);

#endif
