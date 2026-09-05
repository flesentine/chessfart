#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "replay_file.h"

#define CF_REPLAY_PATH_BUFFER 128
#define CF_REPLAY_WORD_BUFFER 32
#define CF_REPLAY_HEX_BUFFER (CF_REPLAY_LABEL * 2 + 1)

static int replay_file_valid_path(const char *path)
{
    size_t len;
    if (path == 0 || path[0] == '\0') return 0;
    len = strlen(path);
    return len + 5U < CF_REPLAY_PATH_BUFFER;
}

static int replay_file_temp_path(const char *path, char *temp)
{
    if (!replay_file_valid_path(path) || temp == 0) return 0;
    strcpy(temp, path);
    strcat(temp, ".TMP");
    return 1;
}

static CfReplayFileResult replay_file_replace(const char *path,
                                              const char *temp)
{
    (void)remove(path);
    if (rename(temp, path) != 0) {
        (void)remove(temp);
        return CF_REPLAY_FILE_IO_ERROR;
    }
    return CF_REPLAY_FILE_OK;
}

static int replay_file_label_length(const char *label)
{
    int i;
    if (label == 0) return -1;
    for (i = 0; i < CF_REPLAY_LABEL; ++i)
        if (label[i] == '\0') return i;
    return -1;
}

static int replay_file_hex_value(int ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return -1;
}

static int replay_file_decode_label(CfReplaySnapshot *snapshot,
                                    int length,
                                    const char *hex)
{
    int i;
    int hi;
    int lo;
    size_t hex_length;

    if (snapshot == 0 || hex == 0 ||
        length < 0 || length >= CF_REPLAY_LABEL) return 0;

    if (length == 0) {
        if (strcmp(hex, "-") != 0) return 0;
        snapshot->label[0] = '\0';
        return 1;
    }

    hex_length = strlen(hex);
    if (hex_length != (size_t)(length * 2)) return 0;

    for (i = 0; i < length; ++i) {
        hi = replay_file_hex_value((unsigned char)hex[i * 2]);
        lo = replay_file_hex_value((unsigned char)hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) return 0;
        snapshot->label[i] = (char)((hi << 4) | lo);
        if (snapshot->label[i] == '\0') return 0;
    }
    snapshot->label[length] = '\0';
    return 1;
}

static int replay_file_snapshot_valid(const CfReplaySnapshot *snapshot)
{
    CfBoard board;
    CfGasState gas;
    CfGameStatus status;
    CfMatchMode mode;

    if (snapshot == 0 ||
        replay_file_label_length(snapshot->label) < 0) return 0;
    return replay_snapshot_restore(snapshot, &board, &gas, &status, &mode);
}

static int replay_file_timeline_valid(const CfReplayTimeline *timeline)
{
    int i;
    const CfReplaySnapshot *snapshot;

    if (timeline == 0 ||
        timeline->count < 1 ||
        timeline->count > CF_REPLAY_CAPACITY ||
        timeline->start < 0 ||
        timeline->start >= CF_REPLAY_CAPACITY ||
        timeline->total < (unsigned long)timeline->count ||
        (timeline->truncated != 0 && timeline->truncated != 1))
        return 0;

    if (timeline->truncated) {
        if (timeline->count != CF_REPLAY_CAPACITY ||
            timeline->total <= (unsigned long)timeline->count) return 0;
    } else if (timeline->total != (unsigned long)timeline->count) {
        return 0;
    }

    for (i = 0; i < timeline->count; ++i) {
        snapshot = replay_timeline_get(timeline, i);
        if (!replay_file_snapshot_valid(snapshot)) return 0;
    }
    return 1;
}

const char *replay_file_result_name(CfReplayFileResult result)
{
    switch (result) {
    case CF_REPLAY_FILE_OK: return "OK";
    case CF_REPLAY_FILE_NOT_FOUND: return "NOT FOUND";
    case CF_REPLAY_FILE_IO_ERROR: return "IO ERROR";
    case CF_REPLAY_FILE_BAD_MAGIC: return "BAD MAGIC";
    case CF_REPLAY_FILE_BAD_VERSION: return "BAD VERSION";
    case CF_REPLAY_FILE_BAD_DATA: return "BAD DATA";
    default: return "ERROR";
    }
}

CfReplayFileResult replay_file_save(const char *path,
                                    const CfReplayTimeline *timeline)
{
    FILE *fp;
    char temp[CF_REPLAY_PATH_BUFFER];
    const CfReplaySnapshot *snapshot;
    int i;
    int square;
    int label_length;
    int label_index;

    if (!replay_file_temp_path(path, temp) ||
        !replay_file_timeline_valid(timeline))
        return CF_REPLAY_FILE_BAD_DATA;

    fp = fopen(temp, "wt");
    if (fp == 0) return CF_REPLAY_FILE_IO_ERROR;

    if (fprintf(fp, "CHESSFART_REPLAY %d\n", CF_REPLAY_FILE_VERSION) < 0 ||
        fprintf(fp, "META %d %lu %d\n",
                timeline->count, timeline->total,
                timeline->truncated) < 0) {
        fclose(fp);
        (void)remove(temp);
        return CF_REPLAY_FILE_IO_ERROR;
    }

    for (i = 0; i < timeline->count; ++i) {
        snapshot = replay_timeline_get(timeline, i);
        if (snapshot == 0) {
            fclose(fp);
            (void)remove(temp);
            return CF_REPLAY_FILE_BAD_DATA;
        }

        if (fprintf(fp, "FRAME %d\n", i) < 0 ||
            fprintf(fp, "STATE %u %u %u %u %u %u %u %u %u %u\n",
                    (unsigned)snapshot->side_to_move,
                    (unsigned)snapshot->castling_rights,
                    (unsigned)snapshot->en_passant_file,
                    (unsigned)snapshot->en_passant_rank,
                    (unsigned)snapshot->halfmove_low,
                    (unsigned)snapshot->halfmove_high,
                    (unsigned)snapshot->fullmove_low,
                    (unsigned)snapshot->fullmove_high,
                    (unsigned)snapshot->status,
                    (unsigned)snapshot->match_mode) < 0 ||
            fprintf(fp, "SQUARES") < 0) {
            fclose(fp);
            (void)remove(temp);
            return CF_REPLAY_FILE_IO_ERROR;
        }

        for (square = 0; square < 64; ++square) {
            if (fprintf(fp, " %u", (unsigned)snapshot->squares[square]) < 0) {
                fclose(fp);
                (void)remove(temp);
                return CF_REPLAY_FILE_IO_ERROR;
            }
        }
        if (fprintf(fp, "\n") < 0) {
            fclose(fp);
            (void)remove(temp);
            return CF_REPLAY_FILE_IO_ERROR;
        }

        label_length = replay_file_label_length(snapshot->label);
        if (label_length < 0 ||
            fprintf(fp, "LABEL %d ", label_length) < 0) {
            fclose(fp);
            (void)remove(temp);
            return CF_REPLAY_FILE_BAD_DATA;
        }
        if (label_length == 0) {
            if (fprintf(fp, "-") < 0) {
                fclose(fp);
                (void)remove(temp);
                return CF_REPLAY_FILE_IO_ERROR;
            }
        } else {
            for (label_index = 0;
                 label_index < label_length;
                 ++label_index) {
                if (fprintf(fp, "%02X",
                            (unsigned)(unsigned char)
                            snapshot->label[label_index]) < 0) {
                    fclose(fp);
                    (void)remove(temp);
                    return CF_REPLAY_FILE_IO_ERROR;
                }
            }
        }

        if (fprintf(fp, "\nEND_FRAME\n") < 0) {
            fclose(fp);
            (void)remove(temp);
            return CF_REPLAY_FILE_IO_ERROR;
        }
    }

    if (fprintf(fp, "END\n") < 0 || fclose(fp) != 0) {
        (void)remove(temp);
        return CF_REPLAY_FILE_IO_ERROR;
    }

    return replay_file_replace(path, temp);
}

CfReplayFileResult replay_file_load(const char *path,
                                    CfReplayTimeline *timeline)
{
    FILE *fp;
    char magic[CF_REPLAY_WORD_BUFFER];
    char word[CF_REPLAY_WORD_BUFFER];
    char hex[CF_REPLAY_HEX_BUFFER];
    int version;
    int count;
    unsigned long total;
    int truncated;
    int frame_index;
    int i;
    int square;
    unsigned value;
    unsigned side;
    unsigned rights;
    unsigned ep_file;
    unsigned ep_rank;
    unsigned half_low;
    unsigned half_high;
    unsigned full_low;
    unsigned full_high;
    unsigned status;
    unsigned mode;
    int label_length;
    CfReplayTimeline loaded;
    CfReplaySnapshot *snapshot;

    if (path == 0 || timeline == 0) return CF_REPLAY_FILE_BAD_DATA;

    fp = fopen(path, "rt");
    if (fp == 0) return CF_REPLAY_FILE_NOT_FOUND;

    replay_timeline_init(&loaded);

    if (fscanf(fp, "%31s %d", magic, &version) != 2) {
        fclose(fp);
        return CF_REPLAY_FILE_BAD_MAGIC;
    }
    if (strcmp(magic, "CHESSFART_REPLAY") != 0) {
        fclose(fp);
        return CF_REPLAY_FILE_BAD_MAGIC;
    }
    if (version != CF_REPLAY_FILE_VERSION) {
        fclose(fp);
        return CF_REPLAY_FILE_BAD_VERSION;
    }

    if (fscanf(fp, "%31s %d %lu %d",
               word, &count, &total, &truncated) != 4 ||
        strcmp(word, "META") != 0 ||
        count < 1 || count > CF_REPLAY_CAPACITY ||
        total < (unsigned long)count ||
        (truncated != 0 && truncated != 1) ||
        (truncated && (count != CF_REPLAY_CAPACITY ||
                       total <= (unsigned long)count)) ||
        (!truncated && total != (unsigned long)count)) {
        fclose(fp);
        return CF_REPLAY_FILE_BAD_DATA;
    }

    loaded.start = 0;
    loaded.count = count;
    loaded.total = total;
    loaded.truncated = truncated;

    for (i = 0; i < count; ++i) {
        snapshot = &loaded.snapshots[i];
        memset(snapshot, 0, sizeof(*snapshot));

        if (fscanf(fp, "%31s %d", word, &frame_index) != 2 ||
            strcmp(word, "FRAME") != 0 ||
            frame_index != i) {
            fclose(fp);
            return CF_REPLAY_FILE_BAD_DATA;
        }

        if (fscanf(fp, "%31s %u %u %u %u %u %u %u %u %u %u",
                   word, &side, &rights, &ep_file, &ep_rank,
                   &half_low, &half_high, &full_low, &full_high,
                   &status, &mode) != 11 ||
            strcmp(word, "STATE") != 0 ||
            side > 255U || rights > 255U ||
            ep_file > 255U || ep_rank > 255U ||
            half_low > 65535U || half_high > 65535U ||
            full_low > 65535U || full_high > 65535U ||
            status > 255U || mode > 255U) {
            fclose(fp);
            return CF_REPLAY_FILE_BAD_DATA;
        }

        snapshot->side_to_move = (cf_u8)side;
        snapshot->castling_rights = (cf_u8)rights;
        snapshot->en_passant_file = (cf_u8)ep_file;
        snapshot->en_passant_rank = (cf_u8)ep_rank;
        snapshot->halfmove_low = (cf_u16)half_low;
        snapshot->halfmove_high = (cf_u16)half_high;
        snapshot->fullmove_low = (cf_u16)full_low;
        snapshot->fullmove_high = (cf_u16)full_high;
        snapshot->status = (cf_u8)status;
        snapshot->match_mode = (cf_u8)mode;

        if (fscanf(fp, "%31s", word) != 1 ||
            strcmp(word, "SQUARES") != 0) {
            fclose(fp);
            return CF_REPLAY_FILE_BAD_DATA;
        }
        for (square = 0; square < 64; ++square) {
            if (fscanf(fp, "%u", &value) != 1 || value > 255U) {
                fclose(fp);
                return CF_REPLAY_FILE_BAD_DATA;
            }
            snapshot->squares[square] = (cf_u8)value;
        }

        if (fscanf(fp, "%31s %d %80s",
                   word, &label_length, hex) != 3 ||
            strcmp(word, "LABEL") != 0 ||
            !replay_file_decode_label(snapshot, label_length, hex)) {
            fclose(fp);
            return CF_REPLAY_FILE_BAD_DATA;
        }

        if (fscanf(fp, "%31s", word) != 1 ||
            strcmp(word, "END_FRAME") != 0 ||
            !replay_file_snapshot_valid(snapshot)) {
            fclose(fp);
            return CF_REPLAY_FILE_BAD_DATA;
        }
    }

    if (fscanf(fp, "%31s", word) != 1 || strcmp(word, "END") != 0) {
        fclose(fp);
        return CF_REPLAY_FILE_BAD_DATA;
    }
    if (fscanf(fp, "%31s", word) == 1) {
        fclose(fp);
        return CF_REPLAY_FILE_BAD_DATA;
    }
    if (ferror(fp)) {
        fclose(fp);
        return CF_REPLAY_FILE_IO_ERROR;
    }
    if (fclose(fp) != 0) return CF_REPLAY_FILE_IO_ERROR;

    *timeline = loaded;
    return CF_REPLAY_FILE_OK;
}
