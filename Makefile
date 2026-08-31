CC ?= cc
CFLAGS ?= -std=c89 -pedantic -Wall -Wextra -Werror -O2 -Iinclude

HOST_SOURCES = \
	src/main_build11.c \
	src/game/board.c \
	src/game/gas.c \
	src/game/cpu_config.c \
	src/game/cpu_actions.c \
	src/game/cpu_eval.c \
	src/game/cpu_search.c \
	src/game/cpu_format.c \
	src/game/ux.c \
	src/game/font.c \
	src/game/board_view_build5.c \
	src/game/board_view_build6.c \
	src/game/board_view_build7.c \
	src/game/presentation.c \
	src/game/audio.c \
	src/game/audio_samples.c \
	src/game/audio_game.c \
	src/game/persistence.c \
	src/game/persistence_ui.c \
	src/platform/host/vga_build11_host.c \
	src/platform/host/input_build11_host.c \
	src/platform/host/audio_build11_host.c \
	src/platform/host/mouse_host.c

HOST_BINARY = build/host/chessfart_host
HOST_PREVIEW = build/host/chessfart_build11.ppm
HOST_TITLE_PREVIEW = build/host/chessfart_build11_title.ppm
HOST_ATTRACT_PREVIEW = build/host/chessfart_build11_attract.ppm
HOST_HELP_PREVIEW = build/host/chessfart_build11_help.ppm
HOST_HISTORY_PREVIEW = build/host/chessfart_build11_history.ppm
HOST_CREDITS_PREVIEW = build/host/chessfart_build11_credits.ppm
HOST_TERMINAL_PREVIEW = build/host/chessfart_build11_terminal.ppm
HOST_AUDIO_LOG = build/host/chessfart_build11_audio.log
HOST_FART_WAV = build/host/chessfart_build11_fart.wav
HOST_SAVE = build/host/CHESSFRT.SAV
HOST_CONFIG = build/host/CHESSFRT.CFG
TEST4_BINARY = build/host/test_build4
TEST5_BINARY = build/host/test_build5
TEST6_BINARY = build/host/test_build6
TEST8_BINARY = build/host/test_build8
TEST9_BINARY = build/host/test_build9
TEST10_BINARY = build/host/test_build10
TEST11_BINARY = build/host/test_build11

.PHONY: all host host-run test test-build11 dos clean

all: host

host: $(HOST_BINARY)

$(HOST_BINARY): $(HOST_SOURCES) include/cf_types.h include/vga.h include/input_build5.h include/font.h include/board.h include/gas.h include/cpu.h include/ux.h include/mouse.h include/board_view_build5.h include/board_view_build6.h include/board_view_build7.h include/presentation.h include/audio.h include/audio_platform.h include/audio_game.h include/persistence.h include/persistence_ui.h src/main_build9.c src/main_build11_hooks.inc src/main_build11_ui.inc
	mkdir -p build/host
	$(CC) $(CFLAGS) -DCF_BUILD6_DEMO -DCF_HOST_BUILD $(HOST_SOURCES) -o $(HOST_BINARY)

$(TEST4_BINARY): tests/test_build4.c src/game/board.c include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build4.c src/game/board.c -o $(TEST4_BINARY)

$(TEST5_BINARY): tests/test_build5.c src/game/board.c src/game/gas.c include/board.h include/gas.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build5.c src/game/board.c src/game/gas.c -o $(TEST5_BINARY)

$(TEST6_BINARY): tests/test_build6.c src/game/board.c src/game/gas.c include/board.h include/gas.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build6.c src/game/board.c src/game/gas.c -o $(TEST6_BINARY)

$(TEST8_BINARY): tests/test_build8.c src/game/audio.c src/game/audio_samples.c src/platform/host/audio_build11_host.c include/audio.h include/audio_platform.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build8.c src/game/audio.c src/game/audio_samples.c src/platform/host/audio_build11_host.c -o $(TEST8_BINARY)

$(TEST9_BINARY): tests/test_build9.c src/game/board.c src/game/gas.c src/game/persistence.c include/persistence.h include/audio.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build9.c src/game/board.c src/game/gas.c src/game/persistence.c -o $(TEST9_BINARY)

$(TEST10_BINARY): tests/test_build10.c src/game/board.c src/game/gas.c src/game/cpu_config.c src/game/cpu_actions.c src/game/cpu_eval.c src/game/cpu_search.c src/game/cpu_format.c include/cpu.h include/cpu_internal.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build10.c src/game/board.c src/game/gas.c src/game/cpu_config.c src/game/cpu_actions.c src/game/cpu_eval.c src/game/cpu_search.c src/game/cpu_format.c -o $(TEST10_BINARY)

$(TEST11_BINARY): tests/test_build11.c src/game/board.c src/game/gas.c src/game/cpu_format.c src/game/ux.c include/ux.h include/cpu.h include/gas.h include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build11.c src/game/board.c src/game/gas.c src/game/cpu_format.c src/game/ux.c -o $(TEST11_BINARY)

host-run: host
	./$(HOST_BINARY)

test: test-build11

test-build11: $(TEST4_BINARY) $(TEST5_BINARY) $(TEST6_BINARY) $(TEST8_BINARY) $(TEST9_BINARY) $(TEST10_BINARY) $(TEST11_BINARY) $(HOST_BINARY)
	./$(TEST4_BINARY)
	./$(TEST5_BINARY)
	./$(TEST6_BINARY)
	./$(TEST8_BINARY)
	./$(TEST9_BINARY)
	./$(TEST10_BINARY)
	./$(TEST11_BINARY)
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
	@echo "Build 11 UX/polish smoke test passed."

dos:
	@echo "Use Open Watcom: wmake -f makefile.build11.dos dos"

clean:
	rm -rf build/host
