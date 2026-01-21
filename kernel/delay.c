// delay.c - simple sleep/delay utilities for Cinser (freestanding, no libgcc helpers)
//
// This module relies on IRQ0/PIT ticks. It does NOT use 64-bit division to avoid
// pulling in __udivdi3 / libgcc helpers.

#include <stdint.h>
#include "delay.h"

/* Updated on each timer IRQ */
static volatile uint32_t g_ticks = 0;

/* Ticks per second (same value used by time_init / PIT programming) */
static uint32_t g_tps = 1000;

void delay_init(uint32_t ticks_per_sec) {
    if (ticks_per_sec == 0) ticks_per_sec = 1000;
    g_tps = ticks_per_sec;
    g_ticks = 0;
}

void delay_tick(void) {
    g_ticks++;
}

uint32_t delay_get_ticks(void) {
    return g_ticks;
}

/* Internal: CPU halt until next interrupt */
static inline void cpu_halt(void) {
    __asm__ __volatile__("hlt");
}

void delay_ticks(uint32_t ticks) {
    if (ticks == 0) return;

    uint32_t start = g_ticks;

    /* Wait until (g_ticks - start) >= ticks, handles wrap-around naturally */
    while ((uint32_t)(g_ticks - start) < ticks) {
        cpu_halt();
    }
}

void delay_time(uint32_t seconds) {
    if (seconds == 0) return;
    if (g_tps == 0) g_tps = 1000;

    while (seconds--) {
        delay_ticks(g_tps);
    }
}

void delay_ms(uint32_t ms) {
    if (ms == 0) return;
    if (g_tps == 0) g_tps = 1000;

    /*
      Compute ticks = ceil(ms * tps / 1000) without 64-bit math:

        ms = q*1000 + r
        ms*tps/1000 = q*tps + (r*tps)/1000

      We do a ceil by rounding up if there is remainder.
    */
    uint32_t q = ms / 1000;
    uint32_t r = ms % 1000;

    uint32_t ticks = q * g_tps;

    uint32_t r_mul = r * g_tps;           /* <= 999*~1000 fits 32-bit */
    uint32_t extra = r_mul / 1000;
    uint32_t rem   = r_mul % 1000;

    ticks += extra;
    if (rem != 0) ticks += 1;             /* ceil */

    if (ticks == 0) ticks = 1;
    delay_ticks(ticks);
}
