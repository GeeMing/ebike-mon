/**
 * dashboard.c
 * A simple LVGL dashboard for the e-bike display (480x272)
 */

#include "dashboard.h"
#include <stdio.h>
#include <time.h>

static lv_obj_t * speed_label;
static lv_obj_t * max_label;
static lv_obj_t * left_turn_icon;
static lv_obj_t * right_turn_icon;
static lv_obj_t * brake_light_obj;
static lv_obj_t * headlight_obj;
static lv_obj_t * power_label;
static lv_obj_t * time_label;
static lv_obj_t * duration_label;
static lv_obj_t * battery_bar;
static lv_obj_t * battery_label;
static lv_obj_t * range_label;
static lv_obj_t * gear_label;

/* Simple helper to create a small colored circle used for indicators */
static lv_obj_t * create_circle(lv_obj_t * parent, lv_color_t color, lv_coord_t size)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_set_size(obj, size, size);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    return obj;
}

static void time_timer_cb(lv_timer_t * t)
{
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char buf[32];
    if(tm_now) {
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
        lv_label_set_text(time_label, buf);
    }

    /* ride duration placeholder: increment seconds stored in timer user_data */
    void *ud = lv_timer_get_user_data(t);
    uint32_t *secs = (uint32_t *)ud;
    if(secs) (*secs)++;
    int h = (*secs) / 3600;
    int m = ((*secs) % 3600) / 60;
    int s = (*secs) % 60;
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
    lv_label_set_text(duration_label, buf);
}

void dashboard_create(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_scr_load(scr);

    /* Background style */
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0B1220), 0);

    /* Left: big speed area */
    lv_obj_t * left = lv_obj_create(scr);
    lv_obj_set_size(left, 260, 240);
    lv_obj_align(left, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);

    speed_label = lv_label_create(left);
    lv_label_set_text_fmt(speed_label, "%.0f", 0.0);
    lv_obj_set_style_text_font(speed_label, &lv_font_montserrat_48, 0);
    lv_obj_align(speed_label, LV_ALIGN_LEFT_MID, 40, -10);

    max_label = lv_label_create(left);
    lv_label_set_text(max_label, "Max 80 km/h");
    lv_obj_set_style_text_font(max_label, &lv_font_montserrat_14, 0);
    lv_obj_align(max_label, LV_ALIGN_LEFT_MID, 40, 40);

    /* Right area: status, power, battery, time */
    lv_obj_t * right = lv_obj_create(scr);
    lv_obj_set_size(right, 200, 240);
    lv_obj_align(right, LV_ALIGN_RIGHT_MID, -8, 0);
    lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);

    /* Top row: turn indicators and lights */
    left_turn_icon = create_circle(right, lv_color_hex(0x00A8FF), 18);
    lv_obj_align(left_turn_icon, LV_ALIGN_TOP_LEFT, 8, 8);
    right_turn_icon = create_circle(right, lv_color_hex(0x00A8FF), 18);
    lv_obj_align(right_turn_icon, LV_ALIGN_TOP_RIGHT, -8, 8);

    headlight_obj = create_circle(right, lv_color_hex(0xFFFFFF), 12);
    lv_obj_align(headlight_obj, LV_ALIGN_TOP_MID, -30, 10);
    brake_light_obj = create_circle(right, lv_color_hex(0xFF0000), 12);
    lv_obj_align(brake_light_obj, LV_ALIGN_TOP_MID, 30, 10);

    /* Power */
    power_label = lv_label_create(right);
    lv_label_set_text(power_label, "P: 0 W");
    lv_obj_align(power_label, LV_ALIGN_CENTER, 0, -20);

    /* Battery bar and label */
    battery_label = lv_label_create(right);
    lv_label_set_text(battery_label, "Batt: 100%%");
    lv_obj_align(battery_label, LV_ALIGN_BOTTOM_LEFT, 8, -36);

    battery_bar = lv_bar_create(right);
    lv_obj_set_size(battery_bar, 120, 12);
    lv_bar_set_range(battery_bar, 0, 100);
    lv_bar_set_value(battery_bar, 100, LV_ANIM_OFF);
    lv_obj_align(battery_bar, LV_ALIGN_BOTTOM_MID, -10, -32);

    range_label = lv_label_create(right);
    lv_label_set_text(range_label, "Est Range: 120 km");
    lv_obj_align(range_label, LV_ALIGN_BOTTOM_RIGHT, -8, -36);

    /* Time and duration */
    time_label = lv_label_create(scr);
    lv_label_set_text(time_label, "--:--:--");
    lv_obj_align(time_label, LV_ALIGN_TOP_RIGHT, -8, 6);

    duration_label = lv_label_create(scr);
    lv_label_set_text(duration_label, "00:00:00");
    lv_obj_align(duration_label, LV_ALIGN_TOP_RIGHT, -8, 26);

    /* Gear box */
    gear_label = lv_label_create(scr);
    lv_label_set_text(gear_label, "Gear: N");
    lv_obj_align(gear_label, LV_ALIGN_BOTTOM_LEFT, 12, -8);

    /* Initialize the values (placeholders). In a real system, sensors update these. */
    lv_label_set_text_fmt(speed_label, "%d km/h", 0);
    lv_label_set_text(power_label, "P: 0 W");

    /* Create a periodic timer to update time and duration */
    uint32_t *secs = lv_malloc(sizeof(uint32_t));
    if(secs) *secs = 0;
    lv_timer_t * t = lv_timer_create(time_timer_cb, 1000, NULL);
    if(t) lv_timer_set_user_data(t, secs);
    (void)t;
}
