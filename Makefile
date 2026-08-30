CC ?= cc
CFLAGS ?= -std=c89 -pedantic -Wall -Wextra -Werror -O2 -Iinclude

HOST_SOURCES = \
	src/main.c \
	src/game/board.c \
	src/game/font.c \
	src/game/board_view.c \
	src/platform/host/vga_host.c \
	src/platform/host/input_host.c

HOST_BINARY = build/host/chessfart_host
HOST_PREVIEW = build/host/chessfart_build4.ppm
TEST_BINARY = build/host/test_build4

.PHONY: all host host-run test test-build4 dos clean

all: host

host: $(HOST_BINARY)

$(HOST_BINARY): $(HOST_SOURCES) include/cf_types.h include/vga.h include/input.h include/font.h include/board.h include/board_view.h
	mkdir -p build/host
	$(CC) $(CFLAGS) $(HOST_SOURCES) -o $(HOST_BINARY)

$(TEST_BINARY): tests/test_build4.c src/game/board.c include/board.h include/cf_types.h
	mkdir -p build/host
	$(CC) $(CFLAGS) tests/test_build4.c src/game/board.c -o $(TEST_BINARY)

host-run: host
	./$(HOST_BINARY)

test: test-build4

test-build4: $(TEST_BINARY) host-run
	./$(TEST_BINARY)
	test -s $(HOST_PREVIEW)
	@echo "Build 4 host smoke test passed."

dos:
	@echo "Use Open Watcom: wmake -f makefile.dos dos"

clean:
	rm -rf build/host
