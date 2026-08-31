# Chess Fart — Build 12: Release Candidate

## Status

**Complete: Chess Fart 1.0.0-rc1 release-candidate pipeline is green before merge to `main`.**

Build 12 freezes features and turns the Build 11 game into a reproducible DOS release candidate. No chess, Gas/Fart, CPU, save-format, or gameplay semantics change in this milestone.

## What shipped

- version stamp `1.0.0-rc1`
- strict host release gate preserving every Build 4–11 regression suite
- host CPU/memory profiling report
- release-source audit for conflict markers, TODO/FIXME/XXX markers and whitespace errors
- real 16-bit DOS `CHESSFRT.EXE` built with Open Watcom 2.0
- DOSBox execution smoke that touches VGA Mode 13h, mouse INT 33h and audio initialization
- sample `DOSBOX.CONF`
- DOS-readable README, controls and known-issues files
- reproducible packaging script
- ZIP integrity validation and SHA-256 generation
- CI artifact containing the distributable ZIP, raw EXE, DOS smoke proof, diagnostics and visual/audio artifacts

## Verified Build 12 measurements

From the Ubuntu GitHub Actions release-candidate runner:

- DOS `CHESSFRT.EXE`: **111,988 bytes**
- `CfBoard`: 536 bytes
- `CfGasState`: 64 bytes
- `CfGasHistory`: 8,452 bytes
- maximum CPU action list: 18,436 bytes
- EASY starting search: depth 1, 20 nodes
- MED starting search: depth 2, 440 nodes
- HARD starting search: depth 3, 7,320 nodes, 325 alpha-beta cutoffs
- Fart-heavy MED fixture: depth 2, 6,240 nodes

Host wall-clock search measurements are diagnostic only and are not treated as DOS-machine performance promises.

## DOS verification

CI uses the maintained `open-watcom/setup-watcom@v1` action with Open Watcom 2.0 and target `dos`, then invokes `scripts/build_dos_ci.sh` to compile both the full game and a small platform smoke executable.

The smoke executable runs inside DOSBox and writes:

```text
CHESS FART 1.0.0-rc1 DOS SMOKE OK
```

The smoke initializes Mode 13h, draws/presents VGA pixels, probes the DOS mouse interface, initializes AUTO audio and attempts a UI sound before returning to DOS.

For reproducibility documentation, the project also records the Open Watcom `2026-07-13-Build` snapshot asset and SHA-256 `35e9c56ca93234ba0b8b596387815399317ea0548aeac1318f0674dca3921314`. The CI-built executable itself is described accurately as Open Watcom 2.0 via the official setup action rather than falsely claiming that exact snapshot.

## Release package

`chessfart-1.0.0-rc1-dos.zip` contains one `CHESSFRT/` directory:

- `CHESSFRT.EXE`
- `DOSBOX.CONF`
- `README.TXT`
- `KEYS.TXT`
- `KNOWNISS.TXT`
- `BUILDINFO.TXT`

The packager tests the ZIP after creation and writes a sibling `.sha256` file.

## Compatibility position

Automated verification now covers strict C89 host builds, the complete Build 4–11 test stack, Open Watcom DOS compilation and DOSBox platform smoke. Physical 386/486 hardware, MS-DOS/FreeDOS combinations, real Sound Blaster variants and third-party mouse drivers remain manual/community validation targets and are not represented as tested.

See `docs/COMPATIBILITY.md` and `docs/RELEASE_CHECKLIST.md`.

## Build commands

Host RC gate:

```sh
make test-build12
```

DOS build with an Open Watcom environment:

```sh
sh scripts/build_dos_ci.sh
```

Package after DOS build:

```sh
sh scripts/package_release.sh
```

Classic DOS/Windows Open Watcom users may also use:

```bat
wmake -f makefile.build12.dos dos
```

## Exit criterion

Build 12 is complete when one source revision passes the full host regression/profile/audit gate, compiles a real 16-bit DOS executable, executes the DOS platform smoke under DOSBox, validates the distributable ZIP, generates its checksum, and preserves all outputs as CI artifacts. RC1 satisfies that criterion.
