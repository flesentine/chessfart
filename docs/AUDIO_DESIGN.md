# Chess Fart — Audio Design

## 1. Goal

Audio should sound like somebody crammed a comedy CD-ROM game into a 1993 DOS machine.

It must be funny without making repeated play unbearable.

## 2. Priority order

1. Reliable mute/no-audio mode
2. Short UI and move sounds
3. Varied fart samples
4. Check/checkmate stingers
5. Optional menu/game music

Do not let music work block gameplay milestones.

## 3. Hardware targets

### Primary

Sound Blaster-compatible digital audio:

- 8-bit unsigned PCM
- mono
- 11025 Hz initially
- small sample memory footprint

### Fallback

PC speaker cues:

- click/select
- move chirp
- crude descending fart buzz
- check alarm
- victory arpeggio

### Optional later

AdLib/OPL2 music if it can be integrated without destabilizing timing.

## 4. Fart sample philosophy

Use a small bank so repeated games do not produce the identical sound every time.

Suggested categories:

- `toot_short`
- `rasp_medium`
- `wet_long`
- `royal_boom`
- `tiny_squeak`

Selection may be cosmetic pseudo-randomness because it does not affect game state. If replays need audiovisual determinism, seed cosmetic RNG from action number.

Keep samples short; most should be under ~500 ms.

## 5. Event table

| Event | Sound |
|---|---|
| Cursor move | very soft tick |
| Select piece | wooden click |
| Invalid action | low bonk |
| Normal move | board tap |
| Capture | heavier clack |
| Enter fart mode | tiny pressure hiss |
| Fart / no target | short toot |
| Fart / successful push | stronger sample + impact pop |
| Check | metallic alert/stinger |
| Checkmate | fanfare + signature fart button |
| Promotion | ascending sparkle/chime |
| Menu confirm | bright click |

## 6. Mixing constraints

Initial engine can play one digital effect at a time. If mixing is added:

- 2–4 channels maximum
- fart SFX may duck music
- UI clicks should never cut off a major checkmate stinger unless necessary

## 7. Volume options

Config menu:

- SFX: OFF / LOW / MED / HIGH
- MUSIC: OFF / LOW / MED / HIGH
- DEVICE: AUTO / SB / PC SPEAKER / NONE

Game must function identically with all sound disabled.

## 8. Music direction

If music is implemented, favor short looping OPL pieces:

- title: mock-royal fanfare
- game: restrained courtly march
- danger/check: optional tempo/voicing change
- victory: pompous ceremonial cadence

No continuous comedic sound bed. The joke lands better against mostly serious music.

## 9. Audio assets and licensing

All final shipped sounds should be original, generated specifically for the project, or have clearly compatible licensing documented in the repository.

Do not scrape random internet samples into the final build.