#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Window Window;

Window* window_make(const char* title, int x, int y, int w, int h);
void    window_close(Window* win);
void    window_focus(Window* win);          // NULL => cycle focus
void    window_draw_all(void);              // draw all in z-order
void    window_key(Window* win, char c);    // sends to window (focused by desktop)
void    window_write(Window* win, const char* text);

#ifdef __cplusplus
}
#endif
