CC ?= cc
CFLAGS ?= -std=c89 -pedantic -Wall -Wextra -Werror -O2 -Iinclude

HOST_SOURCES = \
	src/main_build7.c \
	src/game/board.c \
	src/game/gas.c \
	src/game/font.c \
	src/game/board_view_build5.c \
	src/game/board_view_build6.c \
	src/game/board_view_build7.c \
	src/game/presentation.c \
	src/platform/host/vga_build7_host.c \
	src/platform/host/input_build7_host.c

HOST_BINARY = build/host/chessfart_host
HOST_PREVIEW = build/host/chessfart_build7.ppm
TEST4_BINARY = build/host/test_build4
TEST5_BINARY = build/host/test_build5
TEST6_BINARY = build/host/test_build6

.PHONY: all host host-run test test-build7 dos clean

all: host

host: $(HOST_BINARY)

$(HOST_BINARY): $(HOST_SOURCES) include/cf_types.h include/vga.h include/input_build5.h include/font.h include/board.h include/gas.h include/board_view_build5.h include/board_view_build6.h include/board_view_build7.h include/presentation.h
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

host-run: host
	./$(HOST_BINARY)

test: test-build7

test-build7: $(TEST4_BINARY) $(TEST5_BINARY) $(TEST6_BINARY) host-run
	./$(TEST4_BINARY)
	./$(TEST5_BINARY)
	./$(TEST6_BINARY)
	test -s $(HOST_PREVIEW)
	@echo "Build 7 presentation smoke test passed."

dos:
	@echo "Use Open Watcom: wmake -f makefile.build7.dos dos"

clean:
	rm -rf build/host
