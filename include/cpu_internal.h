#ifndef CF_CPU_INTERNAL_H
#define CF_CPU_INTERNAL_H
#include "cpu.h"
void cpu_internal_sort_actions(CfCpuActionList *list);
int cpu_internal_evaluate(const CfBoard *board, const CfGasState *gas);
#endif
