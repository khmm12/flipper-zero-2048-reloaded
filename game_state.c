#include "game_state.h"

#include <string.h>

static void game_state_reset(GameState* const game_state);
static void game_state_undo(GameState* const state);
static void
    game_state_apply_move_result(GameState* const state, const MoveResult* const move_result);
static void game_state_save_score(GameState* const state);
static bool game_state_board_table_is_valid(const GameStateBoard* const board);

// Not static: the host unit tests call this directly to exercise game-over
// handling without depending on random tile spawns.
void game_state_post_update(GameState* const state);

void game_state_init(GameState* const game_state) {
    game_state->top_score = 0;
    game_state_reset(game_state);
}

void game_state_send(GameState* state, GameEvent event) {
    MoveResult move_result;

    switch(event) {
    case GameMoveLeft:
        game_board_table_move_left(state->board.table, &move_result);
        break;
    case GameMoveRight:
        game_board_table_move_right(state->board.table, &move_result);
        break;
    case GameMoveUp:
        game_board_table_move_up(state->board.table, &move_result);
        break;
    case GameMoveDown:
        game_board_table_move_down(state->board.table, &move_result);
        break;
    case GameMoveUndo:
        game_state_undo(state);
        return;
    case GameReset:
        game_state_reset(state);
        return;
    default:
        return;
    }

    game_state_apply_move_result(state, &move_result);
}

bool game_state_dump(GameState* const state, GameStateDumpCallback cb) {
    return cb(state);
}

bool game_state_load(GameState* state, GameStateLoadCallback cb) {
    GameState tmp;
    game_state_init(&tmp);

    if(cb(&tmp) && game_state_is_valid(&tmp)) {
        memcpy(state, &tmp, sizeof(GameState));
        return true;
    }

    return false;
}

void game_state_reset(GameState* const state) {
    // Reset board
    game_state_board_init(&state->board);

    // Reset history stack
    game_state_board_history_init(&state->history);

    // Reset game state
    state->is_over = false;
    state->is_record_broken = false;
}

void game_state_undo(GameState* const state) {
    game_state_board_history_pop(&state->history, &state->board);
    game_state_post_update(state);
}

void game_state_apply_move_result(GameState* const state, const MoveResult* const move_result) {
    if(!move_result->is_table_updated) return;

    // Save the current state to the history stack
    game_state_board_history_push(&state->history, &state->board);

    // Apply the move to the board
    state->board.score += move_result->score_points;
    state->board.moves++;
    game_board_table_copy(move_result->new_table, state->board.table);

    // Add a new random digit to the board
    game_board_table_push_random_digit(state->board.table);

    // Apply the move to the game state
    game_state_post_update(state);
}

void game_state_post_update(GameState* const state) {
    bool is_over = game_state_board_is_over(&state->board);

    // Capture the record only on the transition into game over, so redundant
    // updates while already over don't clear is_record_broken.
    if(is_over && !state->is_over) {
        game_state_save_score(state);
    }

    state->is_over = is_over;
}

void game_state_save_score(GameState* const state) {
    state->is_record_broken = state->board.score > state->top_score;
    if(state->is_record_broken) {
        state->top_score = state->board.score;
    }
}

// A save file is copied into GameState verbatim, so an invalid or corrupted
// file could otherwise smuggle in out-of-range values: a cell above
// MAX_CELL_VALUE would index past the digits sprite atlas (the drawer's range
// check is only a second line of defense), and history.top outside its range
// makes the history stack read/write out of bounds.
bool game_state_is_valid(const GameState* const state) {
    if(!game_state_board_table_is_valid(&state->board)) return false;

    if(state->history.top < -1 || state->history.top >= HISTORY_SIZE) return false;
    for(int8_t i = 0; i <= state->history.top; i++) {
        if(!game_state_board_table_is_valid(&state->history.items[i])) return false;
    }

    return true;
}

bool game_state_board_table_is_valid(const GameStateBoard* const board) {
    for(uint8_t i = 0; i < CELLS_COUNT; i++) {
        for(uint8_t j = 0; j < CELLS_COUNT; j++) {
            if(board->table[i][j] > MAX_CELL_VALUE) return false;
        }
    }

    return true;
}
