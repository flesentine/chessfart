#ifndef CF_UI_LAYOUT_H
#define CF_UI_LAYOUT_H

/* Canonical 320x200 retail layout. Keep input and rendering in sync. */
#define CF_UI_SCREEN_W 320
#define CF_UI_SCREEN_H 200

#define CF_UI_HEADER_X 0
#define CF_UI_HEADER_Y 0
#define CF_UI_HEADER_W CF_UI_SCREEN_W
#define CF_UI_HEADER_H 21

#define CF_UI_BOARD_X 18
#define CF_UI_BOARD_Y 27
#define CF_UI_SQUARE_SIZE 18
#define CF_UI_BOARD_PIXELS (CF_UI_SQUARE_SIZE * 8)

#define CF_UI_BOARD_FRAME_X 14
#define CF_UI_BOARD_FRAME_Y 23
#define CF_UI_BOARD_FRAME_W 152
#define CF_UI_BOARD_FRAME_H 154

#define CF_UI_PANEL_X 170
#define CF_UI_PANEL_Y 24
#define CF_UI_PANEL_W 143
#define CF_UI_PANEL_H 151
#define CF_UI_PANEL_INSET 10
#define CF_UI_PANEL_CONTENT_X (CF_UI_PANEL_X + CF_UI_PANEL_INSET)
#define CF_UI_PANEL_CONTENT_W 123

#define CF_UI_COMMAND_X 0
#define CF_UI_COMMAND_Y 181
#define CF_UI_COMMAND_W CF_UI_SCREEN_W
#define CF_UI_COMMAND_H 19

#endif
