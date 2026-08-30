CC ?= cc
CFLAGS ?= -std=c89 -pedantic -Wall -Wextra -Werror -O2 -Iinclude

HOST_SOURCES = \
	src/main.c \
	src/game/font.c \
	src/game/board_view.c \
	src/platform/host/vga_host.c \
	src/platform/host/input_host.c

HOST_BINARY = build/host/chessfart_host
HOST_PREVIEW = build/host/chessfart_build1.ppm

.PHONY: all host host-run test-build1 dos clean

all: host

host: $(HOST_BINARY)

$(HOST_BINARY): $(HOST_SOURCES) include/cf_types.h include/vga.h include/input.h include/font.h include/board_view.h
	mkdir -p build/host
	$(CC) $(CFLAGS) $(HOST_SOURCES) -o $(HOST_BINARY)

host-run: host
	./$(HOST_BINARY)

test-build1: host-run
	test -s $(HOST_PREVIEW)
	@echo "Build 1 host smoke test passed."

dos:
	@echo "Use Open Watcom: wmake -f makefile.dos dos"

clean:
	rm -rf build/host
