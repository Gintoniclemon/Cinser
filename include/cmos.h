/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: cmos.h
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

#pragma once
#include <stdint.h>

typedef struct {
    uint8_t sec, min, hour;
    uint8_t day, mon;
    uint16_t year; // 4-digit
} rtc_time_t;

void cmos_read_rtc(rtc_time_t *out);
