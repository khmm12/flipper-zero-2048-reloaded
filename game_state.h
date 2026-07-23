#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "game_internals.h"
#include "game_state_board.h"
#include "game_state_board_history.h"

typedef enum {
    GameMoveLeft,
    GameMoveRight,
    GameMoveUp,
    GameMoveDown,
    GameMoveUndo,
    GameReset,
} GameEvent;

typedef struct {
    GameStateBoard board;
    GameStateBoardHistory history;
    uint32_t top_score;
    bool is_over;
    bool is_record_broken;
} GameState;

typedef bool (*GameStateLoadCallback)(GameState* self);
typedef bool (*GameStateDumpCallback)(GameState* self);

void game_state_init(GameState* const game_state);
void game_state_send(GameState* state, GameEvent event);
bool game_state_load(GameState* state, GameStateLoadCallback cb);
bool game_state_dump(GameState* const state, GameStateDumpCallback cb);
bool game_state_is_valid(const GameState* const state);
