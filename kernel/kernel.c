/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: kernel.c
 * Descrição: Núcleo do sistema operacional / Gerenciamento de processos.
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
#include "vga.h"
#include "idt.h"
#include "pic.h"

#define MULTIBOOT_MAGIC 0x2BADB002u

static void print_hex32(uint32_t v) {
    const char *hex = "0123456789ABCDEF";
    vga_write("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t n = (v >> (i * 4)) & 0xF;
        vga_putc(hex[n]);
    }
}

void kernel_main(uint32_t magic, uint32_t mb_info) {
    // Inicialize VGA e imprima algo ANTES de habilitar IRQs.
    // Se voce fizer `sti` com IRQ0/IRQ1 liberadas enquanto todos os vetores
    // ainda apontam para `isr_halt`, o PIT/teclado vao disparar e o CPU para
    // antes de voce ver qualquer texto.
    vga_init();
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("Tervia Cinser\n");
    vga_write("A i386 Operating System\n\n");

    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_write("[OK] VGA init\n");

    // IDT + PIC (mantem tudo mascarado por padrao)
    idt_init();
    vga_write("[OK] IDT init\n");

    pic_init();
    vga_write("[OK] PIC remap/mask\n\n");

    // IMPORTANTE: nao desmascare IRQ0/IRQ1 nem chame `sti` ate voce ter
    // handlers de IRQ reais (timer/keyboard) e mandar EOI corretamente.
    // Se voce quiser testar com IRQs mais tarde, descomente:
    // pic_unmask_irq(0);
    // pic_unmask_irq(1);
    // asm volatile ("sti");

    vga_write("Multiboot magic: ");
    if (magic == MULTIBOOT_MAGIC) {
		vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_write("OK\n");
    } else {
		vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("INVALID (");
        print_hex32(magic);
        vga_write(")\n");
    }
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_write("Multiboot info ptr: ");
    print_hex32(mb_info);
    vga_write("\n\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("Cinser Kernel OK!\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
