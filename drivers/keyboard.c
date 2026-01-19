/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: keyboard.c
 * Descrição: Driver de teclado PS/2 (set 1) com layout ABNT2.
 * * Copyright (C) 2026 Tervia Corporation.
 *
 * Este programa é um software livre: você pode redistribuí-lo e/ou 
 * modificá-lo sob os termos da Licença Pública Geral GNU como publicada 
 * pela Free Software Foundation, bem como a versão 3 da Licença.
 *
 * Este programa é distribuído na esperança de que possa ser útil, 
 * mas SEM NENHUMA GARANTIA; sem uma garantia implícita de ADEQUAÇÃO 
 * a qualquer MERCADO ou APLICAÇÃO EM PARTICULAR. Veja a 
 * Licença Pública Geral GNU para mais detalhes.
 ****************************************************************************/

#include <stdint.h>
#include "io.h"
#include "pic.h"
#include "irq.h"
#include "isr.h"
#include "keyboard.h"

#define KBD_DATA_PORT 0x60

#define KBD_BUF_SIZE 128
static volatile uint8_t g_buf[KBD_BUF_SIZE];
static volatile uint32_t g_head = 0;
static volatile uint32_t g_tail = 0;

static volatile uint8_t g_shift = 0;
static volatile uint8_t g_caps  = 0;
static volatile uint8_t g_e0    = 0;

/*
 * Mapeamento ABNT2 (baseado em scancode set 1). 
 * Observacao: caracteres acentuados/dead-keys variam por codepage.
 * Aqui usamos ASCII + alguns bytes estendidos comuns (CP437):
 *   'ç' = 0x87, 'Ç' = 0x80.
 */
static const uint16_t g_map[128] = {
    [0x01] = 0,        /* Esc */
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0', [0x0C] = '-', [0x0D] = '=',
    [0x0E] = '\b',     /* Backspace */
    [0x0F] = '\t',     /* Tab */
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']',
    [0x1C] = '\n',     /* Enter */
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = 0x0087,   /* ç */
    [0x28] = '\'', [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
    [0x33] = ',', [0x34] = '.', [0x35] = '/',
    [0x39] = ' ',
    [0x56] = '<',      /* ABNT2 key (OEM_102) */
};

static const uint16_t g_shift_map[128] = {
    [0x01] = 0,
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
    [0x0A] = '(', [0x0B] = ')', [0x0C] = '_', [0x0D] = '+',
    [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R',
    [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',
    [0x18] = 'O', [0x19] = 'P',
    [0x1A] = '{', [0x1B] = '}',
    [0x1C] = '\n',
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F',
    [0x22] = 'G', [0x23] = 'H', [0x24] = 'J', [0x25] = 'K',
    [0x26] = 'L',
    [0x27] = 0x0080,   /* Ç */
    [0x28] = '"', [0x29] = '~',
    [0x2B] = '|',
    [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V',
    [0x30] = 'B', [0x31] = 'N', [0x32] = 'M',
    [0x33] = '<', [0x34] = '>', [0x35] = '?',
    [0x39] = ' ',
    [0x56] = '>',
};

static inline void buf_push(uint8_t c) {
    uint32_t next = (g_head + 1u) % KBD_BUF_SIZE;
    if (next == g_tail) return; /* full: drop */
    g_buf[g_head] = c;
    g_head = next;
}

int keyboard_haschar(void) {
    return g_head != g_tail;
}

int keyboard_getchar(void) {
    if (g_head == g_tail) return -1;
    uint8_t c = g_buf[g_tail];
    g_tail = (g_tail + 1u) % KBD_BUF_SIZE;
    return (int)c;
}

void keyboard_flush(void) {
    g_head = g_tail = 0;
}

int keyboard_capslock(void) { return g_caps ? 1 : 0; }
int keyboard_shift(void)    { return g_shift ? 1 : 0; }

static uint8_t apply_caps(uint8_t ch) {
    if (!g_caps) return ch;
    if (ch >= 'a' && ch <= 'z') return (uint8_t)(ch - 'a' + 'A');
    if (ch >= 'A' && ch <= 'Z') return (uint8_t)(ch - 'A' + 'a');
    return ch;
}

static void keyboard_irq(regs_t *r) {
    (void)r;

    uint8_t sc = inb(KBD_DATA_PORT);

    /* Extended prefix */
    if (sc == 0xE0) { g_e0 = 1; return; }

    /* Key release */
    if (sc & 0x80) {
        uint8_t code = (uint8_t)(sc & 0x7F);

        if (!g_e0) {
            if (code == 0x2A || code == 0x36) g_shift = 0;
        }
        g_e0 = 0;
        return;
    }

    /* Key press */
    if (!g_e0) {
        if (sc == 0x2A || sc == 0x36) { g_shift = 1; return; }
        if (sc == 0x3A) { g_caps ^= 1; return; }
    }

    uint16_t sym = g_shift ? g_shift_map[sc] : g_map[sc];
    g_e0 = 0;
    if (!sym) return;

    uint8_t ch = (uint8_t)(sym & 0xFF);

    /* Caps affects only letters. If shift is active, we already got uppercase; caps toggles it. */
    if (sym < 0x80) {
        if (g_caps) {
            /* If shift selected uppercase letters, caps should flip to lowercase, and vice versa. */
            if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
                ch = apply_caps(ch);
            }
        }
    }

    buf_push(ch);
}

void keyboard_init(void) {
    keyboard_flush();
    g_shift = 0;
    g_caps = 0;
    g_e0 = 0;

    /* Install IRQ1 handler and unmask keyboard IRQ */
    irq_install_handler(1, keyboard_irq);
    pic_unmask_irq(1);
}
