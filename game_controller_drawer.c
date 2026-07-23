#define GAME_CONTROLLER_INTERNAL
#include "game_controller.h"

#include <furi.h>
#include <gui/gui.h>
#include "digits.h"

#define CELL_INNER_SIZE 14
#define FRAME_LEFT      10
#define FRAME_TOP       1
#define FRAME_SIZE      61

static const char* popup_menu_items[] = {"Resume", "New Game"};

static void game_controller_draw_game(const GameController* gamectrl, Canvas* const canvas);
static void game_controller_draw_menu(const GameController* gamectrl, Canvas* const canvas);
static void game_controller_draw_game_over(const GameController* gamectrl, Canvas* const canvas);
static void ui_draw_table(Canvas* canvas, const GameBoardTable table);
static void ui_draw_stats(Canvas* const canvas, const GameState*);
static void ui_draw_digit(Canvas* canvas, uint8_t row, uint8_t column, uint8_t value);
static void ui_draw_popup_background(Canvas* const canvas);

void game_controller_draw(const GameController* gamectrl, Canvas* const canvas) {
    canvas_clear(canvas);

    game_controller_draw_game(gamectrl, canvas);

    switch(gamectrl->ui_state) {
    case UIStateMenu:
        game_controller_draw_menu(gamectrl, canvas);
        break;
    case UIStateGameOver:
        game_controller_draw_game_over(gamectrl, canvas);
        break;
    default:
        break;
    }
}

static void game_controller_draw_game(const GameController* gamectrl, Canvas* const canvas) {
    ui_draw_table(canvas, gamectrl->state.board.table);
    ui_draw_stats(canvas, &gamectrl->state);
}

static void game_controller_draw_menu(const GameController* gamectrl, Canvas* const canvas) {
    ui_draw_popup_background(canvas);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_rbox(canvas, 28, 16, 72, 32, 4);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, 28, 16, 72, 32, 4);

    for(uint8_t i = 0; i < MENU_ITEMS_COUNT; i++) {
        if(i == gamectrl->selected_menu_item) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 34, 20 + 12 * i, 60, 12);
        }

        canvas_set_color(canvas, i == gamectrl->selected_menu_item ? ColorWhite : ColorBlack);
        canvas_draw_str_aligned(
            canvas, 64, 26 + 12 * i, AlignCenter, AlignCenter, popup_menu_items[i]);
    }
}

static void game_controller_draw_game_over(const GameController* gamectrl, Canvas* const canvas) {
    ui_draw_popup_background(canvas);

    bool record_broken = gamectrl->state.is_record_broken;

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_rbox(canvas, 14, 12, 100, 40, 4);

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, 14, 26, 114, 26);
    canvas_draw_rframe(canvas, 14, 12, 100, 40, 4);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 15, AlignCenter, AlignTop, "Game Over");

    canvas_set_font(canvas, FontSecondary);
    if(record_broken) {
        canvas_draw_str_aligned(canvas, 64, 29, AlignCenter, AlignTop, "New Top Score!!!");
    } else {
        canvas_draw_str_aligned(canvas, 64, 29, AlignCenter, AlignTop, "Your Score");
    }

    uint8_t bufSize = 12;
    char buf[bufSize];

    memset(buf, 0, bufSize);
    snprintf(buf, sizeof(buf), "%lu", gamectrl->state.board.score);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 48, AlignCenter, AlignBottom, buf);
}

static void ui_draw_digit(Canvas* canvas, uint8_t row, uint8_t column, uint8_t value) {
    if(value == 0 || value > MAX_CELL_VALUE) return;

    uint8_t left = FRAME_LEFT + 1 + (column * (CELL_INNER_SIZE + 1));
    uint8_t top = FRAME_TOP + 1 + (row * (CELL_INNER_SIZE + 1));

    for(uint8_t r = 0; r < CELL_INNER_SIZE; r++) {
        for(uint8_t c = 0; c < CELL_INNER_SIZE; c++) {
            if(digits[value - 1][r][c] == 1) {
                canvas_draw_dot(canvas, left + c, top + r);
            }
        }
    }
}

static void ui_draw_table(Canvas* canvas, const GameBoardTable table) {
    canvas_draw_frame(canvas, FRAME_LEFT, FRAME_TOP, FRAME_SIZE, FRAME_SIZE);

    uint8_t offs = FRAME_LEFT + CELL_INNER_SIZE + 1;
    for(uint8_t i = 0; i < CELLS_COUNT - 1; i++) {
        canvas_draw_line(canvas, offs, FRAME_TOP + 1, offs, FRAME_TOP + FRAME_SIZE - 2);
        offs += CELL_INNER_SIZE + 1;
    }

    offs = FRAME_TOP + CELL_INNER_SIZE + 1;
    for(uint8_t i = 0; i < CELLS_COUNT - 1; i++) {
        canvas_draw_line(canvas, FRAME_LEFT + 1, offs, FRAME_LEFT + FRAME_SIZE - 2, offs);
        offs += CELL_INNER_SIZE + 1;
    }

    for(uint8_t row = 0; row < CELLS_COUNT; row++) {
        for(uint8_t column = 0; column < CELLS_COUNT; column++) {
            ui_draw_digit(canvas, row, column, table[row][column]);
        }
    }
}

static void ui_draw_stats(Canvas* const canvas, const GameState* state) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP, AlignRight, AlignTop, "Score");
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 20, AlignRight, AlignTop, "Moves");
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 40, AlignRight, AlignTop, "Top Score");

    uint8_t bufSize = 12;
    char buf[bufSize];

    canvas_set_font(canvas, FontSecondary);

    snprintf(buf, sizeof(buf), "%lu", state->board.score);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 10, AlignRight, AlignTop, buf);

    memset(buf, 0, bufSize);
    snprintf(buf, sizeof(buf), "%lu", state->board.moves);
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 30, AlignRight, AlignTop, buf);

    memset(buf, 0, bufSize);
    snprintf(buf, sizeof(buf), "%lu", state->top_score);
    canvas_draw_str_aligned(canvas, 128, FRAME_TOP + 50, AlignRight, AlignTop, buf);
}

static void ui_draw_popup_background(Canvas* const canvas) {
    canvas_set_color(canvas, ColorWhite);
    for(uint8_t x = 0; x < 128; x += 2) {
        for(uint8_t y = 0; y < 64; y++) {
            canvas_draw_dot(canvas, x + (y % 2 == 1 ? 0 : 1), y);
        }
    }
}
