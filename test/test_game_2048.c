// Host-side unit tests for the pure game core: board moves, history, state.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game_state.h"
#include "game_state_board_table.h"

// Internal game_state helper, exercised directly to test game-over handling
// without depending on random tile spawns.
void game_state_post_update(GameState* const state);

static int failures = 0;

#define CHECK(cond)                                                \
    do {                                                           \
        if(!(cond)) {                                              \
            failures++;                                            \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                          \
    } while(0)

static void table_set(GameBoardTable table, const uint8_t values[CELLS_COUNT][CELLS_COUNT]) {
    memcpy(table, values, sizeof(GameBoardTable));
}

static bool table_equals(GameBoardTable table, const uint8_t values[CELLS_COUNT][CELLS_COUNT]) {
    return memcmp(table, values, sizeof(GameBoardTable)) == 0;
}

static uint8_t table_count_non_empty(GameBoardTable table) {
    uint8_t count = 0;
    for(uint8_t i = 0; i < CELLS_COUNT; i++) {
        for(uint8_t j = 0; j < CELLS_COUNT; j++) {
            if(table[i][j] != 0) count++;
        }
    }
    return count;
}

// A full board with no adjacent equal cells: no move is possible.
static const uint8_t dead_board[CELLS_COUNT][CELLS_COUNT] = {
    {1, 2, 1, 2},
    {2, 1, 2, 1},
    {1, 2, 1, 2},
    {2, 1, 2, 1},
};

static void test_move_left_shift(void) {
    GameBoardTable table = {0};
    MoveResult r;

    table[0][3] = 1;
    game_board_table_move_left(table, &r);
    CHECK(r.is_table_updated);
    CHECK(r.score_points == 0);
    CHECK(r.new_table[0][0] == 1 && r.new_table[0][3] == 0);
}

static void test_move_left_merge_with_gap(void) {
    GameBoardTable table = {0};
    MoveResult r;

    // 2 _ 2 _ -> 4 _ _ _, scores 4
    table[0][0] = 1;
    table[0][2] = 1;
    game_board_table_move_left(table, &r);
    CHECK(r.is_table_updated);
    CHECK(r.score_points == 4);
    CHECK(r.new_table[0][0] == 2 && r.new_table[0][1] == 0 && r.new_table[0][2] == 0);
}

static void test_move_left_no_double_merge(void) {
    GameBoardTable table = {0};
    MoveResult r;

    // 2 2 2 _ -> 4 2 _ _ (the leftmost pair merges, the result does not re-merge)
    table[0][0] = 1;
    table[0][1] = 1;
    table[0][2] = 1;
    game_board_table_move_left(table, &r);
    CHECK(r.score_points == 4);
    CHECK(r.new_table[0][0] == 2 && r.new_table[0][1] == 1 && r.new_table[0][2] == 0);
}

static void test_move_left_two_pairs(void) {
    GameBoardTable table = {0};
    MoveResult r;

    // 2 2 4 4 -> 4 8 _ _, scores 4 + 8
    table[0][0] = 1;
    table[0][1] = 1;
    table[0][2] = 2;
    table[0][3] = 2;
    game_board_table_move_left(table, &r);
    CHECK(r.score_points == 12);
    CHECK(r.new_table[0][0] == 2 && r.new_table[0][1] == 3 && r.new_table[0][2] == 0);
}

static void test_move_noop_is_not_an_update(void) {
    GameBoardTable table = {0};
    MoveResult r;

    // 2 4 8 16 packed to the left: moving left changes nothing.
    table[0][0] = 1;
    table[0][1] = 2;
    table[0][2] = 3;
    table[0][3] = 4;
    game_board_table_move_left(table, &r);
    CHECK(!r.is_table_updated);
    CHECK(r.score_points == 0);
    CHECK(table_equals(r.new_table, (const uint8_t(*)[CELLS_COUNT])table));
}

static void test_move_directions(void) {
    GameBoardTable table = {0};
    MoveResult r;

    table[0][0] = 1;
    table[0][1] = 1;
    game_board_table_move_right(table, &r);
    CHECK(r.new_table[0][3] == 2 && r.new_table[0][0] == 0);

    memset(table, 0, sizeof(table));
    table[0][0] = 1;
    table[1][0] = 1;
    game_board_table_move_down(table, &r);
    CHECK(r.new_table[3][0] == 2 && r.new_table[0][0] == 0);

    memset(table, 0, sizeof(table));
    table[2][1] = 1;
    table[3][1] = 1;
    game_board_table_move_up(table, &r);
    CHECK(r.new_table[0][1] == 2 && r.new_table[3][1] == 0);
}

static void test_can_move_and_game_over(void) {
    GameBoardTable table;

    table_set(table, dead_board);
    CHECK(!game_board_table_can_move(table));
    CHECK(!game_board_table_has_empty_cells(table));

    // A single adjacent pair makes the board playable again.
    table[0][1] = 1;
    CHECK(game_board_table_can_move(table));

    GameStateBoard board;
    table_set(board.table, dead_board);
    CHECK(game_state_board_is_over(&board));
}

static void test_push_random_digit(void) {
    GameBoardTable table = {0};

    game_board_table_push_random_digit(table);
    CHECK(table_count_non_empty(table) == 1);
    game_board_table_push_random_digit(table);
    CHECK(table_count_non_empty(table) == 2);

    // A full board must stay untouched.
    table_set(table, dead_board);
    game_board_table_push_random_digit(table);
    CHECK(table_equals(table, dead_board));
}

static void test_history_lifo_and_overflow(void) {
    GameStateBoardHistory history;
    GameStateBoard board = {0};

    game_state_board_history_init(&history);

    // Pop from an empty history is a no-op.
    board.score = 42;
    game_state_board_history_pop(&history, &board);
    CHECK(board.score == 42);

    // Push more boards than HISTORY_SIZE: the oldest are dropped.
    for(uint32_t i = 1; i <= HISTORY_SIZE + 2; i++) {
        board.score = i;
        game_state_board_history_push(&history, &board);
    }
    for(uint32_t i = HISTORY_SIZE + 2; i > 2; i--) {
        game_state_board_history_pop(&history, &board);
        CHECK(board.score == i);
    }
    board.score = 0;
    game_state_board_history_pop(&history, &board);
    CHECK(board.score == 0);
}

static void test_game_state_move_and_undo(void) {
    GameState state;
    game_state_init(&state);

    memset(state.board.table, 0, sizeof(GameBoardTable));
    state.board.table[0][0] = 1;
    state.board.table[0][1] = 1;
    state.board.score = 0;
    state.board.moves = 0;

    GameBoardTable before;
    game_board_table_copy(state.board.table, before);

    game_state_send(&state, GameMoveLeft);
    CHECK(state.board.score == 4);
    CHECK(state.board.moves == 1);
    CHECK(state.board.table[0][0] == 2);

    game_state_send(&state, GameMoveUndo);
    CHECK(state.board.score == 0);
    CHECK(state.board.moves == 0);
    CHECK(table_equals(state.board.table, (const uint8_t(*)[CELLS_COUNT])before));
}

static void test_record_broken_flag(void) {
    GameState state;
    game_state_init(&state);

    // Game over with a score above the previous record.
    state.top_score = 100;
    table_set(state.board.table, dead_board);
    state.board.score = 200;
    game_state_post_update(&state);
    CHECK(state.is_over);
    CHECK(state.is_record_broken);
    CHECK(state.top_score == 200);

    // A redundant update while already over must keep the flag.
    game_state_post_update(&state);
    CHECK(state.is_record_broken);

    // Game over below the record: no flag, record kept.
    game_state_init(&state);
    state.top_score = 100;
    table_set(state.board.table, dead_board);
    state.board.score = 50;
    game_state_post_update(&state);
    CHECK(state.is_over);
    CHECK(!state.is_record_broken);
    CHECK(state.top_score == 100);
}

static bool load_valid(GameState* state) {
    state->top_score = 123;
    return true;
}

static bool load_failed(GameState* state) {
    (void)state;
    return false;
}

static bool load_bad_cell(GameState* state) {
    state->board.table[0][0] = MAX_CELL_VALUE + 1;
    return true;
}

static bool load_bad_history_top(GameState* state) {
    state->history.top = HISTORY_SIZE;
    return true;
}

static bool load_bad_history_cell(GameState* state) {
    state->history.top = 0;
    state->history.items[0].table[1][2] = MAX_CELL_VALUE + 1;
    return true;
}

static void test_load_validation(void) {
    GameState state;
    game_state_init(&state);

    CHECK(game_state_load(&state, load_valid));
    CHECK(state.top_score == 123);

    GameState untouched = state;
    CHECK(!game_state_load(&state, load_failed));
    CHECK(!game_state_load(&state, load_bad_cell));
    CHECK(!game_state_load(&state, load_bad_history_top));
    CHECK(!game_state_load(&state, load_bad_history_cell));
    // Failed loads must not leak partially-read data into the live state.
    CHECK(memcmp(&state, &untouched, sizeof(GameState)) == 0);
}

int main(void) {
    srandom(42);

    test_move_left_shift();
    test_move_left_merge_with_gap();
    test_move_left_no_double_merge();
    test_move_left_two_pairs();
    test_move_noop_is_not_an_update();
    test_move_directions();
    test_can_move_and_game_over();
    test_push_random_digit();
    test_history_lifo_and_overflow();
    test_game_state_move_and_undo();
    test_record_broken_flag();
    test_load_validation();

    if(failures == 0) {
        printf("OK: all tests passed\n");
        return 0;
    }
    printf("%d check(s) failed\n", failures);
    return 1;
}
