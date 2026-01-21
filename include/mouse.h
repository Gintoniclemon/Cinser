/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: mouse.h
 * Descricao: Driver de mouse PS/2 (IRQ12) - leitura de pacotes e estado.
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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mouse_state {
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;
    uint8_t buttons;   // bit0=L, bit1=R, bit2=M
} mouse_state_t;

// Inicializa o mouse PS/2 (habilita dispositivo auxiliar, instala IRQ12).
// Nao depende de desktop/window. Pode ser chamado quando quiser.
void mouse_init(void);

// Define limites para clamping do cursor (inclusive).
// Se max_x/max_y < min_x/min_y, o clamping e desabilitado.
void mouse_set_bounds(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y);

// Obtém o estado atual (sempre retorna algo; dx/dy sao o delta acumulado
// desde o ultimo mouse_poll()).
void mouse_get_state(mouse_state_t *out);

// Copia o estado e zera dx/dy. Retorna 1 se houve movimento/botao desde
// o ultimo poll, senao 0.
int mouse_poll(mouse_state_t *out);

#ifdef __cplusplus
}
#endif
