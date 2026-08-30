#ifndef CF_PRESENTATION_H
#define CF_PRESENTATION_H

#include "gas.h"

int presentation_title_screen(void);
int presentation_make_fart(CfBoard *board, CfGasState *gas,
                           int file, int rank, CfFartDirection direction,
                           CfPieceType promotion, CfFartAction *action);

#endif
