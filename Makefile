CC ?= cc
CFLAGS ?= -std=c89 -pedantic -Wall -Wextra -Werror -O2 -Iinclude

HOST_SOURCES = \
	src/main_build5.c \
	src/game/board.c \
	src/game/gas.c \
	src/game/font.c \
	src/game/board_view_build5.c \
	src/platform/host/vga_build5_host.c \
	src/platform/host/input_build5_host.c

HOST_BINARY = build/host/chessfart_host
HOST_PREVIEW = build/host/chessfart_build5.ppm
TEST4_BINARY = build/host/test_build4
TEST5_BINARY = build/host/test_build5

.PHONY: all host host-run test test-build5 dos clean

all: host

host: $(HOST_BINARY)

$(HOST_BINARY): $(HOST_SOURCES) include/cf_types.h include/vga.h include/input_build5.h include/font.h include/board.h include/gas.h include/board_view_build5.h
	mkdir -p build/host
	$(CC) $(CFLAGS) $(HOST_SOURCES) -o $(HOST_BINARY)

$(TEST4_BINARY): tests/test_build4.c src/game/board.c include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build4.c src/game/board.c -o $(TEST4_BINARY)

$(TEST5_BINARY): tests/test_build5.c src/game/board.c src/game/gas.c include/board.h include/gas.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build5.c src/game/board.c src/game/gas.c -o $(TEST5_BINARY)

host-run: host
	./$(HOST_BINARY)

test: test-build5

test-build5: $(TEST4_BINARY) $(TEST5_BINARY) host-run
	./$(TEST4_BINARY)
	./$(TEST5_BINARY)
	test -s $(HOST_PREVIEW)
	@echo "Build 5 host smoke test passed."

dos:
	@echo "Use Open Watcom: wmake -f makefile.build5.dos dos"

clean:
	rm -rf build/host
