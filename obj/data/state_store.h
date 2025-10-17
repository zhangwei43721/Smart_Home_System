#ifndef STATE_STORE_H
#define STATE_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct {
    int is_on;
    int brightness;
    int color_temp;
} LightPersist;

void ss_lighting_load(LightPersist *arr, int count);
void ss_lighting_save(const LightPersist *arr, int count);

int ss_security_load(void);
void ss_security_save(int state);

#ifdef __cplusplus
}
#endif

#endif
