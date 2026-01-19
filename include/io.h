#ifndef IO_H
#define IO_H

#include <stdint.h>

// Port I/O helpers for x86 (i386)
// Implemented as static inline so no linker symbols are required.

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    // Traditionally used for small delays; port 0x80 is unused on modern PCs.
    outb(0x80, 0);
}

#endif
