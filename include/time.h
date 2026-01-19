/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: time.h
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

#pragma once
#include <stdint.h>
#include "cmos.h"

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa o servico de tempo.
// pit_ticks_per_sec: quantos ticks do IRQ0 equivalem a 1 segundo.
// Ex: 18 para PIT padrao (aprox 18.2Hz), 100 para PIT configurado em 100Hz.
void time_init(uint32_t pit_ticks_per_sec);

void time_tick(void);

// Retorna o tempo atual (estrutura em RAM, atualizada pelo timer).
rtc_time_t time_now(void);

// Retorna um ponteiro para uma string estatica formatada:
// "HH:MM:SS DD/MM/YYYY"
// A string so e atualizada quando o segundo muda (reduz trabalho/writes).
const char *time_datetime_str(void);

// Indica se a string formatada foi atualizada desde a ultima chamada
// de time_consume_update().
int time_has_update(void);

// Marca a atualizacao como consumida.
void time_consume_update(void);

#ifdef __cplusplus
}
#endif
