#pragma once
#include <stdint.h>

/*
  delay.h - simple sleep/delay utilities for Cinser

  How it works:
    - delay_tick() MUST be called from your timer IRQ (IRQ0) handler.
    - delay_init(tps) should be called after you initialize PIT/time with the same TPS.

  Notes:
    - delay_ticks()/delay_ms()/delay_time() require interrupts enabled (sti),
      otherwise the CPU will halt forever because no IRQ wakes it.
*/

void delay_init(uint32_t ticks_per_sec);
void delay_tick(void);

uint32_t delay_get_ticks(void);

void delay_ticks(uint32_t ticks);
void delay_time(uint32_t seconds);
void delay_ms(uint32_t ms);
