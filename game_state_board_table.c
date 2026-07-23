#include "game_state_board_table.h"

#include <stdlib.h>
#include <string.h>

typedef uint8_t GameBoardTableLine[CELLS_COUNT];

static void
    game_board_table_get_column(GameBoardTable table, uint8_t column_index, GameBoardTableLine out);
static void
    game_board_table_set_column(GameBoardTable table, uint8_t column_index, GameBoardTableLine src);
static void
    game_board_table_get_row(GameBoardTable table, uint8_t row_index, GameBoardTableLine out);
static void
    game_board_table_set_row(GameBoardTable table, uint8_t row_index, GameBoardTableLine src);
static void game_board_table_line_move(GameBoardTableLine line, MoveResult* const move_result);
static void game_board_table_line_reverse(GameBoardTableLine line);
static void
    game_board_table_line_shift(GameBoardTableLine line, uint8_t from_index, uint8_t offset);
static uint8_t game_board_table_line_find_non_empty_cell_index(GameBoardTableLine line, uint8_t i);
static void move_result_init(MoveResult* move_result, const GameBoardTable table);

void game_board_table_init(GameBoardTable table) {
    memset(table, 0, sizeof(GameBoardTable));
    game_board_table_push_random_digit(table);
    game_board_table_push_random_digit(table);
}

void game_board_table_copy(GameBoardTable const src, GameBoardTable dest) {
    memcpy(dest, src, sizeof(GameBoardTable));
}

bool game_board_table_can_move(GameBoardTable table) {
    MoveResult move_result;

    game_board_table_move_left(table, &move_result);
    if(move_result.is_table_updated) return true;

    game_board_table_move_right(table, &move_result);
    if(move_result.is_table_updated) return true;

    game_board_table_move_up(table, &move_result);
    if(move_result.is_table_updated) return true;

    game_board_table_move_down(table, &move_result);
    if(move_result.is_table_updated) return true;

    return false;
}

bool game_board_table_has_empty_cells(GameBoardTable table) {
    for(uint8_t i = 0; i < CELLS_COUNT; i++) {
        for(uint8_t j = 0; j < CELLS_COUNT; j++) {
            if(table[i][j] == 0) {
                return true;
            }
        }
    }

    return false;
}

void game_board_table_move_left(GameBoardTable const table, MoveResult* const move_result) {
    move_result_init(move_result, table);

    for(uint8_t row_index = 0; row_index < CELLS_COUNT; row_index++) {
        GameBoardTableLine row;

        game_board_table_get_row(move_result->new_table, row_index, row);
        game_board_table_line_move(row, move_result);
        game_board_table_set_row(move_result->new_table, row_index, row);
    }
}

void game_board_table_move_right(GameBoardTable const table, MoveResult* const move_result) {
    move_result_init(move_result, table);

    for(uint8_t row_index = 0; row_index < CELLS_COUNT; row_index++) {
        GameBoardTableLine row;

        game_board_table_get_row(move_result->new_table, row_index, row);
        game_board_table_line_reverse(row);
        game_board_table_line_move(row, move_result);
        game_board_table_line_reverse(row);
        game_board_table_set_row(move_result->new_table, row_index, row);
    }
}

void game_board_table_move_up(GameBoardTable const table, MoveResult* const move_result) {
    move_result_init(move_result, table);

    for(uint8_t column_index = 0; column_index < CELLS_COUNT; column_index++) {
        GameBoardTableLine column;

        game_board_table_get_column(move_result->new_table, column_index, column);
        game_board_table_line_move(column, move_result);
        game_board_table_set_column(move_result->new_table, column_index, column);
    }
}

void game_board_table_move_down(GameBoardTable const table, MoveResult* const move_result) {
    move_result_init(move_result, table);

    for(uint8_t column_index = 0; column_index < CELLS_COUNT; column_index++) {
        GameBoardTableLine column;

        game_board_table_get_column(move_result->new_table, column_index, column);
        game_board_table_line_reverse(column);
        game_board_table_line_move(column, move_result);
        game_board_table_line_reverse(column);
        game_board_table_set_column(move_result->new_table, column_index, column);
    }
}

void game_board_table_push_random_digit(GameBoardTable table) {
    uint8_t empty_cell_indexes[CELLS_COUNT * CELLS_COUNT];
    uint8_t empty_cells_count = 0;

    for(uint8_t i = 0; i < CELLS_COUNT; i++) {
        for(uint8_t j = 0; j < CELLS_COUNT; j++) {
            if(table[i][j] == 0) {
                empty_cell_indexes[empty_cells_count++] = i * CELLS_COUNT + j;
            }
        }
    }

    if(empty_cells_count == 0) return;

    uint8_t cell_index = empty_cell_indexes[random() % empty_cells_count];
    table[cell_index / CELLS_COUNT][cell_index % CELLS_COUNT] =
        random() % 100 < 90 ? 1 : 2; // 90% for 2, 10% for 4
}

void game_board_table_line_move(GameBoardTableLine line, MoveResult* const move_result) {
    for(uint8_t i = 0; i < CELLS_COUNT - 1; i++) {
        // The cell is empty, shift all not empty cells to its position.
        // X 2 X 2 -> 2 X 2 X
        // X X X 2 -> 2 X X X
        // X X X X -> break, nothing to do
        if(line[i] == 0) {
            uint8_t next_non_empty_index =
                game_board_table_line_find_non_empty_cell_index(line, i);
            if(next_non_empty_index >= CELLS_COUNT) break;

            uint8_t offset = next_non_empty_index - i;
            game_board_table_line_shift(line, i, offset);
            move_result->is_table_updated = true;
        }

        // The next cell is empty, shift all not empty cells to its position.
        // 2 X 2 X -> 2 2 X X
        // 2 X X X -> break, nothing to do
        if(line[i + 1] == 0) {
            uint8_t next_non_empty_index =
                game_board_table_line_find_non_empty_cell_index(line, i + 1);
            if(next_non_empty_index >= CELLS_COUNT) break;

            uint8_t offset = next_non_empty_index - (i + 1);
            game_board_table_line_shift(line, i + 1, offset);
            move_result->is_table_updated = true;
        }

        // Both cells have the same value, merge them, then shift all remaining cells.
        // 2 2 X X -> 4 X X X
        // 2 2 4 8 -> 4 4 8 X
        if(line[i] == line[i + 1]) {
            uint8_t val = line[i];

            line[i]++;
            game_board_table_line_shift(line, i + 1, 1);

            move_result->is_table_updated = true;
            move_result->score_points += 2u << val;
        }
    }
}

void game_board_table_get_column(
    GameBoardTable table,
    uint8_t column_index,
    GameBoardTableLine out) {
    for(uint8_t i = 0; i < CELLS_COUNT; i++) {
        out[i] = table[i][column_index];
    }
}

void game_board_table_set_column(
    GameBoardTable table,
    uint8_t column_index,
    GameBoardTableLine src) {
    for(uint8_t i = 0; i < CELLS_COUNT; i++) {
        table[i][column_index] = src[i];
    }
}

void game_board_table_get_row(GameBoardTable table, uint8_t row_index, GameBoardTableLine out) {
    memcpy(out, table[row_index], sizeof(GameBoardTableLine));
}

void game_board_table_set_row(GameBoardTable table, uint8_t row_index, GameBoardTableLine src) {
    memcpy(table[row_index], src, sizeof(GameBoardTableLine));
}

void game_board_table_line_reverse(GameBoardTableLine line) {
    for(uint8_t i = 0; i < CELLS_COUNT / 2; i++) {
        uint8_t tmp = line[i];
        line[i] = line[CELLS_COUNT - 1 - i];
        line[CELLS_COUNT - 1 - i] = tmp;
    }
}

uint8_t
    game_board_table_line_find_non_empty_cell_index(GameBoardTableLine line, uint8_t start_index) {
    uint8_t offset = 1;
    while(start_index + offset < CELLS_COUNT && line[start_index + offset] == 0)
        offset++;
    return start_index + offset;
}

void game_board_table_line_shift(GameBoardTableLine line, uint8_t from_index, uint8_t offset) {
    for(uint8_t i = from_index; i < CELLS_COUNT; i++) {
        line[i] = i + offset < CELLS_COUNT ? line[i + offset] : 0;
    }
}

void move_result_init(MoveResult* move_result, GameBoardTable const table) {
    move_result->is_table_updated = false;
    move_result->score_points = 0;
    game_board_table_copy(table, move_result->new_table);
}
