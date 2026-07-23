# AGENTS.md

Guidance for AI coding agents working in this repository. Claude Code reads this file through the `CLAUDE.md` symlink.

2048 game for the Flipper Zero (C, built with ufbt). The FAP id is `game_2048_reloaded`.

## Commands

```sh
mise install        # provision ufbt (one-time)
mise run build      # build the FAP -> dist/
mise run test       # host unit tests: strict warnings + ASan/UBSan
mise run lint       # strict-warning compile of the core + clang-format check
mise run format     # apply clang-format (run before committing C changes)
mise run launch     # build, deploy over USB, run on the Flipper
```

Without mise: `ufbt`, `make -C test test`, `make -C test lint`, `ufbt format`. There is no single-test runner — the whole suite (`test/test_game_2048.c`) runs in well under a second; run it all.

CI (`.github/workflows/ci.yml`) runs the same checks plus FAP builds on the `release` and `dev` SDK channels. CI compiles the tests with **GCC**, local macOS `cc` is clang — GCC is stricter with `-Wconversion`/`-Wpedantic`, so a clean local run does not guarantee a clean CI run.

## Architecture

Two strictly separated layers:

**Pure core** — `game_state_board_table.[ch]` → `game_state_board.[ch]` → `game_state_board_history.[ch]` → `game_state.[ch]`. Standard C only, **no Flipper SDK includes** — this is what lets the tests build on the host. Keep it that way: no `furi.h` here, and no logging (the controller layer logs instead). Cell values are exponents (`1` = tile "2", `MAX_CELL_VALUE` = 16 = tile "65536"), `0` is empty. Moves are expressed as left-moves: right/up/down reuse the same line algorithm via reverse/transpose composition.

**Flipper layer** — `game_controller*.c` (state machine: menu / in progress / game over; split into init+persistence, input handler, drawer) and `game_2048.c` (app entry, event loop). The controller's private API (`game_controller_send`, menu helpers) is gated behind `#define GAME_CONTROLLER_INTERNAL` before including `game_controller.h`.

### Threading model

Three threads touch the app: the main loop (sole **writer** of game state), the GUI thread (draw callback, reads under the mutex), and the timer service thread. The timer callback must never block or touch storage — it only posts a `SaveTick` to the event queue; the main loop does the actual SD write, without holding the mutex (safe because it is the only writer). Keep any new work off the timer thread the same way.

### Persistence

The save file is a `GameSaveHeader` (magic + version) followed by a raw `GameState` dump. **Any change to the `GameState` layout requires bumping `SAVE_VERSION`** in `game_controller.c`. Loaded saves are validated (`game_state_is_valid`) before use — cell values above `MAX_CELL_VALUE` or `history.top` out of `[-1, HISTORY_SIZE)` would cause OOB access downstream. All state mutations go through `game_controller_send`, which marks a dirty flag; the periodic autosave skips the SD write while clean. New mutation paths must go through `game_controller_send`, not `game_state_send` directly.

### Tests

`test/test_game_2048.c` tests the pure core through its public API, with one documented exception (`game_state_post_update`, extern-declared to reach game-over logic without random spawns). `can_move` has a property test against move simulation as the oracle — if you touch move logic, that test is the safety net. The strict warning set in `test/Makefile` doubles as the linter.

## Gotchas

- **fbt's source glob is recursive**: `application.fam` needs `sources=["*.c", "!test"]` or host-only test code gets linked into the firmware. If you add source directories, verify what actually got compiled (object list in `~/.ufbt/build/game_2048_reloaded/`).
- **No `const` on `GameBoardTable` (array typedef) parameters** — before C23, qualifier mismatches on pointers-to-arrays are pedantic errors in both directions, and GCC enforces this in CI.
- `digits.h` is generated, not hand-edited: sprites are packed 1-bit XBM rows (LSB-first, 2 bytes per 14-pixel row). If sprites need changing, regenerate programmatically with a round-trip check against a readable source format.
