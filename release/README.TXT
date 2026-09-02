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
KNOWNISS.TXT   Platform notes and unverified hardware targets
BUILDINFO.TXT  Exact source/toolchain build information

SAVE FILES
----------
The game writes CHESSFRT.SAV and CHESSFRT.CFG in the current directory.
The versioned save/config formats are unchanged from the release candidate.

This is Chess Fart 1.0.0. The automated release gate covers host rules and
CPU regressions, a real Open Watcom 16-bit DOS build, DOSBox smoke, browser
runtime/playtest coverage, and deterministic full Chromium games. Physical
DOS hardware and specific third-party drivers remain explicitly unverified
where listed in KNOWNISS.TXT.
