/**
 * dashboard.h
 * E-bike dashboard UI
 */
#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>


extern const lv_coord_t SCREEN_W;
extern const lv_coord_t SCREEN_H;

void dashboard_create(lv_obj_t *scr);
void dashboard_set_power(int kw);
void dashboard_set_night_mode(bool enable);
void dashboard_set_gear(int gear);

#endif /*DASHBOARD_H*/
