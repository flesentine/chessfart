#ifndef CF_REPLAY_FILE_H
#define CF_REPLAY_FILE_H

#include "replay.h"

#define CF_REPLAY_FILE_VERSION 1

#ifndef CF_DEFAULT_REPLAY_PATH
#define CF_DEFAULT_REPLAY_PATH "CHESSFRT.RPL"
#endif

typedef enum CfReplayFileResult {
    CF_REPLAY_FILE_OK = 0,
    CF_REPLAY_FILE_NOT_FOUND,
    CF_REPLAY_FILE_IO_ERROR,
    CF_REPLAY_FILE_BAD_MAGIC,
    CF_REPLAY_FILE_BAD_VERSION,
    CF_REPLAY_FILE_BAD_DATA
} CfReplayFileResult;

CfReplayFileResult replay_file_save(const char *path,
                                    const CfReplayTimeline *timeline);
CfReplayFileResult replay_file_load(const char *path,
                                    CfReplayTimeline *timeline);
const char *replay_file_result_name(CfReplayFileResult result);

#endif
