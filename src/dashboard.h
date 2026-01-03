/**
 * dashboard.h
 * E-bike dashboard UI
 */
#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "lvgl/lvgl.h"
#include <stdbool.h>
#include <stdint.h>

extern const lv_coord_t SCREEN_W;
extern const lv_coord_t SCREEN_H;

void dashboard_create(void);
void dashboard_set_power(int32_t kw);
void dashboard_set_night_mode(bool enable);

#endif /*DASHBOARD_H*/
