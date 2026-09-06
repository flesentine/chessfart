#include <stdio.h>
#include <string.h>

#include "replay.h"

static int failures;
#define CHECK(expr) do { if (!(expr)) {     printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; } } while (0)

static void test_memory_budget(void)
{
    CHECK(sizeof(CfReplayTimeline) < 32768U);
    CHECK(sizeof(CfReplaySnapshot) < 128U);
    CHECK((unsigned long)sizeof(CfReplayTimeline) * 2UL < 65536UL);
}

static void test_move_round_trip(void)
{
    CfBoard board;
    CfBoard restored;
    CfGasState gas;
    CfGasState restored_gas;
    CfGasMove move;
    CfReplayTimeline timeline;
    const CfReplaySnapshot *snapshot;
    CfGameStatus status = CF_GAME_ONGOING;
    CfMatchMode mode = CF_MATCH_CPU;
    int file;
    int rank;

    board_init_starting_position(&board);
    gas_init(&gas);
    replay_timeline_reset(&timeline, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_LOCAL, "START");

    CHECK(timeline.count == 1);
    CHECK(timeline.total == 1UL);
    CHECK(!timeline.truncated);
    snapshot = replay_timeline_get(&timeline, 0);
    CHECK(snapshot != 0);
    CHECK(strcmp(snapshot->label, "START") == 0);
    CHECK(replay_snapshot_matches(snapshot, &board, &gas,
                                  CF_GAME_ONGOING, CF_MATCH_LOCAL));

    CHECK(gas_make_move(&board, &gas, 4, 1, 4, 3, &move));
    replay_timeline_record(&timeline, &board, &gas, CF_GAME_ONGOING,
                           CF_MATCH_LOCAL, "WHITE E2-E4");

    CHECK(timeline.count == 2);
    CHECK(timeline.total == 2UL);
    snapshot = replay_timeline_get(&timeline, 1);
    CHECK(snapshot != 0);
    CHECK(strcmp(snapshot->label, "WHITE E2-E4") == 0);
    CHECK(replay_snapshot_matches(snapshot, &board, &gas,
                                  CF_GAME_ONGOING, CF_MATCH_LOCAL));

    snapshot = replay_timeline_get(&timeline, 0);
    CHECK(replay_snapshot_restore(snapshot, &restored, &restored_gas,
                                  &status, &mode));
    CHECK(status == CF_GAME_ONGOING);
    CHECK(mode == CF_MATCH_LOCAL);
    CHECK(restored.side_to_move == CF_COLOR_WHITE);
    CHECK(restored.fullmove_number == 1U);
    CHECK(restored.castling_rights == CF_CASTLE_ALL);
    CHECK(restored.en_passant_file == -1);
    CHECK(restored.en_passant_rank == -1);

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            CHECK(gas_at(&restored_gas, file, rank) == 0U);
        }
    }
    CHECK(board_piece_at(&restored, 4, 1)->type == CF_PIECE_PAWN);
    CHECK(board_piece_at(&restored, 4, 3)->type == CF_PIECE_NONE);
}

static void test_fart_round_trip(void)
{
    CfBoard board;
    CfBoard restored;
    CfGasState gas;
    CfGasState restored_gas;
    CfFartAction fart;
    CfReplayTimeline timeline;
    const CfReplaySnapshot *snapshot;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 3, 3, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 4, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 3, 3, 3U);
    gas_set(&gas, 3, 4, 2U);

    replay_timeline_reset(&timeline, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_LOCAL, "START");
    CHECK(gas_make_fart(&board, &gas, 3, 3, CF_FART_N,
                        CF_PIECE_QUEEN, &fart));
    replay_timeline_record(&timeline, &board, &gas, CF_GAME_ONGOING,
                           CF_MATCH_LOCAL, "WHITE FART D4 N PUSH");

    snapshot = replay_timeline_get(&timeline, 1);
    CHECK(snapshot != 0);
    CHECK(replay_snapshot_restore(snapshot, &restored, &restored_gas, 0, 0));
    CHECK(board_piece_at(&restored, 3, 4)->type == CF_PIECE_NONE);
    CHECK(board_piece_at(&restored, 3, 5)->type == CF_PIECE_PAWN);
    CHECK(board_piece_at(&restored, 3, 5)->color == CF_COLOR_BLACK);
    CHECK(gas_at(&restored_gas, 3, 3) == 1U);
    CHECK(gas_at(&restored_gas, 3, 5) == 2U);
}

static void test_ring_and_labels(void)
{
    CfBoard board;
    CfGasState gas;
    CfReplayTimeline timeline;
    const CfReplaySnapshot *first;
    const CfReplaySnapshot *last;
    char long_label[96];
    int i;

    board_init_starting_position(&board);
    gas_init(&gas);
    replay_timeline_reset(&timeline, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_CPU, "START");

    for (i = 0; i < 300; ++i) {
        board.fullmove_number = (unsigned)(i + 2);
        replay_timeline_record(&timeline, &board, &gas, CF_GAME_ONGOING,
                               CF_MATCH_CPU, "FRAME");
    }

    CHECK(timeline.count == CF_REPLAY_CAPACITY);
    CHECK(timeline.total == 301UL);
    CHECK(timeline.truncated);
    first = replay_timeline_get(&timeline, 0);
    last = replay_timeline_get(&timeline, CF_REPLAY_CAPACITY - 1);
    CHECK(first != 0);
    CHECK(last != 0);
    if (first != 0) {
        CfBoard restored;
        CfGasState restored_gas;
        CHECK(replay_snapshot_restore(first, &restored, &restored_gas, 0, 0));
        CHECK(restored.fullmove_number == 46U);
    }
    if (last != 0) {
        CfBoard restored;
        CfGasState restored_gas;
        CHECK(replay_snapshot_restore(last, &restored, &restored_gas, 0, 0));
        CHECK(restored.fullmove_number == 301U);
    }
    CHECK(replay_timeline_get(&timeline, -1) == 0);
    CHECK(replay_timeline_get(&timeline, CF_REPLAY_CAPACITY) == 0);

    memset(long_label, 'X', sizeof(long_label));
    long_label[sizeof(long_label) - 1U] = '\0';
    replay_timeline_reset(&timeline, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_CPU, long_label);
    first = replay_timeline_get(&timeline, 0);
    CHECK(first != 0);
    CHECK(strlen(first->label) == CF_REPLAY_LABEL - 1U);
    CHECK(first->label[CF_REPLAY_LABEL - 1U] == '\0');
}

static void test_large_clock_round_trip(void)
{
    CfBoard board;
    CfBoard restored;
    CfGasState gas;
    CfGasState restored_gas;
    CfReplayTimeline timeline;

    board_init_starting_position(&board);
    gas_init(&gas);
    board.halfmove_clock = 70000U;
    board.fullmove_number = 80000U;
    replay_timeline_reset(&timeline, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_CPU, "LARGE CLOCKS");

    CHECK(replay_snapshot_restore(replay_timeline_get(&timeline, 0),
                                  &restored, &restored_gas, 0, 0));
    CHECK(restored.halfmove_clock == 70000U);
    CHECK(restored.fullmove_number == 80000U);
}

static void test_invalid_snapshot_rejected(void)
{
    CfBoard board;
    CfBoard restored;
    CfGasState gas;
    CfGasState restored_gas;
    CfReplayTimeline timeline;
    CfReplaySnapshot bad;

    board_init_starting_position(&board);
    gas_init(&gas);
    replay_timeline_reset(&timeline, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_CPU, "START");
    bad = *replay_timeline_get(&timeline, 0);

    bad.match_mode = 99U;
    CHECK(!replay_snapshot_restore(&bad, &restored, &restored_gas, 0, 0));

    bad = *replay_timeline_get(&timeline, 0);
    bad.squares[0] = 7U;
    CHECK(!replay_snapshot_restore(&bad, &restored, &restored_gas, 0, 0));

    bad = *replay_timeline_get(&timeline, 0);
    bad.squares[0] |= 0x80U;
    CHECK(!replay_snapshot_restore(&bad, &restored, &restored_gas, 0, 0));

    bad = *replay_timeline_get(&timeline, 0);
    bad.squares[0] = 0x20U;
    CHECK(!replay_snapshot_restore(&bad, &restored, &restored_gas, 0, 0));

    bad = *replay_timeline_get(&timeline, 0);
    bad.fullmove_low = 0U;
    bad.fullmove_high = 0U;
    CHECK(!replay_snapshot_restore(&bad, &restored, &restored_gas, 0, 0));

    bad = *replay_timeline_get(&timeline, 0);
    bad.en_passant_file = 5U;
    bad.en_passant_rank = 0U;
    CHECK(!replay_snapshot_restore(&bad, &restored, &restored_gas, 0, 0));
}

int main(void)
{
    test_memory_budget();
    test_move_round_trip();
    test_fart_round_trip();
    test_ring_and_labels();
    test_large_clock_round_trip();
    test_invalid_snapshot_rejected();

    if (failures != 0) {
        printf("Build 15 replay tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 15 replay tests passed.\n");
    return 0;
}
