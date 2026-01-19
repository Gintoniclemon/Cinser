/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: cmos.c
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
#include "io.h"
#include "cmos.h"

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static inline uint8_t cmos_read(uint8_t reg) {
    // keep NMI enabled (bit7=0). If you want to disable NMI: reg | 0x80
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

static inline int cmos_uip(void) {
    return (cmos_read(0x0A) & 0x80) != 0;
}

static inline uint8_t bcd_to_bin(uint8_t bcd) {
    return (uint8_t)((bcd & 0x0F) + ((bcd >> 4) * 10));
}

void cmos_read_rtc(rtc_time_t *out) {
    // wait update cycle end
    while (cmos_uip()) { }

    uint8_t sec  = cmos_read(0x00);
    uint8_t min  = cmos_read(0x02);
    uint8_t hour = cmos_read(0x04);
    uint8_t day  = cmos_read(0x07);
    uint8_t mon  = cmos_read(0x08);
    uint8_t year = cmos_read(0x09);

    // read format flags
    uint8_t regB = cmos_read(0x0B);

    int is_bcd = ((regB & 0x04) == 0);
    int is_24h = ((regB & 0x02) != 0);

    if (is_bcd) {
        sec  = bcd_to_bin(sec);
        min  = bcd_to_bin(min);
        // hour needs special handling if 12h + PM bit
        if (!is_24h) {
            uint8_t pm = hour & 0x80;
            hour = bcd_to_bin(hour & 0x7F);
            if (pm && hour < 12) hour = (uint8_t)(hour + 12);
            if (!pm && hour == 12) hour = 0;
        } else {
            hour = bcd_to_bin(hour);
        }
        day  = bcd_to_bin(day);
        mon  = bcd_to_bin(mon);
        year = bcd_to_bin(year);
    } else {
        if (!is_24h) {
            uint8_t pm = hour & 0x80;
            hour = (uint8_t)(hour & 0x7F);
            if (pm && hour < 12) hour = (uint8_t)(hour + 12);
            if (!pm && hour == 12) hour = 0;
        }
    }

    // Simple century handling: assume 2000-2099
    out->sec = sec;
    out->min = min;
    out->hour = hour;
    out->day = day;
    out->mon = mon;
    out->year = (uint16_t)(2000 + year);
}
