#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <dolphin/dolphin.h>

#include "game_controller.h"
#include "app_shared.h"

#define EVENT_QUEUE_SIZE 8

typedef enum {
    AppEventTypeInput,
    AppEventTypeSaveTick,
} AppEventType;

typedef struct {
    AppEventType type;
    InputEvent input;
} AppEvent;

typedef struct {
    FuriMessageQueue* event_queue;
    FuriMutex* mutex;
    FuriTimer* save_timer;
    Gui* gui;
    ViewPort* view_port;
    GameController* gamectrl;
} Game2048App;

static void game_2048_app_init(Game2048App* app);
static void game_2048_app_deinit(Game2048App* app);
static void game_2048_app_update(Game2048App* app);
static bool game_2048_app_request_update(Game2048App* app);
static void input_callback(InputEvent* input_event, void* ctx);
static void draw_callback(Canvas* const canvas, void* ctx);
static void save_timer_callback(void* ctx);

int32_t game_2048_app() {
    Game2048App* app = malloc(sizeof(Game2048App));
    game_2048_app_init(app);

    dolphin_deed(DolphinDeedPluginGameStart);

    bool is_running = true;
    while(is_running) {
        is_running = game_2048_app_request_update(app);
    }

    game_2048_app_deinit(app);
    free(app);

    return 0;
}

void game_2048_app_init(Game2048App* app) {
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    app->gamectrl = malloc(sizeof(GameController));
    game_controller_init(app->gamectrl);

    app->event_queue = furi_message_queue_alloc(EVENT_QUEUE_SIZE, sizeof(AppEvent));

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);

    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->save_timer = furi_timer_alloc(save_timer_callback, FuriTimerTypePeriodic, app);
    furi_timer_start(app->save_timer, 60 * 1000); // 1 minute
}

void game_2048_app_deinit(Game2048App* app) {
    furi_timer_stop(app->save_timer);
    furi_timer_free(app->save_timer);

    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(app->view_port);

    furi_message_queue_free(app->event_queue);

    furi_mutex_free(app->mutex);

    free(app->gamectrl);
}

void game_2048_app_acquire_lock(Game2048App* app) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);
}

void game_2048_app_release_lock(Game2048App* app) {
    furi_mutex_release(app->mutex);
}

bool game_2048_app_request_update(Game2048App* app) {
    AppEvent event;

    FuriStatus event_status = furi_message_queue_get(app->event_queue, &event, FuriWaitForever);
    if(event_status != FuriStatusOk) return true;

    switch(event.type) {
    case AppEventTypeInput: {
        GameControllerInputHandlerResult r;
        game_2048_app_acquire_lock(app);
        game_controller_handle_input(app->gamectrl, event.input, &r);
        game_2048_app_release_lock(app);
        if(r.is_handled) game_2048_app_update(app);
        return !r.should_exit;
    }
    case AppEventTypeSaveTick:
        // The main loop is the only writer of the game state, so the periodic
        // save can read it without the mutex and never blocks the draw callback
        // for the duration of the storage write.
        game_controller_save_state(app->gamectrl);
        return true;
    }

    return true;
}

void game_2048_app_update(Game2048App* app) {
    view_port_update(app->view_port);
}

void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    Game2048App* app = ctx;
    AppEvent event = {.type = AppEventTypeInput, .input = *input_event};
    furi_message_queue_put(app->event_queue, &event, FuriWaitForever);
}

void draw_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    Game2048App* app = ctx;
    game_2048_app_acquire_lock(app);
    game_controller_draw(app->gamectrl, canvas);
    game_2048_app_release_lock(app);
}

void save_timer_callback(void* ctx) {
    furi_assert(ctx);
    Game2048App* app = ctx;
    // Runs in the timer service thread: never block and never touch storage
    // here, just ask the main loop to save. A dropped tick is fine — the next
    // one fires in a minute.
    AppEvent event = {.type = AppEventTypeSaveTick};
    furi_message_queue_put(app->event_queue, &event, 0);
}
