#ifndef CF_PERSISTENCE_H
#define CF_PERSISTENCE_H

#include "audio.h"
#include "board.h"
#include "gas.h"
#include "match_mode.h"

#define CF_SAVE_VERSION 2
#define CF_SAVE_VERSION_LEGACY 1
#define CF_CONFIG_VERSION 1
#ifndef CF_DEFAULT_SAVE_PATH
#define CF_DEFAULT_SAVE_PATH "CHESSFRT.SAV"
#endif
#ifndef CF_DEFAULT_CONFIG_PATH
#define CF_DEFAULT_CONFIG_PATH "CHESSFRT.CFG"
#endif

typedef enum CfPersistenceResult {
    CF_PERSIST_OK = 0,
    CF_PERSIST_NOT_FOUND,
    CF_PERSIST_IO_ERROR,
    CF_PERSIST_BAD_MAGIC,
    CF_PERSIST_BAD_VERSION,
    CF_PERSIST_BAD_DATA,
    CF_PERSIST_DISABLED
} CfPersistenceResult;

CfPersistenceResult persistence_save_game(const char *path,
                                          const CfBoard *board,
                                          const CfGasState *gas,
                                          const CfGasHistory *history);
CfPersistenceResult persistence_load_game(const char *path,
                                          CfBoard *board,
                                          CfGasState *gas,
                                          CfGasHistory *history);
CfPersistenceResult persistence_save_game_mode(const char *path,
                                               const CfBoard *board,
                                               const CfGasState *gas,
                                               const CfGasHistory *history,
                                               CfMatchMode match_mode);
CfPersistenceResult persistence_load_game_mode(const char *path,
                                               CfBoard *board,
                                               CfGasState *gas,
                                               CfGasHistory *history,
                                               CfMatchMode *match_mode);
CfPersistenceResult persistence_save_config(const char *path,
                                            const CfAudioConfig *config);
CfPersistenceResult persistence_load_config(const char *path,
                                            CfAudioConfig *config);
const char *persistence_result_name(CfPersistenceResult result);

#endif
