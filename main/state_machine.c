#include <fsm.h>
#include <broadcaster.h>
#include <points_provider.h>
#include <steppers.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>

static void delay1s(){
    vTaskDelay(pdMS_TO_TICKS(1000));
}

static void fsm_state_initializing() {
    delay1s();
    fsm_post_system_event(FSM_SYSTEM_EVENT_BOOT_INIT_OK, NULL);
}

static void fsm_state_unhomed() {}

static void fsm_state_homing() {
    delay1s();
    fsm_post_system_event(FSM_SYSTEM_EVENT_HOMING_DONE, NULL);
}

static void fsm_state_idle() {}

static void fsm_state_starting() {
    points_provider_starting();
    steppers_starting();
    delay1s();
    fsm_post_system_event(FSM_SYSTEM_EVENT_RAMP_UP_DONE, NULL);
}

static void fsm_state_running() {
    steppers_running();
}

static void fsm_state_pausing() {
    points_provider_pausing();
    steppers_pausing();
    delay1s();
    fsm_post_system_event(FSM_SYSTEM_EVENT_RAMP_DOWN_DONE, NULL);
}

static void fsm_state_paused() {

}

static void fsm_state_resuming() {
    points_provider_resuming();
    steppers_resuming();
    delay1s();
    fsm_post_system_event(FSM_SYSTEM_EVENT_RAMP_UP_DONE, NULL);
}

static void fsm_state_stopping() {
    points_provider_stopping();
    steppers_stopping();
    delay1s();
    fsm_post_system_event(FSM_SYSTEM_EVENT_RAMP_DOWN_DONE, NULL);
}

static void fsm_state_rebooting() {}

static void fsm_state_error() {}

static void fsm_state_changed(fsm_state_t state) {
    broadcaster_publish("systemState", fsm_state_to_str(state));
    switch (state) {
        case FSM_STATE_INITIALIZING: return fsm_state_initializing();
        case FSM_STATE_UNHOMED: return fsm_state_unhomed();
        case FSM_STATE_HOMING: return fsm_state_homing();
        case FSM_STATE_IDLE: return fsm_state_idle();
        case FSM_STATE_STARTING: return fsm_state_starting();
        case FSM_STATE_RUNNING: return fsm_state_running();
        case FSM_STATE_PAUSING: return fsm_state_pausing();
        case FSM_STATE_PAUSED: return fsm_state_paused();
        case FSM_STATE_RESUMING: return fsm_state_resuming();
        case FSM_STATE_STOPPING: return fsm_state_stopping();
        case FSM_STATE_REBOOTING: return fsm_state_rebooting();
        case FSM_STATE_ERROR: return fsm_state_error();
        default: break;
    }
}


void state_machine_init() {
    fsm_init(fsm_state_changed, points_provider_has_job);
}
