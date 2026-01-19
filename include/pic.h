/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: pic.h
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

#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/* IO base addresses for the 8259 PIC */
#define PIC1            0x20
#define PIC2            0xA0

/* Command and data ports */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1 + 1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2 + 1)

/* End of Interrupt */
#define PIC_EOI         0x20

/* IRQ offsets (after remap) */
#define PIC1_OFFSET     0x20
#define PIC2_OFFSET     0x28

/* Public API */
void pic_init(void);
void pic_send_eoi(uint8_t irq);
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

#endif