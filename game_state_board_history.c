#include "game_state_board_history.h"

#include <string.h>

void game_state_board_history_init(GameStateBoardHistory* history) {
    history->top = -1;
}

void game_state_board_history_push(GameStateBoardHistory* history, GameStateBoard const* board) {
    if(history->top >= HISTORY_SIZE - 1) {
        // Shift elements
        for(uint8_t i = 1; i <= history->top; i++) {
            memcpy(&history->items[i - 1], &history->items[i], sizeof(GameStateBoard));
        }
    } else {
        history->top++;
    }

    memcpy(&history->items[history->top], board, sizeof(GameStateBoard));
}

void game_state_board_history_pop(GameStateBoardHistory* history, GameStateBoard* board) {
    if(history->top >= 0) {
        memcpy(board, &history->items[history->top], sizeof(GameStateBoard));
        history->top--;
    }
}
