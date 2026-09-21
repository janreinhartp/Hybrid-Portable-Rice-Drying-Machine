#include "screen_manager.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "esp_log.h"
#include "hmi_command.h"
#include "hmi_state.h"
#include "lvgl.h"

static const char *TAG = "screen_manager";

typedef enum {
    SCREEN_HOME = 0,
    SCREEN_DRYING,
    SCREEN_MANUAL,
    SCREEN_COUNT,
} screen_id_t;

typedef struct {
    lv_obj_t *clock;
    lv_obj_t *machine_state;
    lv_obj_t *upper_conditions;
    lv_obj_t *lower_conditions;
    lv_obj_t *moisture;
    lv_obj_t *actuators;
    lv_obj_t *storage;
} home_screen_t;

static lv_obj_t *screens[SCREEN_COUNT];
static home_screen_t home_screen;
static lv_obj_t *drying_details;
static lv_obj_t *manual_details;

static const char *actuator_text(hmi_actuator_state_t state)
{
    switch (state) {
    case HMI_ACTUATOR_ON:
        return "ON";
    case HMI_ACTUATOR_FAULT:
        return "FAULT";
    case HMI_ACTUATOR_INTERLOCKED:
        return "INTERLOCKED";
    case HMI_ACTUATOR_OFF:
    default:
        return "OFF";
    }
}

static const char *machine_state_text(hmi_machine_state_t state)
{
    switch (state) {
    case HMI_MACHINE_DRYING:
        return "DRYING";
    case HMI_MACHINE_PAUSED:
        return "PAUSED";
    case HMI_MACHINE_COMPLETE:
        return "COMPLETE";
    case HMI_MACHINE_FAULT:
        return "FAULT";
    case HMI_MACHINE_EMERGENCY_STOP:
        return "EMERGENCY STOP";
    case HMI_MACHINE_IDLE:
    default:
        return "IDLE";
    }
}

static lv_obj_t *create_label(lv_obj_t *parent, const char *text, lv_coord_t x, lv_coord_t y)
{
    lv_obj_t *label = lv_label_create(parent);
    if (label == NULL) {
        return NULL;
    }

    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    return label;
}

static void show_screen_event(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
        const screen_id_t screen_id = (screen_id_t)(uintptr_t)lv_event_get_user_data(event);
        if (screen_id < SCREEN_COUNT && screens[screen_id] != NULL) {
            lv_scr_load(screens[screen_id]);
        }
    }
}

static void add_navigation(lv_obj_t *screen)
{
    const char *names[] = {"HOME", "DRYING", "MANUAL"};
    for (screen_id_t screen_id = SCREEN_HOME; screen_id < SCREEN_COUNT; screen_id++) {
        lv_obj_t *button = lv_btn_create(screen);
        lv_obj_set_size(button, 150, 42);
        lv_obj_set_pos(button, 24 + (lv_coord_t)screen_id * 165, 535);
        lv_obj_add_event_cb(button, show_screen_event, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)screen_id);
        create_label(button, names[screen_id], 20, 10);
    }
}

static void manual_actuator_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    const uintptr_t packed = (uintptr_t)lv_event_get_user_data(event);
    hmi_command_t command = {
        .type = HMI_COMMAND_SET_ACTUATOR,
        .payload.actuator = {
            .actuator = (hmi_actuator_t)(packed >> 1),
            .on = (packed & 1U) != 0,
        },
    };
    (void)hmi_command_send(&command, 0);
}

static void add_manual_control(lv_obj_t *screen, hmi_actuator_t actuator, const char *name, lv_coord_t y)
{
    lv_obj_t *label = create_label(screen, name, 40, y + 10);
    lv_obj_t *off_button = lv_btn_create(screen);
    lv_obj_t *on_button = lv_btn_create(screen);
    if (label == NULL || off_button == NULL || on_button == NULL) {
        return;
    }

    lv_obj_set_size(off_button, 120, 42);
    lv_obj_set_size(on_button, 120, 42);
    lv_obj_set_pos(off_button, 300, y);
    lv_obj_set_pos(on_button, 440, y);
    lv_obj_add_event_cb(off_button, manual_actuator_event, LV_EVENT_CLICKED,
                        (void *)(uintptr_t)(actuator << 1));
    lv_obj_add_event_cb(on_button, manual_actuator_event, LV_EVENT_CLICKED,
                        (void *)(uintptr_t)((actuator << 1) | 1U));
    create_label(off_button, "OFF", 36, 10);
    create_label(on_button, "ON", 40, 10);
}

static void update_home_screen(lv_timer_t *timer)
{
    hmi_state_t state;
    char datetime_text[32];
    char upper_text[64];
    char lower_text[64];
    char moisture_text[64];
    char actuator_text_buffer[160];
    char storage_text[64];
    (void)timer;

    if (hmi_state_read(&state) != ESP_OK) {
        return;
    }

    if (state.rtc_valid) {
        struct tm local_time;
        localtime_r(&state.current_datetime, &local_time);
        strftime(datetime_text, sizeof(datetime_text), "%d %b %Y  %H:%M:%S", &local_time);
    } else {
        snprintf(datetime_text, sizeof(datetime_text), "RTC INVALID");
    }

    snprintf(upper_text, sizeof(upper_text), "UPPER  %.1f C   %.1f %%RH",
             state.upper_temperature_c, state.upper_humidity_percent);
    snprintf(lower_text, sizeof(lower_text), "LOWER  %.1f C   %.1f %%RH",
             state.lower_temperature_c, state.lower_humidity_percent);
    snprintf(moisture_text, sizeof(moisture_text), "MOISTURE  %.1f %%   TARGET  %.1f %%",
             state.moisture_filtered, state.target_moisture_percent);
    snprintf(actuator_text_buffer, sizeof(actuator_text_buffer),
             "HEATER %s   FAN %s\nELEVATOR %s   DISCHARGE %s",
             actuator_text(state.heater.actual), actuator_text(state.fan.actual),
             actuator_text(state.elevator.actual), actuator_text(state.discharge.actual));
    snprintf(storage_text, sizeof(storage_text), "SD %s",
             state.sd_available ? "READY" : "NOT AVAILABLE");

    lv_label_set_text(home_screen.clock, datetime_text);
    lv_label_set_text(home_screen.machine_state, machine_state_text(state.machine_state));
    lv_label_set_text(home_screen.upper_conditions, upper_text);
    lv_label_set_text(home_screen.lower_conditions, lower_text);
    lv_label_set_text(home_screen.moisture, moisture_text);
    lv_label_set_text(home_screen.actuators, actuator_text_buffer);
    lv_label_set_text(home_screen.storage, storage_text);
}

static void update_other_screens(lv_timer_t *timer)
{
    hmi_state_t state;
    char drying_text[192];
    char manual_text[256];
    (void)timer;

    if (hmi_state_read(&state) != ESP_OK) {
        return;
    }

    snprintf(drying_text, sizeof(drying_text),
             "DRYING\n\nSTATE: %s\nMOISTURE: %.1f %%\nTARGET: %.1f %%\nUPPER: %.1f C\nLOWER: %.1f C\nDIFFERENTIAL: %.1f C\nELAPSED: %lu s\nSESSION: %s",
             machine_state_text(state.machine_state), state.moisture_filtered,
             state.target_moisture_percent, state.upper_temperature_c,
             state.lower_temperature_c, state.temperature_difference_c,
             (unsigned long)state.drying_elapsed_seconds,
             state.session_id[0] == '\0' ? "NONE" : state.session_id);
    snprintf(manual_text, sizeof(manual_text),
             "MANUAL CONTROL\n\nHEATER  %s / %s\nFAN  %s / %s\nELEVATOR  %s / %s\nDISCHARGE  %s / %s",
             actuator_text(state.heater.requested), actuator_text(state.heater.actual),
             actuator_text(state.fan.requested), actuator_text(state.fan.actual),
             actuator_text(state.elevator.requested), actuator_text(state.elevator.actual),
             actuator_text(state.discharge.requested), actuator_text(state.discharge.actual));
    lv_label_set_text(drying_details, drying_text);
    lv_label_set_text(manual_details, manual_text);
}

esp_err_t screen_manager_init(void)
{
    screens[SCREEN_HOME] = lv_obj_create(NULL);
    screens[SCREEN_DRYING] = lv_obj_create(NULL);
    screens[SCREEN_MANUAL] = lv_obj_create(NULL);
    if (screens[SCREEN_HOME] == NULL || screens[SCREEN_DRYING] == NULL || screens[SCREEN_MANUAL] == NULL) {
        return ESP_ERR_NO_MEM;
    }

    for (screen_id_t screen_id = SCREEN_HOME; screen_id < SCREEN_COUNT; screen_id++) {
        lv_obj_set_style_pad_all(screens[screen_id], 24, 0);
        add_navigation(screens[screen_id]);
    }

    lv_obj_t *title = create_label(screens[SCREEN_HOME], "HYBRID RICE DRYER", 0, 0);
    home_screen.clock = create_label(screens[SCREEN_HOME], "RTC INVALID", 700, 0);
    home_screen.machine_state = create_label(screens[SCREEN_HOME], "IDLE", 0, 80);
    home_screen.upper_conditions = create_label(screens[SCREEN_HOME], "UPPER  --.- C   --.- %RH", 0, 160);
    home_screen.lower_conditions = create_label(screens[SCREEN_HOME], "LOWER  --.- C   --.- %RH", 0, 205);
    home_screen.moisture = create_label(screens[SCREEN_HOME], "MOISTURE  --.- %   TARGET  --.- %", 0, 280);
    home_screen.actuators = create_label(screens[SCREEN_HOME], "HEATER OFF   FAN OFF\nELEVATOR OFF   DISCHARGE OFF", 0, 370);
    home_screen.storage = create_label(screens[SCREEN_HOME], "SD NOT AVAILABLE", 0, 470);

    drying_details = create_label(screens[SCREEN_DRYING], "DRYING", 40, 30);
    add_manual_control(screens[SCREEN_MANUAL], HMI_ACTUATOR_HEATER, "HEATER", 80);
    add_manual_control(screens[SCREEN_MANUAL], HMI_ACTUATOR_FAN, "FAN", 160);
    add_manual_control(screens[SCREEN_MANUAL], HMI_ACTUATOR_ELEVATOR, "ELEVATOR", 240);
    add_manual_control(screens[SCREEN_MANUAL], HMI_ACTUATOR_DISCHARGE, "DISCHARGE", 320);
    manual_details = create_label(screens[SCREEN_MANUAL], "MANUAL CONTROL", 40, 410);

    if (title == NULL || home_screen.clock == NULL || home_screen.machine_state == NULL ||
        home_screen.upper_conditions == NULL || home_screen.lower_conditions == NULL ||
        home_screen.moisture == NULL || home_screen.actuators == NULL || home_screen.storage == NULL ||
        drying_details == NULL || manual_details == NULL) {
        return ESP_ERR_NO_MEM;
    }

    lv_scr_load(screens[SCREEN_HOME]);
    lv_timer_create(update_home_screen, 250, NULL);
    lv_timer_create(update_other_screens, 250, NULL);
    ESP_LOGI(TAG, "Home, drying, and manual screens initialized");
    return ESP_OK;
}
