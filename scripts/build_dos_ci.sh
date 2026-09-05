#!/usr/bin/env sh
set -eu

: "${WATCOM:?WATCOM must point to the Open Watcom snapshot root}"
export PATH="$WATCOM/binl64:$WATCOM/binw:$PATH"
export INCLUDE="$WATCOM/h"

mkdir -p build/dos
rm -f ./*.o ./*.obj build/dos/CHESSFRT.EXE build/dos/DOSSMOKE.EXE build/dos/DOSSMOKE.OK

wcl -zq -bt=dos -ml -3 -ox -w4 -Iinclude -fe=build/dos/CHESSFRT.EXE \
  src/main_build11.c \
  src/game/board.c src/game/gas.c \
  src/game/cpu_config.c src/game/cpu_actions.c src/game/cpu_eval.c \
  src/game/cpu_search.c src/game/cpu_format.c src/game/ux.c src/game/replay.c \
  src/game/font.c src/game/board_view_build5.c src/game/board_view_build6.c \
  src/game/ui_assets.c src/game/board_view_art.c src/game/presentation.c \
  src/game/audio.c src/game/audio_samples.c src/game/audio_game.c \
  src/game/persistence.c src/game/persistence_ui.c \
  src/platform/dos/vga_dos.c src/platform/dos/input_build5_dos.c \
  src/platform/dos/audio_dos.c src/platform/dos/mouse_dos.c

rm -f ./*.o ./*.obj

wcl -zq -bt=dos -ml -3 -ox -w4 -Iinclude -fe=build/dos/DOSSMOKE.EXE \
  tests/dos_smoke.c src/game/audio.c src/game/audio_samples.c \
  src/platform/dos/audio_dos.c src/platform/dos/vga_dos.c \
  src/platform/dos/mouse_dos.c

rm -f ./*.o ./*.obj

test -s build/dos/CHESSFRT.EXE
test -s build/dos/DOSSMOKE.EXE
printf 'Built %s and DOSSMOKE.EXE with Open Watcom.\n' "$(cat VERSION)"
