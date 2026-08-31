#include <stdio.h>
#include <string.h>
#include "cpu.h"

void cpu_format_action(const CfCpuAction *a, char *out, unsigned cap)
{
    char temp[40];
    unsigned len;
    if (out == 0 || cap == 0U) return;
    if (a == 0 || a->type == CF_CPU_ACTION_NONE) strcpy(temp, "CPU NONE");
    else if (a->type == CF_CPU_ACTION_MOVE) {
        sprintf(temp, "CPU %c%c-%c%c", (char)('A'+a->from_file),
                (char)('1'+a->from_rank), (char)('A'+a->to_file),
                (char)('1'+a->to_rank));
        if (a->promotion != CF_PIECE_NONE) {
            len = (unsigned)strlen(temp);
            if (len + 2U < sizeof(temp)) {
                temp[len] = '='; temp[len+1U] = board_piece_letter(a->promotion);
                temp[len+2U] = '\0';
            }
        }
    } else {
        sprintf(temp, "CPU FART %s %s", gas_direction_name(a->direction),
                gas_fart_preview_name(a->fart_result));
    }
    strncpy(out, temp, cap-1U); out[cap-1U] = '\0';
}
