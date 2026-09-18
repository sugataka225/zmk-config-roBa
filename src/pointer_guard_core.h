/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>

struct roba_guard_frame {
    bool forwarded;
};

/* Each driver report consists of X (sync=false), then Y (sync=true).
 * Drop tiny axes only during typing. Never drop the final sync after forwarding
 * an earlier axis: that would defer movement into a later report.
 * This processor must precede all processors which scale or remap X/Y.
 */
static inline bool roba_guard_filter(struct roba_guard_frame *frame, bool recent_typing,
                                     bool pointer_axis, bool sync, int32_t threshold,
                                     int32_t *value) {
    bool forward = true;
    if (recent_typing && pointer_axis && *value >= -threshold && *value <= threshold) {
        *value = 0;
        forward = sync && frame->forwarded;
    }
    if (forward) {
        frame->forwarded = true;
    }
    if (sync) {
        frame->forwarded = false;
    }
    return forward;
}
