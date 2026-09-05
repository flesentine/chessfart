#include <stdio.h>
#include <string.h>

#include "replay_file.h"

#define TEST_REPLAY_PATH "build/host/test_build15_file.rpl"

static int failures;
#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static int write_text(const char *text)
{
    FILE *fp = fopen(TEST_REPLAY_PATH, "wt");
    if (fp == 0) return 0;
    if (fputs(text, fp) == EOF) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int append_text(const char *text)
{
    FILE *fp = fopen(TEST_REPLAY_PATH, "at");
    if (fp == 0) return 0;
    if (fputs(text, fp) == EOF) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void build_two_frame_timeline(CfReplayTimeline *timeline)
{
    CfBoard board;
    CfGasState gas;
    CfGasMove move;

    board_init_starting_position(&board);
    gas_init(&gas);
    replay_timeline_reset(timeline, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_LOCAL, "START / LOCAL");
    CHECK(gas_make_move(&board, &gas, 4, 1, 4, 3, &move));
    replay_timeline_record(timeline, &board, &gas, CF_GAME_ONGOING,
                           CF_MATCH_LOCAL, "WHITE E2-E4 +1 GAS");
}

static void test_file_round_trip(void)
{
    CfReplayTimeline source;
    CfReplayTimeline loaded;
    const CfReplaySnapshot *a;
    const CfReplaySnapshot *b;
    int i;

    build_two_frame_timeline(&source);
    replay_timeline_init(&loaded);

    CHECK(replay_file_save(TEST_REPLAY_PATH, &source) == CF_REPLAY_FILE_OK);
    CHECK(replay_file_load(TEST_REPLAY_PATH, &loaded) == CF_REPLAY_FILE_OK);
    CHECK(loaded.start == 0);
    CHECK(loaded.count == source.count);
    CHECK(loaded.total == source.total);
    CHECK(loaded.truncated == source.truncated);

    for (i = 0; i < source.count; ++i) {
        a = replay_timeline_get(&source, i);
        b = replay_timeline_get(&loaded, i);
        CHECK(a != 0);
        CHECK(b != 0);
        if (a != 0 && b != 0)
            CHECK(memcmp(a, b, sizeof(*a)) == 0);
    }

    CHECK(strcmp(replay_timeline_get(&loaded, 0)->label,
                 "START / LOCAL") == 0);
    CHECK(strcmp(replay_timeline_get(&loaded, 1)->label,
                 "WHITE E2-E4 +1 GAS") == 0);
}

static void test_ring_file_round_trip(void)
{
    CfBoard board;
    CfGasState gas;
    CfReplayTimeline source;
    CfReplayTimeline loaded;
    CfBoard restored;
    CfGasState restored_gas;
    int i;

    board_init_starting_position(&board);
    gas_init(&gas);
    replay_timeline_reset(&source, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_CPU, "FRAME 0");

    for (i = 1; i < 600; ++i) {
        board.fullmove_number = (unsigned)(i + 1);
        replay_timeline_record(&source, &board, &gas, CF_GAME_ONGOING,
                               CF_MATCH_CPU, "RING FRAME");
    }

    CHECK(source.count == CF_REPLAY_CAPACITY);
    CHECK(source.total == 600UL);
    CHECK(source.truncated == 1);
    CHECK(replay_file_save(TEST_REPLAY_PATH, &source) == CF_REPLAY_FILE_OK);
    CHECK(replay_file_load(TEST_REPLAY_PATH, &loaded) == CF_REPLAY_FILE_OK);
    CHECK(loaded.start == 0);
    CHECK(loaded.count == CF_REPLAY_CAPACITY);
    CHECK(loaded.total == 600UL);
    CHECK(loaded.truncated == 1);

    CHECK(replay_snapshot_restore(replay_timeline_get(&loaded, 0),
                                  &restored, &restored_gas, 0, 0));
    CHECK(restored.fullmove_number == 345U);
    CHECK(replay_snapshot_restore(
              replay_timeline_get(&loaded, CF_REPLAY_CAPACITY - 1),
              &restored, &restored_gas, 0, 0));
    CHECK(restored.fullmove_number == 600U);
}

static void expect_failed_load(CfReplayFileResult expected)
{
    CfReplayTimeline target;
    CfReplayTimeline before;
    CfBoard board;
    CfGasState gas;
    CfReplayFileResult result;

    board_init_starting_position(&board);
    gas_init(&gas);
    replay_timeline_reset(&target, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_CPU, "KEEP ME");
    before = target;

    result = replay_file_load(TEST_REPLAY_PATH, &target);
    CHECK(result == expected);
    CHECK(memcmp(&target, &before, sizeof(target)) == 0);
}

static void write_bad_snapshot_file(unsigned bad_square,
                                    const char *label_line)
{
    FILE *fp;
    int i;

    fp = fopen(TEST_REPLAY_PATH, "wt");
    CHECK(fp != 0);
    if (fp == 0) return;

    fprintf(fp, "CHESSFART_REPLAY 1\n");
    fprintf(fp, "META 1 1 0\n");
    fprintf(fp, "FRAME 0\n");
    fprintf(fp, "STATE 1 0 0 0 0 0 1 0 0 0\n");
    fprintf(fp, "SQUARES");
    for (i = 0; i < 64; ++i)
        fprintf(fp, " %u", i == 0 ? bad_square : 0U);
    fprintf(fp, "\n%s\nEND_FRAME\nEND\n", label_line);
    CHECK(fclose(fp) == 0);
}

static void test_transactional_rejection(void)
{
    CfReplayTimeline valid;

    CHECK(write_text("NOPE 1\n"));
    expect_failed_load(CF_REPLAY_FILE_BAD_MAGIC);

    CHECK(write_text("CHESSFART_REPLAY 99\n"));
    expect_failed_load(CF_REPLAY_FILE_BAD_VERSION);

    CHECK(write_text("CHESSFART_REPLAY 1\nMETA 0 0 0\n"));
    expect_failed_load(CF_REPLAY_FILE_BAD_DATA);

    CHECK(write_text("CHESSFART_REPLAY 1\nMETA 1 2 1\n"));
    expect_failed_load(CF_REPLAY_FILE_BAD_DATA);

    write_bad_snapshot_file(128U, "LABEL 0 -");
    expect_failed_load(CF_REPLAY_FILE_BAD_DATA);

    write_bad_snapshot_file(32U, "LABEL 0 -");
    expect_failed_load(CF_REPLAY_FILE_BAD_DATA);

    write_bad_snapshot_file(0U, "LABEL 1 GG");
    expect_failed_load(CF_REPLAY_FILE_BAD_DATA);

    build_two_frame_timeline(&valid);
    CHECK(replay_file_save(TEST_REPLAY_PATH, &valid) == CF_REPLAY_FILE_OK);
    CHECK(append_text("TRAILING_JUNK\n"));
    expect_failed_load(CF_REPLAY_FILE_BAD_DATA);
}

static void test_bad_save_inputs(void)
{
    CfReplayTimeline empty;
    CfReplayTimeline bad;
    CfBoard board;
    CfGasState gas;

    replay_timeline_init(&empty);
    CHECK(replay_file_save(TEST_REPLAY_PATH, &empty) ==
          CF_REPLAY_FILE_BAD_DATA);
    CHECK(replay_file_save("", &empty) == CF_REPLAY_FILE_BAD_DATA);

    board_init_starting_position(&board);
    gas_init(&gas);
    replay_timeline_reset(&bad, &board, &gas, CF_GAME_ONGOING,
                          CF_MATCH_CPU, "START");
    bad.total = 2UL;
    CHECK(replay_file_save(TEST_REPLAY_PATH, &bad) ==
          CF_REPLAY_FILE_BAD_DATA);
}

static void test_result_names(void)
{
    CHECK(strcmp(replay_file_result_name(CF_REPLAY_FILE_OK), "OK") == 0);
    CHECK(strcmp(replay_file_result_name(CF_REPLAY_FILE_BAD_MAGIC),
                 "BAD MAGIC") == 0);
    CHECK(strcmp(replay_file_result_name(CF_REPLAY_FILE_BAD_VERSION),
                 "BAD VERSION") == 0);
    CHECK(strcmp(replay_file_result_name(CF_REPLAY_FILE_BAD_DATA),
                 "BAD DATA") == 0);
}

int main(void)
{
    (void)remove(TEST_REPLAY_PATH);
    test_file_round_trip();
    test_ring_file_round_trip();
    test_transactional_rejection();
    test_bad_save_inputs();
    test_result_names();
    (void)remove(TEST_REPLAY_PATH);
    (void)remove(TEST_REPLAY_PATH ".TMP");

    if (failures != 0) {
        printf("Build 15 replay-file tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 15 replay-file tests passed.\n");
    return 0;
}
