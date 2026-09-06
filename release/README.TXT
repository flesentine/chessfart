CHESS FART 1.0.0
=================

Check. Mate. Ventilate.

Chess Fart is a 320x200, 256-color DOS chess variant. Pieces earn Gas by
moving and capturing, then spend Gas on directional Fart Actions that can
push adjacent pieces while preserving normal king-safety rules.

QUICK START
-----------
DOSBox:
  1. Extract this ZIP.
  2. Start DOSBox in the extracted CHESSFRT directory.
  3. Run CHESSFRT.EXE.

A sample DOSBOX.CONF is included. On real DOS, run CHESSFRT.EXE directly.
Choose PLAY CPU for the deterministic computer opponent, 2 PLAYERS for a
local hot-seat White/Black match, or PRACTICE for local play with U undo.
Practice retains the latest 32 committed actions for exact Undo, including
moves, Farts, promotion, castling and en passant. Practice is session-only
and disables game Save/Load. A Microsoft-compatible mouse driver is
optional; keyboard play is complete. During a match, press R to open the
read-only session replay and step retained frames with Left/Right. With a
mouse driver present, the replay footer provides Previous, Next, Export,
Import and Close. Inside Replay, S exports the live session to CHESSFRT.RPL
and L imports CHESSFRT.RPL temporarily for viewing; closing Replay restores
the untouched live-session replay timeline.

SOUND
-----
Sound Blaster AUTO detection uses the BLASTER environment variable when
available. A typical DOSBox value is:

  SET BLASTER=A220 I7 D1 H5 T6

If Sound Blaster detection fails, AUTO falls back to PC speaker.

FILES
-----
CHESSFRT.EXE   Game executable
DOSBOX.CONF    Sample emulator configuration
README.TXT     This file
KEYS.TXT       Complete controls
KNOWNISS.TXT   Platform notes and unverified hardware targets
BUILDINFO.TXT  Exact source/toolchain build information

SAVE / REPLAY FILES
-------------------
The game writes CHESSFRT.SAV and CHESSFRT.CFG in the current directory.
New game saves use format version 2 so CPU/local match mode is preserved.
Legacy version-1 game saves remain loadable and restore CPU mode. The config
format remains version 1.

CHESSFRT.RPL is created only when the player exports from Replay. It is a
separate version-1 replay file and is not included as a pre-made file in the
release ZIP. Importing it for review does not replace the live game replay.

This is Chess Fart 1.0.0. The automated release gate covers host rules and
CPU regressions, a real Open Watcom 16-bit DOS build, DOSBox smoke, browser
runtime/playtest coverage, deterministic full Chromium CPU games, a
33-ply local two-player Chromium game with exact version-2 LOCAL save/load
rollback, replay-viewer state invariance, transactional CHESSFRT.RPL
export/import, 600-frame ring-file coverage, terminal replay integrity, real-input Practice threefold-draw terminal/Undo coverage, and a separate
300-action review commit-path stress that rolls over the undo, action-log,
repetition-history and replay rings before exact 32-action rollback.
Physical DOS hardware and specific third-party drivers remain explicitly
unverified where listed in KNOWNISS.TXT.

THEMES
Press T on the title screen to switch Royal Basement / Crimson Cellar.
Theme choice is session-only and does not change save/replay formats.
