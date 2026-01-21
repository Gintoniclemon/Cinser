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
#include "idt.h"
#include "pic.h"
#include "irq.h"
#include "io.h"
#include "cmos.h"
#include "time.h"
#include "delay.h"
#include "memory.h"
#include "keyboard.h"
#include "sysconfig.h"
#include "console.h"
#include "video.h"
#include "multiboot.h"
#include "desktop.h"
#include "mouse.h"

#define MULTIBOOT_MAGIC 0x2BADB002u

uint32_t time_get_ticks(void);

void wait_vsync(void) {
    // 1. Se já estamos no meio de um VSync (retrace), espera ele acabar
    // (Isso evita pegar o VSync no finalzinho e não ter tempo de desenhar)
    while (inb(0x3DA) & 8);
    
    // 2. Espera o PRÓXIMO VSync começar
    // Nesse momento exato, o canhão de elétrons volta pro topo da tela.
    // Temos alguns milissegundos para desenhar antes dele descer.
    while (!(inb(0x3DA) & 8));
}

static void print_hex32(uint32_t v) {
    const char *hex = "0123456789ABCDEF";
    vga_write("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t n = (v >> (i * 4)) & 0xF;
        vga_putc(hex[n]);
    }
}

static void dump_bytes(const char *label, const char *s) {
    vga_write(label);
    vga_write(": ");
    for (int i = 0; s[i]; i++) {
        vga_write("0x");
        const char *hex = "0123456789ABCDEF";
        uint8_t b = (uint8_t)s[i];
        vga_putc(hex[(b >> 4) & 0xF]);
        vga_putc(hex[b & 0xF]);
        vga_putc(' ');
    }
    vga_write("\n");
}

static void vga_put2(uint8_t v) {
    vga_putc('0' + (v / 10));
    vga_putc('0' + (v % 10));
}

static void vga_put4(uint16_t v) {
    vga_putc('0' + (v / 1000) % 10);
    vga_putc('0' + (v / 100)  % 10);
    vga_putc('0' + (v / 10)   % 10);
    vga_putc('0' + (v % 10));
}

static volatile uint32_t g_ticks = 0;

static void timer_irq(regs_t *r) {
    (void)r;
    time_tick();
    delay_tick();
}

static void keyboard_irq(regs_t *r) {
    (void)r;
    uint8_t sc = inb(0x60);

    // Ignora key release (bit 7)
    if (sc & 0x80) return;

    vga_write(" [KBD ");
    print_hex32((uint32_t)sc);
    vga_write("] ");
}

void kernel_main(uint32_t magic, uint32_t mb_info) {
    // 1) VGA primeiro: se qualquer coisa travar, voce ainda ve o log
    video_init_system((void*)mb_info);
	
	console_init();
	
	vga_write("VBE 1.2 Universal Video BIOS Driver");
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	vga_write(" [OK]\n\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	vga_write("8-Bit Font Test:\n");
	console_write("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
	console_write("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz\n");
    console_write("abcdefghijklmnopqrstuvwxyz\n");
    console_write("0123456789\n");
	console_write("123123456456789789123456789\n\n");
	
    // Diagnóstico extra: confirma se o GRUB realmente passou framebuffer e se os campos parecem sane.
    multiboot_info_t *mbi = (multiboot_info_t*)(uintptr_t)mb_info;
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_write("MB flags="); print_hex32(mbi->flags); vga_write("\n");
    if (mbi->flags & (1u<<12)) {
        vga_write("FB addr(lo)="); print_hex32((uint32_t)(mbi->framebuffer_addr & 0xFFFFFFFFu)); vga_write("\n");
        vga_write("FB pitch="); print_hex32(mbi->framebuffer_pitch); vga_write("\n");
        vga_write("FB w="); print_hex32(mbi->framebuffer_width); vga_write(" h="); print_hex32(mbi->framebuffer_height); vga_write("\n");
        vga_write("FB bpp="); print_hex32(mbi->framebuffer_bpp); vga_write(" type="); print_hex32(mbi->framebuffer_type); vga_write("\n\n");
    } else {
        vga_write("FB: not present\n\n");
    }
	
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    // Diagnóstico: se strings aparecerem "ROT+1" na tela, isso confirma corrupção/alteração em memória.
    // Este dump mostra os bytes reais do literal antes de imprimir.
    dump_bytes("DBG 'Tervia Cinser' bytes", "Tervia Cinser");
    dump_bytes("DBG 'Operating' bytes", "A i386 Operating System");

    vga_write("Tervia Cinser\n");
    vga_write("A i386 Operating System\n\n");

    vga_write("Multiboot magic: ");
    if (magic == MULTIBOOT_MAGIC) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_write(" [OK]\n");
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
	vga_write("[1] Memory... ");
	memory_init(magic, mb_info);   // use a assinatura do seu kernel novo
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	vga_write(" [OK]\n");

	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	vga_write("[2] SysConfig... ");
	sysconfig_init();
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	vga_write(" [OK]\n");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("[3] IDT... ");
    idt_init();
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write(" [OK]\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("[4] PIC... ");
    pic_init();
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write(" [OK]\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("[5] IRQ layer... ");
    irq_init();
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write(" [OK]\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	vga_write("[6] CMOS... ");
	rtc_time_t t;
	cmos_read_rtc(&t);
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	vga_write(" [OK]\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	vga_write("[7] TIME... ");
	time_init(1000);
	delay_init(1000);
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	vga_write(" [OK]\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	vga_write("[8] Basic Handlers... ");
    irq_install_handler(0, timer_irq);     // IRQ0 = PIT
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	vga_write(" [OK]\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	vga_write("[9] IRQ Unmasking... ");
    pic_unmask_irq(0);
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	vga_write(" [OK]\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("[10] STI (enable interrupts)... ");
    __asm__ volatile("sti");
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write(" [OK]\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("[11] Keyboard... ");
    keyboard_init();
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write(" [OK]\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("[12] Mouse... ");
    mouse_init();
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write(" [OK]\n\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	vga_write("CPU:");
	vga_write(sysconfig_cpu_str());
	vga_write("\n");
	vga_write(meminfo_str());
	vga_write("\n\n");
	
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("Cinser Kernel OK! ");
    vga_write("(dots = timer, KBD shows scancode)\n\n");
	
	delay_time(5);
	
// ... código de inicialização anterior ...

    desktop_init();

    int need_redraw = 1;
    uint32_t last_frame_time = 0; // Controle de FPS

    for (;;) {
        // HLT economiza CPU e espera a próxima interrupção (Timer ou Teclado)
        __asm__ volatile("hlt");

        // 1. Processa TODAS as teclas pendentes antes de desenhar
        // Isso evita desenhar 10 vezes se 10 teclas chegaram juntas
        if (keyboard_haschar()) {
            while (keyboard_haschar()) {
                int ch = keyboard_getchar();
                if (ch > 0) {
                    desktop_key((char)ch);
                    need_redraw = 1; 
                }
            }
        }
        
        // 2. Desenha (Com Limitador de 60 FPS)
        if (need_redraw) {
            uint32_t now = time_get_ticks();
            
            // 1000ms / 60fps ≈ 16ms por frame
            // Se passou menos de 16ms desde o último desenho, PULA.
            // Isso evita sobrecarregar o barramento de vídeo à toa.
            if ((now - last_frame_time) < 16) {
                continue; 
            }
            
            // Marca o tempo deste frame
            last_frame_time = now;

            // --- INÍCIO DO DESENHO ---
            
            // A. Desenha na RAM (Backbuffer)
            desktop_draw();
            
            // B. Sincronia Fina com o Monitor (VSync)
            wait_vsync();

            // C. Copia para a Tela (VRAM) de forma Atômica
            if (g_video_driver && g_video_driver->update) {
                g_video_driver->update();
            }
            
            need_redraw = 0;
            // -------------------------
        }
    }
}