/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT roba_input_processor_typing_guard

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <drivers/input_processor.h>
#include <zmk/events/keycode_state_changed.h>
#include "pointer_guard_core.h"

struct roba_guard_config {
    uint32_t typing_window_ms;
    int32_t motion_threshold;
};

struct roba_guard_data {
    struct roba_guard_frame frame;
};

static atomic_t last_key_time;
static atomic_t key_seen;

static int roba_guard_key_event(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    /* Keyboard keys only; ignore modifiers so Ctrl/Shift + pointer remains usable.
     * Press and release both guard against vibration. No key contents are stored.
     * Use delivery time because hold-tap behaviors may delay the key event.
     */
    if (ev && ev->usage_page == HID_USAGE_KEY && ev->keycode >= 4 &&
        !is_mod(ev->usage_page, ev->keycode)) {
        atomic_set(&last_key_time, (atomic_val_t)k_uptime_get_32());
        atomic_set(&key_seen, 1);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(roba_guard_keys, roba_guard_key_event);
ZMK_SUBSCRIPTION(roba_guard_keys, zmk_keycode_state_changed);

static int roba_guard_handle_event(const struct device *dev, struct input_event *event,
                                   uint32_t param1, uint32_t param2,
                                   struct zmk_input_processor_state *state) {
    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);
    const struct roba_guard_config *cfg = dev->config;
    struct roba_guard_data *data = dev->data;
    bool seen = atomic_get(&key_seen) != 0;
    uint32_t last = (uint32_t)atomic_get(&last_key_time);
    uint32_t elapsed = k_uptime_get_32() - last;
    bool pointer_axis = event->type == INPUT_EV_REL &&
                        (event->code == INPUT_REL_X || event->code == INPUT_REL_Y);
    bool forward = roba_guard_filter(&data->frame, seen && elapsed < cfg->typing_window_ms,
                                     pointer_axis, event->sync, cfg->motion_threshold,
                                     &event->value);
    return forward ? ZMK_INPUT_PROC_CONTINUE : ZMK_INPUT_PROC_STOP;
}

static const struct zmk_input_processor_driver_api roba_guard_api = {
    .handle_event = roba_guard_handle_event,
};

#define ROBA_GUARD_INST(n)                                                                         \
    BUILD_ASSERT(DT_INST_PROP(n, typing_window_ms) >= 0 &&                                         \
                 DT_INST_PROP(n, typing_window_ms) <= 2000, "Invalid typing window");              \
    BUILD_ASSERT(DT_INST_PROP(n, motion_threshold) >= 0 &&                                         \
                 DT_INST_PROP(n, motion_threshold) <= 20, "Invalid motion threshold");             \
    static struct roba_guard_data roba_guard_data_##n;                                             \
    static const struct roba_guard_config roba_guard_config_##n = {                                 \
        .typing_window_ms = DT_INST_PROP(n, typing_window_ms),                                     \
        .motion_threshold = DT_INST_PROP(n, motion_threshold),                                     \
    };                                                                                            \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &roba_guard_data_##n, &roba_guard_config_##n,                \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &roba_guard_api);

DT_INST_FOREACH_STATUS_OKAY(ROBA_GUARD_INST)
