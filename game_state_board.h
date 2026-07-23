#pragma once
#include <stdint.h>
#include "game_state_board_table.h"
#include "game_internals.h"

typedef struct {
    GameBoardTable table;
    uint32_t score;
    uint32_t moves;
} GameStateBoard;

void game_state_board_init(GameStateBoard* board);
bool game_state_board_is_over(GameStateBoard* const board);
