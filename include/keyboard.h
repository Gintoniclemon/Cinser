/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: keyboard.h
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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);

/* Non-blocking read: returns -1 if empty, else 0-255 */
int keyboard_getchar(void);
int keyboard_haschar(void);
void keyboard_flush(void);

/* State helpers (optional) */
int keyboard_capslock(void);
int keyboard_shift(void);

#endif
