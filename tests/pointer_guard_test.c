/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "../src/pointer_guard_core.h"

static void check_report(bool recent, int32_t x, int32_t y, bool expect_x, bool expect_y,
                         int32_t out_x, int32_t out_y) {
    struct roba_guard_frame frame = {0};
    assert(roba_guard_filter(&frame, recent, true, false, 2, &x) == expect_x);
    assert(roba_guard_filter(&frame, recent, true, true, 2, &y) == expect_y);
    assert(x == out_x && y == out_y);
    assert(!frame.forwarded);
}

int main(void) {
    check_report(true, 1, -2, false, false, 0, 0); /* Typing vibration */
    check_report(true, 2, 2, false, false, 0, 0);  /* Inclusive threshold */
    check_report(true, 8, 1, true, true, 8, 0);    /* X must flush on Y sync */
    check_report(true, 1, -9, false, true, 0, -9); /* Deliberate Y */
    check_report(true, -8, 9, true, true, -8, 9);  /* Deliberate diagonal */
    check_report(false, 1, -1, true, true, 1, -1); /* Fine positioning */
    check_report(false, 0, 0, true, true, 0, 0);  /* Normal sync preserved */

    struct roba_guard_frame frame = {0};
    int32_t wheel = 1;
    assert(roba_guard_filter(&frame, true, false, true, 2, &wheel));
    assert(wheel == 1); /* Scroll is never filtered */

    /* No discarded movement may leak into a subsequent report. */
    int32_t x = 1, y = 1;
    assert(!roba_guard_filter(&frame, true, true, false, 2, &x));
    assert(!roba_guard_filter(&frame, true, true, true, 2, &y));
    x = 1; y = 0;
    assert(roba_guard_filter(&frame, false, true, false, 2, &x));
    assert(roba_guard_filter(&frame, false, true, true, 2, &y));
    assert(x == 1 && y == 0);
    puts("pointer guard: 9 scenarios passed");
    return 0;
}
