#include <stdio.h>
#include <string.h>

#include "board.h"
#include "gas.h"
#include "persistence.h"

static int failures;
#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static int game_equal(const CfBoard *a, const CfBoard *b,
                      const CfGasState *ga, const CfGasState *gb,
                      const CfGasHistory *ha, const CfGasHistory *hb)
{
    if (memcmp(a, b, sizeof(*a)) != 0) return 0;
    if (memcmp(ga, gb, sizeof(*ga)) != 0) return 0;
    if (ha->count != hb->count) return 0;
    return memcmp(ha->keys, hb->keys,
                  (size_t)ha->count * sizeof(ha->keys[0])) == 0;
}

static void make_saved_position(CfBoard *board, CfGasState *gas,
                                CfGasHistory *history)
{
    CfGasMove move;
    board_init_starting_position(board);
    gas_init(gas);
    gas_history_init(history, board, gas);

    CHECK(gas_make_move(board, gas, 4, 1, 4, 3, &move));
    gas_history_record(history, board, gas);
    CHECK(gas_make_move(board, gas, 4, 6, 4, 4, &move));
    gas_history_record(history, board, gas);
    CHECK(gas_make_move(board, gas, 6, 0, 5, 2, &move));
    gas_history_record(history, board, gas);
}

static void test_game_round_trip(void)
{
    CfBoard board;
    CfBoard loaded;
    CfGasState gas;
    CfGasState loaded_gas;
    CfGasHistory history;
    CfGasHistory loaded_history;
    CfPersistenceResult result;
    const char *path = "build/host/build9_roundtrip.sav";

    make_saved_position(&board, &gas, &history);
    result = persistence_save_game(path, &board, &gas, &history);
    CHECK(result == CF_PERSIST_OK);

    memset(&loaded, 0x55, sizeof(loaded));
    memset(&loaded_gas, 0x44, sizeof(loaded_gas));
    memset(&loaded_history, 0x33, sizeof(loaded_history));
    result = persistence_load_game(path, &loaded, &loaded_gas,
                                   &loaded_history);
    CHECK(result == CF_PERSIST_OK);
    CHECK(game_equal(&board, &loaded, &gas, &loaded_gas,
                     &history, &loaded_history));
    (void)remove(path);
}

static void test_failed_load_is_transactional(void)
{
    CfBoard board;
    CfBoard before;
    CfGasState gas;
    CfGasState before_gas;
    CfGasHistory history;
    CfGasHistory before_history;
    FILE *fp;
    CfPersistenceResult result;
    const char *path = "build/host/build9_bad.sav";

    make_saved_position(&board, &gas, &history);
    before = board;
    before_gas = gas;
    before_history = history;

    fp = fopen(path, "wt");
    CHECK(fp != 0);
    if (fp != 0) {
        fprintf(fp, "CHESSFART_SAVE 99\n");
        fclose(fp);
    }
    result = persistence_load_game(path, &board, &gas, &history);
    CHECK(result == CF_PERSIST_BAD_VERSION);
    CHECK(game_equal(&before, &board, &before_gas, &gas,
                     &before_history, &history));

    fp = fopen(path, "wt");
    CHECK(fp != 0);
    if (fp != 0) {
        fprintf(fp, "CHESSFART_SAVE 1\n");
        fprintf(fp, "STATE 1 15 -1 -1 0 1\nSQUARES\n");
        fclose(fp);
    }
    result = persistence_load_game(path, &board, &gas, &history);
    CHECK(result == CF_PERSIST_BAD_DATA);
    CHECK(game_equal(&before, &board, &before_gas, &gas,
                     &before_history, &history));
    (void)remove(path);
}

static void test_missing_and_bad_source_state(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;

    make_saved_position(&board, &gas, &history);
    CHECK(persistence_load_game("build/host/no_such_build9.sav",
                                &board, &gas, &history) ==
          CF_PERSIST_NOT_FOUND);

    gas.squares[0][0] = 4U;
    CHECK(persistence_save_game("build/host/build9_invalid.sav",
                                &board, &gas, &history) ==
          CF_PERSIST_BAD_DATA);
    (void)remove("build/host/build9_invalid.sav");
}

static void test_config_round_trip(void)
{
    CfAudioConfig config;
    CfAudioConfig loaded;
    FILE *fp;
    const char *path = "build/host/build9_test.cfg";

    config.device = CF_AUDIO_DEVICE_PC_SPEAKER;
    config.sfx_level = CF_AUDIO_LEVEL_HIGH;
    config.music_level = CF_AUDIO_LEVEL_LOW;
    CHECK(persistence_save_config(path, &config) == CF_PERSIST_OK);

    loaded.device = CF_AUDIO_DEVICE_NONE;
    loaded.sfx_level = CF_AUDIO_LEVEL_OFF;
    loaded.music_level = CF_AUDIO_LEVEL_OFF;
    CHECK(persistence_load_config(path, &loaded) == CF_PERSIST_OK);
    CHECK(loaded.device == config.device);
    CHECK(loaded.sfx_level == config.sfx_level);
    CHECK(loaded.music_level == config.music_level);

    fp = fopen(path, "wt");
    CHECK(fp != 0);
    if (fp != 0) {
        fprintf(fp, "CHESSFART_CONFIG 2\nAUDIO 0 2 0\nEND\n");
        fclose(fp);
    }
    CHECK(persistence_load_config(path, &loaded) ==
          CF_PERSIST_BAD_VERSION);
    (void)remove(path);
}

int main(void)
{
    test_game_round_trip();
    test_failed_load_is_transactional();
    test_missing_and_bad_source_state();
    test_config_round_trip();

    if (failures != 0) {
        printf("Build 9 persistence tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 9 persistence tests passed.\n");
    return 0;
}
