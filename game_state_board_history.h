#pragma once
#include <stdint.h>
#include "game_state_board.h"
#include "game_internals.h"

typedef struct {
    GameStateBoard items[HISTORY_SIZE];
    int8_t top;
} GameStateBoardHistory;

void game_state_board_history_init(GameStateBoardHistory* history);
void game_state_board_history_pop(GameStateBoardHistory* history, GameStateBoard* board);
void game_state_board_history_push(GameStateBoardHistory* history, GameStateBoard const* board);
