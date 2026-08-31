#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "persistence.h"

#define CF_PATH_BUFFER 128

static int valid_path(const char *path)
{
    size_t len;
    if (path == 0 || path[0] == '\0') return 0;
    len = strlen(path);
    return len + 5U < CF_PATH_BUFFER;
}

static int make_temp_path(const char *path, char *temp)
{
    if (!valid_path(path) || temp == 0) return 0;
    strcpy(temp, path);
    strcat(temp, ".TMP");
    return 1;
}

static CfPersistenceResult replace_with_temp(const char *path,
                                             const char *temp)
{
    (void)remove(path);
    if (rename(temp, path) != 0) {
        (void)remove(temp);
        return CF_PERSIST_IO_ERROR;
    }
    return CF_PERSIST_OK;
}

static int valid_piece(int type, int color, int gas)
{
    if (type < (int)CF_PIECE_NONE || type > (int)CF_PIECE_KING) return 0;
    if (gas < 0 || gas > (int)CF_GAS_MAX) return 0;
    if (type == (int)CF_PIECE_NONE)
        return color == (int)CF_COLOR_NONE && gas == 0;
    return color == (int)CF_COLOR_WHITE || color == (int)CF_COLOR_BLACK;
}

static int valid_history_byte(unsigned value)
{
    unsigned piece = value & 0x0FU;
    unsigned gas = (value >> 4) & 0x03U;
    if ((value & 0xC0U) != 0U) return 0;
    if (piece > 12U) return 0;
    if (piece == 0U && gas != 0U) return 0;
    return 1;
}

static int valid_board_state(int side, unsigned rights,
                             int ep_file, int ep_rank)
{
    if (side != (int)CF_COLOR_WHITE && side != (int)CF_COLOR_BLACK) return 0;
    if ((rights & ~CF_CASTLE_ALL) != 0U) return 0;
    if (ep_file == -1 || ep_rank == -1)
        return ep_file == -1 && ep_rank == -1;
    return ep_file >= 0 && ep_file < 8 && ep_rank >= 0 && ep_rank < 8;
}

const char *persistence_result_name(CfPersistenceResult result)
{
    switch (result) {
    case CF_PERSIST_OK: return "OK";
    case CF_PERSIST_NOT_FOUND: return "NOT FOUND";
    case CF_PERSIST_IO_ERROR: return "IO ERROR";
    case CF_PERSIST_BAD_MAGIC: return "BAD MAGIC";
    case CF_PERSIST_BAD_VERSION: return "BAD VERSION";
    case CF_PERSIST_BAD_DATA: return "BAD DATA";
    default: return "ERROR";
    }
}

CfPersistenceResult persistence_save_game(const char *path,
                                          const CfBoard *board,
                                          const CfGasState *gas,
                                          const CfGasHistory *history)
{
    FILE *fp;
    char temp[CF_PATH_BUFFER];
    int rank;
    int file;
    int i;
    int square;

    if (board == 0 || gas == 0 || history == 0 ||
        !make_temp_path(path, temp)) return CF_PERSIST_BAD_DATA;
    if (history->count < 1 || history->count > CF_GAS_HISTORY)
        return CF_PERSIST_BAD_DATA;

    fp = fopen(temp, "wt");
    if (fp == 0) return CF_PERSIST_IO_ERROR;

    if (fprintf(fp, "CHESSFART_SAVE %d\n", CF_SAVE_VERSION) < 0 ||
        fprintf(fp, "STATE %d %u %d %d %u %u\n",
                (int)board->side_to_move, board->castling_rights,
                board->en_passant_file, board->en_passant_rank,
                board->halfmove_clock, board->fullmove_number) < 0 ||
        fprintf(fp, "SQUARES\n") < 0) {
        fclose(fp);
        (void)remove(temp);
        return CF_PERSIST_IO_ERROR;
    }

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (!valid_piece((int)board->squares[rank][file].type,
                             (int)board->squares[rank][file].color,
                             (int)gas->squares[rank][file]) ||
                fprintf(fp, "%d %d %u\n",
                        (int)board->squares[rank][file].type,
                        (int)board->squares[rank][file].color,
                        (unsigned)gas->squares[rank][file]) < 0) {
                fclose(fp);
                (void)remove(temp);
                return CF_PERSIST_BAD_DATA;
            }
        }
    }

    if (fprintf(fp, "HISTORY %d\n", history->count) < 0) {
        fclose(fp);
        (void)remove(temp);
        return CF_PERSIST_IO_ERROR;
    }
    for (i = 0; i < history->count; ++i) {
        if (fprintf(fp, "KEY %u %u\n",
                    (unsigned)history->keys[i].state,
                    (unsigned)history->keys[i].en_passant_file) < 0) {
            fclose(fp);
            (void)remove(temp);
            return CF_PERSIST_IO_ERROR;
        }
        for (square = 0; square < 64; ++square) {
            if (fprintf(fp, "%u%c",
                        (unsigned)history->keys[i].squares[square],
                        square == 63 ? '\n' : ' ') < 0) {
                fclose(fp);
                (void)remove(temp);
                return CF_PERSIST_IO_ERROR;
            }
        }
    }

    if (fprintf(fp, "END\n") < 0 || fclose(fp) != 0) {
        (void)remove(temp);
        return CF_PERSIST_IO_ERROR;
    }
    return replace_with_temp(path, temp);
}

CfPersistenceResult persistence_load_game(const char *path,
                                          CfBoard *board,
                                          CfGasState *gas,
                                          CfGasHistory *history)
{
    FILE *fp;
    char magic[32];
    char word[32];
    int version;
    int side;
    unsigned rights;
    int ep_file;
    int ep_rank;
    unsigned halfmove;
    unsigned fullmove;
    int rank;
    int file;
    int type;
    int color;
    unsigned gas_value;
    int history_count;
    int i;
    int square;
    unsigned state;
    unsigned history_ep;
    unsigned history_byte;
    CfBoard loaded_board;
    CfGasState loaded_gas;
    CfGasHistory loaded_history;

    if (path == 0 || board == 0 || gas == 0 || history == 0)
        return CF_PERSIST_BAD_DATA;

    fp = fopen(path, "rt");
    if (fp == 0) return CF_PERSIST_NOT_FOUND;

    memset(&loaded_board, 0, sizeof(loaded_board));
    memset(&loaded_gas, 0, sizeof(loaded_gas));
    memset(&loaded_history, 0, sizeof(loaded_history));

    if (fscanf(fp, "%31s %d", magic, &version) != 2) {
        fclose(fp);
        return CF_PERSIST_BAD_MAGIC;
    }
    if (strcmp(magic, "CHESSFART_SAVE") != 0) {
        fclose(fp);
        return CF_PERSIST_BAD_MAGIC;
    }
    if (version != CF_SAVE_VERSION) {
        fclose(fp);
        return CF_PERSIST_BAD_VERSION;
    }

    if (fscanf(fp, "%31s %d %u %d %d %u %u",
               word, &side, &rights, &ep_file, &ep_rank,
               &halfmove, &fullmove) != 7 ||
        strcmp(word, "STATE") != 0 ||
        !valid_board_state(side, rights, ep_file, ep_rank) ||
        fullmove == 0U) {
        fclose(fp);
        return CF_PERSIST_BAD_DATA;
    }

    loaded_board.side_to_move = (CfPieceColor)side;
    loaded_board.castling_rights = rights;
    loaded_board.en_passant_file = ep_file;
    loaded_board.en_passant_rank = ep_rank;
    loaded_board.halfmove_clock = halfmove;
    loaded_board.fullmove_number = fullmove;

    if (fscanf(fp, "%31s", word) != 1 || strcmp(word, "SQUARES") != 0) {
        fclose(fp);
        return CF_PERSIST_BAD_DATA;
    }

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (fscanf(fp, "%d %d %u", &type, &color, &gas_value) != 3 ||
                !valid_piece(type, color, (int)gas_value)) {
                fclose(fp);
                return CF_PERSIST_BAD_DATA;
            }
            loaded_board.squares[rank][file].type = (CfPieceType)type;
            loaded_board.squares[rank][file].color = (CfPieceColor)color;
            loaded_gas.squares[rank][file] = (cf_u8)gas_value;
        }
    }

    if (fscanf(fp, "%31s %d", word, &history_count) != 2 ||
        strcmp(word, "HISTORY") != 0 ||
        history_count < 1 || history_count > CF_GAS_HISTORY) {
        fclose(fp);
        return CF_PERSIST_BAD_DATA;
    }
    loaded_history.count = history_count;

    for (i = 0; i < history_count; ++i) {
        if (fscanf(fp, "%31s %u %u", word, &state, &history_ep) != 3 ||
            strcmp(word, "KEY") != 0 ||
            state > 31U || history_ep > 8U) {
            fclose(fp);
            return CF_PERSIST_BAD_DATA;
        }
        loaded_history.keys[i].state = (cf_u8)state;
        loaded_history.keys[i].en_passant_file = (cf_u8)history_ep;
        for (square = 0; square < 64; ++square) {
            if (fscanf(fp, "%u", &history_byte) != 1 ||
                history_byte > 255U || !valid_history_byte(history_byte)) {
                fclose(fp);
                return CF_PERSIST_BAD_DATA;
            }
            loaded_history.keys[i].squares[square] = (cf_u8)history_byte;
        }
    }

    if (fscanf(fp, "%31s", word) != 1 || strcmp(word, "END") != 0) {
        fclose(fp);
        return CF_PERSIST_BAD_DATA;
    }
    fclose(fp);

    *board = loaded_board;
    *gas = loaded_gas;
    *history = loaded_history;
    return CF_PERSIST_OK;
}

CfPersistenceResult persistence_save_config(const char *path,
                                            const CfAudioConfig *config)
{
    FILE *fp;
    char temp[CF_PATH_BUFFER];

    if (config == 0 || !make_temp_path(path, temp)) return CF_PERSIST_BAD_DATA;
    if ((int)config->device < (int)CF_AUDIO_DEVICE_AUTO ||
        (int)config->device > (int)CF_AUDIO_DEVICE_NONE ||
        (int)config->sfx_level < (int)CF_AUDIO_LEVEL_OFF ||
        (int)config->sfx_level > (int)CF_AUDIO_LEVEL_HIGH ||
        (int)config->music_level < (int)CF_AUDIO_LEVEL_OFF ||
        (int)config->music_level > (int)CF_AUDIO_LEVEL_HIGH)
        return CF_PERSIST_BAD_DATA;

    fp = fopen(temp, "wt");
    if (fp == 0) return CF_PERSIST_IO_ERROR;
    if (fprintf(fp, "CHESSFART_CONFIG %d\n", CF_CONFIG_VERSION) < 0 ||
        fprintf(fp, "AUDIO %d %d %d\n",
                (int)config->device,
                (int)config->sfx_level,
                (int)config->music_level) < 0 ||
        fprintf(fp, "END\n") < 0) {
        fclose(fp);
        (void)remove(temp);
        return CF_PERSIST_IO_ERROR;
    }
    if (fclose(fp) != 0) {
        (void)remove(temp);
        return CF_PERSIST_IO_ERROR;
    }
    return replace_with_temp(path, temp);
}

CfPersistenceResult persistence_load_config(const char *path,
                                            CfAudioConfig *config)
{
    FILE *fp;
    char magic[32];
    char word[32];
    int version;
    int device;
    int sfx;
    int music;
    CfAudioConfig loaded;

    if (path == 0 || config == 0) return CF_PERSIST_BAD_DATA;
    fp = fopen(path, "rt");
    if (fp == 0) return CF_PERSIST_NOT_FOUND;

    if (fscanf(fp, "%31s %d", magic, &version) != 2 ||
        strcmp(magic, "CHESSFART_CONFIG") != 0) {
        fclose(fp);
        return CF_PERSIST_BAD_MAGIC;
    }
    if (version != CF_CONFIG_VERSION) {
        fclose(fp);
        return CF_PERSIST_BAD_VERSION;
    }
    if (fscanf(fp, "%31s %d %d %d", word, &device, &sfx, &music) != 4 ||
        strcmp(word, "AUDIO") != 0 ||
        device < (int)CF_AUDIO_DEVICE_AUTO ||
        device > (int)CF_AUDIO_DEVICE_NONE ||
        sfx < (int)CF_AUDIO_LEVEL_OFF ||
        sfx > (int)CF_AUDIO_LEVEL_HIGH ||
        music < (int)CF_AUDIO_LEVEL_OFF ||
        music > (int)CF_AUDIO_LEVEL_HIGH ||
        fscanf(fp, "%31s", word) != 1 ||
        strcmp(word, "END") != 0) {
        fclose(fp);
        return CF_PERSIST_BAD_DATA;
    }
    fclose(fp);

    loaded.device = (CfAudioDevice)device;
    loaded.sfx_level = (CfAudioLevel)sfx;
    loaded.music_level = (CfAudioLevel)music;
    *config = loaded;
    return CF_PERSIST_OK;
}
