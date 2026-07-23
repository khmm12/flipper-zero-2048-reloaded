#define GAME_CONTROLLER_INTERNAL
#include "game_controller.h"

#include <input/input.h>

void ui_state_menu_handle(
    GameController* gamectrl,
    InputEvent input,
    GameControllerInputHandlerResult* out) {
    if(input.type != InputTypePress) return;

    switch(input.key) {
    case InputKeyUp:
        gamectrl->selected_menu_item--;
        if(gamectrl->selected_menu_item < 0) {
            gamectrl->selected_menu_item = MENU_ITEMS_COUNT - 1;
        }
        out->is_handled = true;
        break;
    case InputKeyDown:
        gamectrl->selected_menu_item++;
        if(gamectrl->selected_menu_item >= MENU_ITEMS_COUNT) {
            gamectrl->selected_menu_item = 0;
        }
        out->is_handled = true;
        break;
    case InputKeyOk:
        if(gamectrl->selected_menu_item == 0) {
            // Continue
            game_controller_close_menu(gamectrl);
            out->is_handled = true;
        } else if(gamectrl->selected_menu_item == 1) {
            // new game
            game_state_send(&gamectrl->state, GameReset);
            game_controller_save_state(gamectrl);
            game_controller_close_menu(gamectrl);
            out->is_handled = true;
        }
        break;
    case InputKeyBack:
        game_controller_close_menu(gamectrl);
        out->is_handled = true;
        break;
    default:
        break;
    }

    return;
}

void ui_state_in_progress_handle(
    GameController* gamectrl,
    InputEvent input,
    GameControllerInputHandlerResult* out) {
    switch(input.key) {
    case InputKeyLeft:
        if(input.type != InputTypePress) break;
        game_state_send(&gamectrl->state, GameMoveLeft);
        out->is_handled = true;
        break;
    case InputKeyRight:
        if(input.type != InputTypePress) break;
        game_state_send(&gamectrl->state, GameMoveRight);
        out->is_handled = true;
        break;
    case InputKeyUp:
        if(input.type != InputTypePress) break;
        game_state_send(&gamectrl->state, GameMoveUp);
        out->is_handled = true;
        break;
    case InputKeyDown:
        if(input.type != InputTypePress) break;
        game_state_send(&gamectrl->state, GameMoveDown);
        out->is_handled = true;
        break;
    case InputKeyOk:
        if(input.type != InputTypePress) break;
        game_controller_show_menu(gamectrl);
        out->is_handled = true;
        break;
    case InputKeyBack:
        if(input.type != InputTypeShort) break;
        game_state_send(&gamectrl->state, GameMoveUndo);
        out->is_handled = true;
        break;
    default:
        break;
    }

    if(gamectrl->state.is_over) {
        gamectrl->ui_state = UIStateGameOver;
    }

    return;
}

void ui_state_game_over_handle(
    GameController* gamectrl,
    InputEvent input,
    GameControllerInputHandlerResult* out) {
    switch(input.key) {
    case InputKeyOk:
        if(input.type != InputTypePress) break;
        game_state_send(&gamectrl->state, GameReset);
        game_controller_save_state(gamectrl);
        gamectrl->ui_state = UIStateInProgress;
        out->is_handled = true;
        break;
    case InputKeyBack:
        if(input.type != InputTypeShort) break;
        game_state_send(&gamectrl->state, GameMoveUndo);
        // With an empty history the undo is a no-op and the board stays dead —
        // leave the game over screen up in that case.
        if(!gamectrl->state.is_over) gamectrl->ui_state = UIStateInProgress;
        out->is_handled = true;
        break;
    default:
        break;
    }
}

void game_controller_handle_input(
    GameController* gamectrl,
    InputEvent input,
    GameControllerInputHandlerResult* out) {
    if(input.key == InputKeyBack && input.type == InputTypeLong) {
        game_controller_save_state(gamectrl);
        out->is_handled = true;
        out->should_exit = true;
        return;
    }

    out->is_handled = false;
    out->should_exit = false;

    switch(gamectrl->ui_state) {
    case UIStateMenu:
        return ui_state_menu_handle(gamectrl, input, out);
    case UIStateInProgress:
        return ui_state_in_progress_handle(gamectrl, input, out);
    case UIStateGameOver:
        return ui_state_game_over_handle(gamectrl, input, out);
    }
}
