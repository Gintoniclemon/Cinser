/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: time.c
 * Descricao: Servicos de tempo (CMOS/RTC + contagem por IRQ0/PIT).
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
#include "time.h"

static volatile uint32_t g_ticks = 0;
static volatile uint32_t g_ticks_per_sec = 18;
static volatile int g_ready = 0;

static volatile rtc_time_t g_time;

// Buffer estatico: "HH:MM:SS DD/MM/YYYY" + '\0' => 20
static char g_time_str[20];
static volatile int g_time_str_dirty = 1;

static inline void put2(char *p, uint8_t v) {
    p[0] = (char)('0' + (v / 10));
    p[1] = (char)('0' + (v % 10));
}

static inline void put4(char *p, uint16_t v) {
    p[0] = (char)('0' + (v / 1000) % 10);
    p[1] = (char)('0' + (v / 100)  % 10);
    p[2] = (char)('0' + (v / 10)   % 10);
    p[3] = (char)('0' + (v % 10));
}

static void rebuild_time_str(void) {
    // "HH:MM:SS DD/MM/YYYY"
    put2(&g_time_str[0],  g_time.hour);
    g_time_str[2] = ':';
    put2(&g_time_str[3],  g_time.min);
    g_time_str[5] = ':';
    put2(&g_time_str[6],  g_time.sec);
    g_time_str[8] = ' ';
    put2(&g_time_str[9],  g_time.day);
    g_time_str[11] = '/';
    put2(&g_time_str[12], g_time.mon);
    g_time_str[14] = '/';
    put4(&g_time_str[15], g_time.year);
    g_time_str[19] = '\0';
}

// Incremento simples (sem calendario completo). Funciona bem para segundos/minutos/horas.
// Se quiser suporte a dia/mes/ano correto com meses e anos bissextos, fazemos um patch depois.
static void tick_one_second(void) {
    g_time.sec++;
    if (g_time.sec >= 60) {
        g_time.sec = 0;
        g_time.min++;
        if (g_time.min >= 60) {
            g_time.min = 0;
            g_time.hour++;
            if (g_time.hour >= 24) {
                g_time.hour = 0;
                // Opcional: dia++ (sem calendario completo)
                // g_time.day++;
            }
        }
    }

    rebuild_time_str();
    g_time_str_dirty = 1;
}

// Deve ser chamado pelo handler do IRQ0 (timer).
// Mantemos o contador interno e avancamos 1 segundo quando
// g_ticks_per_sec ticks forem atingidos.
void time_tick(void) {
    g_ticks++;

    if (!g_ready) return;

    if ((g_ticks_per_sec != 0) && ((g_ticks % g_ticks_per_sec) == 0)) {
        tick_one_second();
    }
}

void time_init(uint32_t pit_ticks_per_sec) {
    if (pit_ticks_per_sec == 0) pit_ticks_per_sec = 18;
    g_ticks_per_sec = pit_ticks_per_sec;

    // Le o RTC uma vez e usa o IRQ0 para manter o tempo andando (Opcao A)
    rtc_time_t t;
    cmos_read_rtc(&t);
    g_time = t;

    rebuild_time_str();
    g_time_str_dirty = 1;
    g_ready = 1;
}

rtc_time_t time_now(void) {
    // copia atomica simples (na pratica suficiente aqui)
    rtc_time_t t = g_time;
    return t;
}

const char *time_datetime_str(void) {
    return g_time_str;
}

int time_has_update(void) {
    return g_time_str_dirty ? 1 : 0;
}

void time_consume_update(void) {
    g_time_str_dirty = 0;
}
