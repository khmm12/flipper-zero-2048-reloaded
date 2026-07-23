# 2048 Reloaded

A modernized version of the classic **2048** game for the
[Flipper Zero](https://flipperzero.one/).

[![CI](https://github.com/khmm12/flipper-zero-2048-game/actions/workflows/ci.yml/badge.svg)](https://github.com/khmm12/flipper-zero-2048-game/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](./LICENSE)
[![Flipper category: Games](https://img.shields.io/badge/Flipper-Games-orange.svg)](https://flipperzero.one/)

## Features

- Play up to the 65536 tile.
- Progress is saved automatically — quit anytime and pick up where you left off.
- Undo — bad luck? Take up to 10 moves back at any point, even after game over.
- Top score tracking.

## Controls

| Key           | Action                          |
| ------------- | ------------------------------- |
| D-pad         | Move tiles                      |
| OK            | Open the menu (resume/new game) |
| Back (short)  | Undo the last move              |
| Back (long)   | Save and exit                   |

## Install

Build from source (below) — the `.fap` lands in `dist/` — then copy it to your
Flipper's SD card under `apps/Games/` (with [qFlipper](https://flipperzero.one/downloads)
or `ufbt launch` over USB).

## Build from source

The project uses [ufbt](https://pypi.org/project/ufbt/), the micro Flipper Build Tool.

```sh
ufbt        # build; the .fap ends up in dist/
ufbt launch # build, deploy over USB, and run on the Flipper
```

With [mise](https://mise.jdx.dev/) installed, `mise install` provisions ufbt for you.

## Development

The game logic (board moves, merge rules, history, save-state validation) is pure C
with no Flipper SDK dependencies, so it builds and runs on any host machine:

```sh
make -C test test  # build with strict warnings and run the unit tests
make -C test lint  # compile-only pass with the strict warning set
ufbt format        # apply the project clang-format style
```

CI runs the same checks plus a firmware build against both the `release` and `dev`
SDK channels.

## Acknowledgements

This project is based on the great
[work](https://github.com/eugene-kirzhanov/flipper-zero-2048-game) by Eugene Kirzhanov.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for
details.
