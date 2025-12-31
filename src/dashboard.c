/**
 * dashboard.c
 * A simple LVGL dashboard for the e-bike display (480x272)
 */

#include "dashboard.h"
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "left.h"
#include "right.h"

/* Global screen size constants */
const lv_coord_t SCREEN_W = 480;
const lv_coord_t SCREEN_H = 272;

static lv_obj_t *speed_label;
static lv_obj_t *max_label;
static lv_obj_t *left_turn_icon;
static lv_obj_t *right_turn_icon;
static lv_obj_t *brake_light_obj;
static lv_obj_t *headlight_obj;
static lv_obj_t *power_label;
static lv_obj_t *time_label;
static lv_obj_t *battery_bar;
static lv_obj_t *battery_label;
static lv_obj_t *range_label;
static lv_obj_t *gear_label;

/* Left panel labels */
static lv_obj_t *left_odo_title;
static lv_obj_t *left_odo_value;
static lv_obj_t *left_trip_title;
static lv_obj_t *left_trip_value;
static lv_obj_t *left_ride_time_title;
static lv_obj_t *left_ride_time_value;
static lv_obj_t *left_max_speed_title;
static lv_obj_t *left_max_speed_value;
static lv_obj_t *left_used_title;
static lv_obj_t *left_used_value;

/* Right panel labels */
static lv_obj_t *right_batt_title;
static lv_obj_t *right_batt_cap_value;
static lv_obj_t *right_range_title;
static lv_obj_t *right_est_range_value;
static lv_obj_t *right_maxp_title;
static lv_obj_t *right_maxp_value;
static lv_obj_t *right_hist_avg_title;
static lv_obj_t *right_hist_avg_value;
static lv_obj_t *right_1min_avg_title;
static lv_obj_t *right_1min_avg_value;

/* Energy bar (bottom center) */
static lv_obj_t *energy_bar_cont;
static lv_obj_t *energy_bar_left;
static lv_obj_t *energy_bar_right;
static lv_obj_t *energy_bar_label;
static int32_t energy_power_w;
static const int32_t ENERGY_MIN_W = -2000;
static const int32_t ENERGY_MAX_W = 6000;

/* Simple helper to create a small colored circle used for indicators */
static lv_obj_t *create_circle(lv_obj_t *parent, lv_color_t color, lv_coord_t size)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_size(obj, size, size);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    return obj;
}

static void time_timer_cb(lv_timer_t *t)
{
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char buf[32];
    if (tm_now)
    {
        snprintf(buf, sizeof(buf), "%02d:%02d", tm_now->tm_hour, tm_now->tm_min);
        lv_label_set_text(time_label, buf);
    }
}

/* Update energy bar fill and label (power in kW, range -2000..6000) */
void dashboard_set_power(int32_t kw)
{
    if (!energy_bar_cont)
        return;
    energy_power_w = kw;
    if (energy_power_w < ENERGY_MIN_W)
        energy_power_w = ENERGY_MIN_W;
    if (energy_power_w > ENERGY_MAX_W)
        energy_power_w = ENERGY_MAX_W;

    int bar_w = lv_obj_get_width(energy_bar_cont);
    int half = bar_w / 2;
    int left_w = 0, right_w = 0;
    char buf[32];

    if (energy_power_w >= 0)
    {
        float frac = (float)energy_power_w / (float)ENERGY_MAX_W;
        if (frac > 1.0f)
            frac = 1.0f;
        right_w = (int)(frac * half);
    }
    else
    {
        float frac = (float)(-energy_power_w) / (float)(-ENERGY_MIN_W);
        if (frac > 1.0f)
            frac = 1.0f;
        left_w = (int)(frac * half);
    }

    lv_obj_set_size(energy_bar_left, left_w, lv_obj_get_height(energy_bar_cont));
    lv_obj_set_pos(energy_bar_left, half - left_w, 0);
    lv_obj_set_size(energy_bar_right, right_w, lv_obj_get_height(energy_bar_cont));
    lv_obj_set_pos(energy_bar_right, half, 0);

    snprintf(buf, sizeof(buf), "%+d kW", (int)energy_power_w);
    lv_label_set_text(energy_bar_label, buf);
}

/* Draw top-left and top-right icons */
static void draw_icons(lv_obj_t *scr)
{
    // int offset = 8;
    int offset = 2;

    lv_obj_t *img_left = lv_img_create(scr);
    lv_img_set_src(img_left, &left_png);
    lv_obj_align(img_left, LV_ALIGN_TOP_LEFT, offset, offset);

    lv_obj_t *img_right = lv_img_create(scr);
    lv_img_set_src(img_right, &right_png);
    lv_obj_align(img_right, LV_ALIGN_TOP_RIGHT, -offset, offset);

    lv_obj_move_foreground(img_left);
    lv_obj_move_foreground(img_right);
}

/* Draw vertical separators at left_x and right_x */
static void draw_separators(lv_obj_t *scr)
{
    /* Compute layout and draw separators */
    const lv_coord_t mid_w = 200;
    const lv_coord_t left_x = (SCREEN_W - mid_w) / 2; /* 140 */
    const lv_coord_t right_x = left_x + mid_w;        /* 340 */

    lv_obj_t *line1 = lv_obj_create(scr);
    lv_obj_set_size(line1, 2, SCREEN_H);
    lv_obj_set_style_bg_color(line1, lv_color_hex(0x888888), 0);
    lv_obj_set_style_border_width(line1, 0, 0);
    lv_obj_set_pos(line1, left_x, 0);

    lv_obj_t *line2 = lv_obj_create(scr);
    lv_obj_set_size(line2, 2, SCREEN_H);
    lv_obj_set_style_bg_color(line2, lv_color_hex(0x888888), 0);
    lv_obj_set_style_border_width(line2, 0, 0);
    lv_obj_set_pos(line2, right_x, 0);

    const lv_coord_t top_h = 40;
    lv_obj_t *top_line = lv_obj_create(scr);
    lv_obj_set_size(top_line, SCREEN_W, 2);
    lv_obj_set_style_bg_color(top_line, lv_color_hex(0x888888), 0);
    lv_obj_set_style_border_width(top_line, 0, 0);
    lv_obj_set_pos(top_line, 0, top_h);
}

static void draw_current_time(lv_obj_t *scr)
{
    time_label = lv_label_create(scr);
    lv_label_set_text(time_label, "00:00");
    lv_obj_clear_flag(time_label, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0x000000), 0);
    /* larger clock font */
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_36, 0);
    // lv_obj_set_scrollbar_mode(time_label, LV_SCROLLBAR_MODE_OFF);

    lv_timer_create(time_timer_cb, 1000, NULL);
}

/* Draw energy bar at bottom center */
static void draw_energy_bar(lv_obj_t *scr)
{
    const lv_coord_t energy_h = 50;
    const lv_coord_t mid_w = 200;
    const lv_coord_t left_x = (SCREEN_W - mid_w) / 2;

    energy_bar_cont = lv_obj_create(scr);
    lv_obj_set_size(energy_bar_cont, mid_w, energy_h);
    lv_obj_set_style_bg_color(energy_bar_cont, lv_color_hex(0xDDDDDD), 0);
    lv_obj_set_style_border_width(energy_bar_cont, 0, 0);
    lv_obj_set_style_radius(energy_bar_cont, 6, 0);
    lv_obj_set_pos(energy_bar_cont, left_x, SCREEN_H - energy_h);

    energy_bar_left = lv_obj_create(energy_bar_cont);
    lv_obj_set_size(energy_bar_left, 0, energy_h);
    lv_obj_set_style_bg_color(energy_bar_left, lv_color_hex(0xFF4444), 0);
    lv_obj_set_style_border_width(energy_bar_left, 0, 0);

    energy_bar_right = lv_obj_create(energy_bar_cont);
    lv_obj_set_size(energy_bar_right, 0, energy_h);
    lv_obj_set_style_bg_color(energy_bar_right, lv_color_hex(0x44CC44), 0);
    lv_obj_set_style_border_width(energy_bar_right, 0, 0);

    energy_bar_label = lv_label_create(energy_bar_cont);
    lv_obj_align(energy_bar_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(energy_bar_label, lv_color_hex(0x000000), 0);

    energy_power_w = 0;
    dashboard_set_power(0);
}

/* Small helper to create a title/value pair at (x,y) */
static void create_kv_pair(lv_obj_t *parent, lv_obj_t **title_out, lv_obj_t **value_out, const char *title, const char *value, lv_coord_t x, lv_coord_t y)
{
    int title_value_spacing = 12;

    *title_out = lv_label_create(parent);
    lv_label_set_text(*title_out, title);
    lv_obj_set_style_text_color(*title_out, lv_color_hex(0x666666), 0);
    /* slightly smaller title font */
    lv_obj_set_style_text_font(*title_out, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(*title_out, x, y);

    *value_out = lv_label_create(parent);
    lv_label_set_text(*value_out, value);
    lv_obj_set_style_text_color(*value_out, lv_color_hex(0x000000), 0);
    /* larger value font */
    lv_obj_set_style_text_font(*value_out, &lv_font_montserrat_26, 0);
    /* smaller gap between title and value */
    lv_obj_set_pos(*value_out, x, y + title_value_spacing);
}

/* Draw left and right side panels with requested info */
static void draw_side_panels(lv_obj_t *scr)
{
    const lv_coord_t mid_w = 200;
    const lv_coord_t left_x = (SCREEN_W - mid_w) / 2; /* 140 */
    const lv_coord_t right_x = left_x + mid_w;       /* 340 */
    /* move panels slightly up to make room for increased pair spacing */
    const lv_coord_t top_h = 44;
    /* slightly increase spacing between kv pairs */
    const lv_coord_t spacing = 46;

    /* Left side (x=8) */
    lv_coord_t lx = 8;
    create_kv_pair(scr, &left_odo_title, &left_odo_value, "ODO km", "00000.0", lx, top_h + spacing * 0);
    create_kv_pair(scr, &left_trip_title, &left_trip_value, "TRIP km", "000.0", lx, top_h + spacing * 1);
    create_kv_pair(scr, &left_ride_time_title, &left_ride_time_value, "RIDE TIME", "00:00:00", lx, top_h + spacing * 2);
    create_kv_pair(scr, &left_max_speed_title, &left_max_speed_value, "MAX SPD km/h", "42", lx, top_h + spacing * 3);
    create_kv_pair(scr, &left_used_title, &left_used_value, "USED kWh", "40.0", lx, top_h + spacing * 4);

    /* Right side (aligned to right column) */
    lv_coord_t rx = right_x + 8;
    create_kv_pair(scr, &right_range_title, &right_est_range_value, "RANGE km", "100", rx, top_h + spacing * 0); // 预估剩余续航
    create_kv_pair(scr, &right_hist_avg_title, &right_hist_avg_value, "AVG Wh/km", "12.0", rx, top_h + spacing * 1);
    create_kv_pair(scr, &right_1min_avg_title, &right_1min_avg_value, "TRIP Wh/km", "34.0", rx, top_h + spacing * 2);
    create_kv_pair(scr, &right_maxp_title, &right_maxp_value, "PEAK kW", "4.321", rx, top_h + spacing * 3);
    create_kv_pair(scr, &right_batt_title, &right_batt_cap_value, "BATT CAP kWh", "42.0", rx, top_h + spacing * 4); // 电池容量
}

void dashboard_create(void)
{
    /* Create a clean screen */
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_scr_load(scr);

    /* Disable scrollbars on the main screen */
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    /* Background style */
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    draw_separators(scr);
    draw_current_time(scr);
    draw_icons(scr);
    draw_side_panels(scr);
    draw_energy_bar(scr);

}
