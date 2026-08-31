CHESS FART 1.0.0-rc1
=====================

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
A Microsoft-compatible mouse driver is optional; keyboard play is complete.

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
KNOWNISS.TXT   RC1 limitations and platform notes
BUILDINFO.TXT  Exact source/toolchain build information

SAVE FILES
----------
The game writes CHESSFRT.SAV and CHESSFRT.CFG in the current directory.

This is release candidate 1. Please retain a copy of important save files
before testing future release candidates.
