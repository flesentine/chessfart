CC ?= cc
CFLAGS ?= -std=c89 -pedantic -Wall -Wextra -Werror -O2 -Iinclude

HOST_SOURCES = \
	src/main.c \
	src/game/board.c \
	src/game/gas.c \
	src/game/practice_undo.c \
	src/game/cpu_config.c \
	src/game/cpu_actions.c \
	src/game/cpu_eval.c \
	src/game/cpu_search.c \
	src/game/cpu_format.c \
	src/game/ux.c \
	src/game/replay.c \
	src/game/replay_file.c \
	src/game/font.c \
	src/game/ui_assets.c \
	src/game/ui_theme.c \
	src/game/board_view_art.c \
	src/game/presentation.c \
	src/game/audio.c \
	src/game/audio_samples.c \
	src/game/audio_game.c \
	src/game/persistence.c \
	src/platform/host/vga_host.c \
	src/platform/host/input_host.c \
	src/platform/host/audio_host.c \
	src/platform/host/mouse_host.c

CPU_SOURCES = \
	src/game/board.c \
	src/game/gas.c \
	src/game/cpu_config.c \
	src/game/cpu_actions.c \
	src/game/cpu_eval.c \
	src/game/cpu_search.c \
	src/game/cpu_format.c

HOST_BINARY = build/host/chessfart_host
HOST_PREVIEW = build/host/chessfart.ppm
HOST_TITLE_PREVIEW = build/host/chessfart_title.ppm
HOST_ATTRACT_PREVIEW = build/host/chessfart_attract.ppm
HOST_HELP_PREVIEW = build/host/chessfart_help.ppm
HOST_HISTORY_PREVIEW = build/host/chessfart_history.ppm
HOST_CREDITS_PREVIEW = build/host/chessfart_credits.ppm
HOST_TERMINAL_PREVIEW = build/host/chessfart_terminal.ppm
HOST_AUDIO_LOG = build/host/chessfart_audio.log
HOST_FART_WAV = build/host/chessfart_fart.wav
HOST_SAVE = build/host/CHESSFRT.SAV
HOST_CONFIG = build/host/CHESSFRT.CFG
PROFILE_REPORT = build/host/PROFILE.txt
SIZE_REPORT = build/host/SIZE.txt
TEST4_BINARY = build/host/test_build4
TEST5_BINARY = build/host/test_build5
TEST6_BINARY = build/host/test_build6
TEST8_BINARY = build/host/test_build8
TEST9_BINARY = build/host/test_build9
TEST10_BINARY = build/host/test_build10
TEST10_ALPHA_BINARY = build/host/test_build10_alpha_beta
TEST11_BINARY = build/host/test_build11
TEST15_BINARY = build/host/test_build15
TEST15_FILE_BINARY = build/host/test_build15_replay_file
TEST16_BINARY = build/host/test_build16
TEST17_BINARY = build/host/test_build17
PROFILE_BINARY = build/host/profile_runtime

.PHONY: all host host-run test profile release-audit dos release clean

all: host

host: $(HOST_BINARY)

$(HOST_BINARY): $(HOST_SOURCES) include/cf_types.h include/vga.h include/input.h include/font.h include/board.h include/gas.h include/practice_undo.h include/cpu.h include/ux.h include/match_mode.h include/mouse.h include/board_view.h include/ui_assets.h include/ui_theme.h include/presentation.h include/audio.h include/audio_platform.h include/audio_game.h include/persistence.h include/replay.h include/replay_file.h src/main_loop.inc src/main_hooks.inc src/main_ui.inc
	mkdir -p build/host
	$(CC) $(CFLAGS) -DCF_HOST_DEMO_FIXTURE -DCF_HOST_BUILD $(HOST_SOURCES) -o $(HOST_BINARY)

$(TEST4_BINARY): tests/test_build4.c src/game/board.c include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build4.c src/game/board.c -o $(TEST4_BINARY)

$(TEST5_BINARY): tests/test_build5.c src/game/board.c src/game/gas.c include/board.h include/gas.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build5.c src/game/board.c src/game/gas.c -o $(TEST5_BINARY)

$(TEST6_BINARY): tests/test_build6.c src/game/board.c src/game/gas.c include/board.h include/gas.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build6.c src/game/board.c src/game/gas.c -o $(TEST6_BINARY)

$(TEST8_BINARY): tests/test_build8.c src/game/audio.c src/game/audio_samples.c src/platform/host/audio_host.c include/audio.h include/audio_platform.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build8.c src/game/audio.c src/game/audio_samples.c src/platform/host/audio_host.c -o $(TEST8_BINARY)

$(TEST9_BINARY): tests/test_build9.c src/game/board.c src/game/gas.c src/game/persistence.c include/persistence.h include/ui_theme.h include/match_mode.h include/audio.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build9.c src/game/board.c src/game/gas.c src/game/persistence.c -o $(TEST9_BINARY)

$(TEST10_BINARY): tests/test_build10.c $(CPU_SOURCES) include/cpu.h include/cpu_internal.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build10.c $(CPU_SOURCES) -o $(TEST10_BINARY)

$(TEST10_ALPHA_BINARY): tests/test_build10_alpha_beta.c $(CPU_SOURCES) include/cpu.h include/cpu_internal.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build10_alpha_beta.c $(CPU_SOURCES) -o $(TEST10_ALPHA_BINARY)

$(TEST11_BINARY): tests/test_build11.c src/game/board.c src/game/gas.c src/game/cpu_format.c src/game/ux.c include/ux.h include/match_mode.h include/cpu.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build11.c src/game/board.c src/game/gas.c src/game/cpu_format.c src/game/ux.c -o $(TEST11_BINARY)

$(TEST15_BINARY): tests/test_build15.c src/game/replay.c src/game/board.c src/game/gas.c include/replay.h include/match_mode.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build15.c src/game/replay.c src/game/board.c src/game/gas.c -o $(TEST15_BINARY)

$(TEST15_FILE_BINARY): tests/test_build15_replay_file.c src/game/replay_file.c src/game/replay.c src/game/board.c src/game/gas.c include/replay_file.h include/replay.h include/match_mode.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build15_replay_file.c src/game/replay_file.c src/game/replay.c src/game/board.c src/game/gas.c -o $(TEST15_FILE_BINARY)

$(TEST16_BINARY): tests/test_build16.c src/game/practice_undo.c src/game/board.c src/game/gas.c include/practice_undo.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build16.c src/game/practice_undo.c src/game/board.c src/game/gas.c -o $(TEST16_BINARY)

$(TEST17_BINARY): tests/test_build17.c src/game/ui_theme.c include/ui_theme.h include/cf_types.h include/vga.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build17.c src/game/ui_theme.c -o $(TEST17_BINARY)

$(PROFILE_BINARY): tests/profile_runtime.c $(CPU_SOURCES) include/version.h include/replay.h include/cpu.h include/cpu_internal.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/profile_runtime.c $(CPU_SOURCES) -o $(PROFILE_BINARY)

host-run: host
	./$(HOST_BINARY)

profile: $(PROFILE_BINARY) $(HOST_BINARY)
	./$(PROFILE_BINARY) > $(PROFILE_REPORT)
	size $(HOST_BINARY) > $(SIZE_REPORT)
	test -s $(PROFILE_REPORT)
	test -s $(SIZE_REPORT)
	cat $(PROFILE_REPORT)
	cat $(SIZE_REPORT)

release-audit:
	sh scripts/release_audit.sh

test: $(TEST4_BINARY) $(TEST5_BINARY) $(TEST6_BINARY) $(TEST8_BINARY) \
      $(TEST9_BINARY) $(TEST10_BINARY) $(TEST10_ALPHA_BINARY) \
      $(TEST11_BINARY) $(TEST15_BINARY) $(TEST15_FILE_BINARY) \
      $(TEST16_BINARY) $(TEST17_BINARY) $(PROFILE_BINARY) $(HOST_BINARY)
	./$(TEST4_BINARY)
	./$(TEST5_BINARY)
	./$(TEST6_BINARY)
	./$(TEST8_BINARY)
	./$(TEST9_BINARY)
	./$(TEST10_BINARY)
	./$(TEST10_ALPHA_BINARY)
	./$(TEST11_BINARY)
	./$(TEST15_BINARY)
	./$(TEST15_FILE_BINARY)
	./$(TEST16_BINARY)
	./$(TEST17_BINARY)
	./$(HOST_BINARY)
	test -s $(HOST_PREVIEW)
	test -s $(HOST_TITLE_PREVIEW)
	test -s $(HOST_ATTRACT_PREVIEW)
	test -s $(HOST_HELP_PREVIEW)
	test -s $(HOST_HISTORY_PREVIEW)
	test -s $(HOST_CREDITS_PREVIEW)
	test -s $(HOST_TERMINAL_PREVIEW)
	test -s $(HOST_AUDIO_LOG)
	test -s $(HOST_FART_WAV)
	test -s $(HOST_SAVE)
	test -s $(HOST_CONFIG)
	./$(PROFILE_BINARY) > $(PROFILE_REPORT)
	size $(HOST_BINARY) > $(SIZE_REPORT)
	test -s $(PROFILE_REPORT)
	test -s $(SIZE_REPORT)
	cat $(PROFILE_REPORT)
	cat $(SIZE_REPORT)
	sh scripts/release_audit.sh
	@echo "Full host regression gate passed."

dos:
	@echo "Set WATCOM/PATH/INCLUDE, then run: sh scripts/build_dos_ci.sh"

release:
	sh scripts/package_release.sh

clean:
	rm -rf build/host build/release
