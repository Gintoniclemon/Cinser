/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: pic.c
 * Descricao: Driver do PIC 8259 (remap, mask/unmask, EOI)
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
#include "pic.h"
#include "io.h"

// ICW1 bits
#define ICW1_ICW4       0x01
#define ICW1_INIT       0x10

// ICW4 bits
#define ICW4_8086       0x01

// Le mascara atual (IMR) de cada PIC
static inline uint8_t pic_get_mask_master(void) { return inb(PIC1_DATA); }
static inline uint8_t pic_get_mask_slave(void)  { return inb(PIC2_DATA); }

static inline void pic_set_mask_master(uint8_t mask) { outb(PIC1_DATA, mask); }
static inline void pic_set_mask_slave(uint8_t mask)  { outb(PIC2_DATA, mask); }

static void pic_remap(uint8_t offset1, uint8_t offset2) {
    // salva mascaras atuais
    uint8_t a1 = pic_get_mask_master();
    uint8_t a2 = pic_get_mask_slave();

    // comeca init sequence
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // ICW2: novos offsets (vetores base)
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();

    // ICW3: wiring (master tem slave no IRQ2, slave tem identity 2)
    outb(PIC1_DATA, 0x04); // 0000 0100 (bit 2 ligado)
    io_wait();
    outb(PIC2_DATA, 0x02); // slave id = 2
    io_wait();

    // ICW4: 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // restaura mascaras antigas
    pic_set_mask_master(a1);
    pic_set_mask_slave(a2);
}

void pic_init(void) {
    // remapeia para evitar colisao com excecoes CPU (0x00-0x1F)
    pic_remap(PIC1_OFFSET, PIC2_OFFSET);

    // Por padrao, mascara tudo e voce libera so o que quiser.
    pic_set_mask_master(0xFF);
    pic_set_mask_slave(0xFF);
}

void pic_send_eoi(uint8_t irq) {
    // Se a interrupcao veio do slave (IRQ >= 8), manda EOI pro slave tambem
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_mask_irq(uint8_t irq) {
    if (irq < 8) {
        uint8_t mask = pic_get_mask_master();
        mask |= (uint8_t)(1u << irq);
        pic_set_mask_master(mask);
    } else {
        irq = (uint8_t)(irq - 8);
        uint8_t mask = pic_get_mask_slave();
        mask |= (uint8_t)(1u << irq);
        pic_set_mask_slave(mask);
    }
}

void pic_unmask_irq(uint8_t irq) {
    if (irq < 8) {
        uint8_t mask = pic_get_mask_master();
        mask &= (uint8_t)~(1u << irq);
        pic_set_mask_master(mask);
    } else {
        irq = (uint8_t)(irq - 8);
        uint8_t mask = pic_get_mask_slave();
        mask &= (uint8_t)~(1u << irq);
        pic_set_mask_slave(mask);
    }
}
