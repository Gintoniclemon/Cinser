/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: mouse.c
 * Descricao: Driver de mouse PS/2 (IRQ12) - pacotes 3 bytes.
 * Copyright (C) 2026 Tervia Corporation.
 *
 * Este programa e um software livre: voce pode redistribui-lo e/ou
 * modifica-lo sob os termos da Licenca Publica Geral GNU como publicada
 * pela Free Software Foundation, bem como a versao 3 da Licenca.
 *
 * Este programa e distribuido na esperanca de que possa ser util,
 * mas SEM NENHUMA GARANTIA; sem uma garantia implicita de ADEQUACAO
 * a qualquer MERCADO ou APLICACAO EM PARTICULAR. Veja a
 * Licenca Publica Geral GNU para mais detalhes.
 ****************************************************************************/

#include <stdint.h>
#include "io.h"
#include "irq.h"
#include "pic.h"
#include "mouse.h"

// PS/2 controller ports
#define PS2_DATA   0x60
#define PS2_STAT   0x64
#define PS2_CMD    0x64

// Controller commands
#define CMD_READ_STATUS   0x20
#define CMD_WRITE_STATUS  0x60
#define CMD_ENABLE_AUX    0xA8
#define CMD_WRITE_AUX     0xD4

// Mouse device commands
#define MOUSE_SET_DEFAULTS 0xF6
#define MOUSE_ENABLE_DATA  0xF4

// Responses
#define MOUSE_ACK          0xFA

static volatile mouse_state_t g_state;
static volatile int32_t g_min_x = 0;
static volatile int32_t g_min_y = 0;
static volatile int32_t g_max_x = -1;
static volatile int32_t g_max_y = -1;

static volatile uint8_t g_pkt[3];
static volatile uint8_t g_pkt_i = 0;
static volatile uint8_t g_dirty = 0;

static inline int ps2_can_read(void) {
    return (inb(PS2_STAT) & 0x01) ? 1 : 0;
}

static inline int ps2_can_write(void) {
    return (inb(PS2_STAT) & 0x02) ? 0 : 1;
}

static void ps2_wait_read(void) {
    for (int i = 0; i < 100000; i++) {
        if (ps2_can_read()) return;
    }
}

static void ps2_wait_write(void) {
    for (int i = 0; i < 100000; i++) {
        if (ps2_can_write()) return;
    }
}

static uint8_t ps2_read_data(void) {
    ps2_wait_read();
    return inb(PS2_DATA);
}

static void ps2_write_cmd(uint8_t cmd) {
    ps2_wait_write();
    outb(PS2_CMD, cmd);
}

static void ps2_write_data(uint8_t data) {
    ps2_wait_write();
    outb(PS2_DATA, data);
}

static uint8_t mouse_write(uint8_t val) {
    // Write next byte to mouse (aux device)
    ps2_write_cmd(CMD_WRITE_AUX);
    ps2_write_data(val);
    return ps2_read_data();
}

static void mouse_irq(regs_t *r) {
    (void)r;

    // Bit 5 = AUX data available; Bit 0 = output buffer full
    uint8_t st = inb(PS2_STAT);
    if (!(st & 0x01) || !(st & 0x20)) {
        return; // not mouse data
    }

    uint8_t b = inb(PS2_DATA);

    // Sync: bit3 of first byte must be 1
    if (g_pkt_i == 0 && !(b & 0x08)) {
        return;
    }

    g_pkt[g_pkt_i++] = b;
    if (g_pkt_i < 3) return;
    g_pkt_i = 0;

    uint8_t b0 = g_pkt[0];
    uint8_t b1 = g_pkt[1];
    uint8_t b2 = g_pkt[2];

    // Overflow flags? Drop packet to avoid crazy jumps.
    if (b0 & 0xC0) {
        return;
    }

    int32_t dx = (int32_t)(int8_t)b1;
    int32_t dy = (int32_t)(int8_t)b2;

    // PS/2 Y is negative when moving up; in screen coords, up = y-.
    // So: y -= dy.
    int32_t new_x = g_state.x + dx;
    int32_t new_y = g_state.y - dy;

    // Optional clamping if bounds are sane
    if (g_max_x >= g_min_x) {
        if (new_x < g_min_x) new_x = g_min_x;
        if (new_x > g_max_x) new_x = g_max_x;
    }
    if (g_max_y >= g_min_y) {
        if (new_y < g_min_y) new_y = g_min_y;
        if (new_y > g_max_y) new_y = g_max_y;
    }

    uint8_t buttons = (uint8_t)(b0 & 0x07);

    // Update global state
    g_state.dx += dx;
    g_state.dy += (int32_t)(-dy); // keep dy in screen coordinates
    g_state.x = new_x;
    g_state.y = new_y;
    g_state.buttons = buttons;

    g_dirty = 1;
}

void mouse_set_bounds(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y) {
    g_min_x = min_x;
    g_min_y = min_y;
    g_max_x = max_x;
    g_max_y = max_y;

    // Clamp current position immediately
    int32_t x = g_state.x;
    int32_t y = g_state.y;
    if (g_max_x >= g_min_x) {
        if (x < g_min_x) x = g_min_x;
        if (x > g_max_x) x = g_max_x;
    }
    if (g_max_y >= g_min_y) {
        if (y < g_min_y) y = g_min_y;
        if (y > g_max_y) y = g_max_y;
    }
    g_state.x = x;
    g_state.y = y;
}

void mouse_get_state(mouse_state_t *out) {
    if (!out) return;
    // Snapshot (best-effort; state is small)
    out->x = g_state.x;
    out->y = g_state.y;
    out->dx = g_state.dx;
    out->dy = g_state.dy;
    out->buttons = g_state.buttons;
}

int mouse_poll(mouse_state_t *out) {
    if (!out) return 0;
    int had = g_dirty ? 1 : 0;
    mouse_get_state(out);
    // consume deltas
    g_state.dx = 0;
    g_state.dy = 0;
    g_dirty = 0;
    return had;
}

void mouse_init(void) {
    // reset state
    g_state.x = 0;
    g_state.y = 0;
    g_state.dx = 0;
    g_state.dy = 0;
    g_state.buttons = 0;
    g_pkt_i = 0;
    g_dirty = 0;

    // Enable AUX (mouse) device on the controller
    ps2_write_cmd(CMD_ENABLE_AUX);

    // Enable IRQ12 in controller config byte
    ps2_write_cmd(CMD_READ_STATUS);
    uint8_t status = ps2_read_data();
    status |= 0x02; // enable IRQ12
    status |= 0x01; // enable IRQ1 (keyboard) just in case
    ps2_write_cmd(CMD_WRITE_STATUS);
    ps2_write_data(status);

    // Tell mouse to use defaults and start streaming
    (void)mouse_write(MOUSE_SET_DEFAULTS);
    (void)mouse_write(MOUSE_ENABLE_DATA);

    // Install IRQ12 handler
    irq_install_handler(12, mouse_irq);

    // IMPORTANT: IRQ12 is on the slave PIC, so master IRQ2 (cascade) must be unmasked.
    pic_unmask_irq(2);
    pic_unmask_irq(12);
}
