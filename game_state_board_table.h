#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "game_internals.h"

typedef uint8_t GameBoardTable[CELLS_COUNT][CELLS_COUNT];

typedef struct {
    GameBoardTable new_table;
    uint32_t score_points;
    bool is_table_updated;
} MoveResult;

void game_board_table_init(GameBoardTable table);
void game_board_table_copy(GameBoardTable src, GameBoardTable dest);
bool game_board_table_has_empty_cells(GameBoardTable table);
bool game_board_table_can_move(GameBoardTable table);
void game_board_table_push_random_digit(GameBoardTable table);
void game_board_table_move_down(GameBoardTable table, MoveResult* const move_result);
void game_board_table_move_left(GameBoardTable table, MoveResult* const move_result);
void game_board_table_move_right(GameBoardTable table, MoveResult* const move_result);
void game_board_table_move_up(GameBoardTable table, MoveResult* const move_result);
