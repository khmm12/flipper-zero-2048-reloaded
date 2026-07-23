#pragma once
#include <gui/gui.h>
#include "game_state.h"

typedef enum {
    UIStateMenu,
    UIStateInProgress,
    UIStateGameOver,
} UIState;

typedef struct {
    UIState ui_state;
    GameState state;
    int8_t selected_menu_item;
    // The game state changed since the last successful save; lets the periodic
    // autosave skip the SD write when nothing happened.
    bool is_state_dirty;
} GameController;

typedef struct {
    bool is_handled;
    bool should_exit;
} GameControllerInputHandlerResult;

void game_controller_init(GameController* gamectrl);
void game_controller_save_state(GameController* gamectrl);
void game_controller_draw(const GameController* gamectrl, Canvas* const canvas);
void game_controller_handle_input(
    GameController* gamectrl,
    InputEvent input,
    GameControllerInputHandlerResult* out);

#ifdef GAME_CONTROLLER_INTERNAL
#define MENU_ITEMS_COUNT 2
void game_controller_show_menu(GameController* gamectrl);
void game_controller_close_menu(GameController* gamectrl);
void game_controller_send(GameController* gamectrl, GameEvent event);
#endif
