#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void desktop_init(void);
void desktop_draw(void);
void desktop_key(char c);

#ifdef __cplusplus
}
#endif
