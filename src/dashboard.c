/**
 * dashboard.c
 * A simple LVGL dashboard for the e-bike display (480x272)
 */

#include "dashboard.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "left.h"
#include "right.h"

/* Global screen size constants */
const lv_coord_t SCREEN_W = 480;
const lv_coord_t SCREEN_H = 272;

static lv_obj_t* speed_label;
static lv_obj_t* max_label;
static lv_obj_t* left_turn_icon;
static lv_obj_t* right_turn_icon;
static lv_obj_t* brake_light_obj;
static lv_obj_t* headlight_obj;
static lv_obj_t* power_label;
static lv_obj_t* time_label;
static lv_obj_t* battery_bar;
static lv_obj_t* battery_label;
static lv_obj_t* range_label;
static lv_obj_t* gear_label;

/* Left panel labels */
static lv_obj_t* left_odo_title;
static lv_obj_t* left_odo_value;
static lv_obj_t* left_trip_title;
static lv_obj_t* left_trip_value;
static lv_obj_t* left_ride_time_title;
static lv_obj_t* left_ride_time_value;
static lv_obj_t* left_max_speed_title;
static lv_obj_t* left_max_speed_value;
static lv_obj_t* left_used_title;
static lv_obj_t* left_used_value;

/* Right panel labels */
static lv_obj_t* right_batt_title;
static lv_obj_t* right_batt_cap_value;
static lv_obj_t* right_range_title;
static lv_obj_t* right_est_range_value;
static lv_obj_t* right_maxp_title;
static lv_obj_t* right_maxp_value;
static lv_obj_t* right_hist_avg_title;
static lv_obj_t* right_hist_avg_value;
static lv_obj_t* right_1min_avg_title;
static lv_obj_t* right_1min_avg_value;

/* Energy bar (bottom center) */
static lv_obj_t*     energy_bar_cont;
static lv_obj_t*     energy_bar_label;
static int32_t       energy_power_w;
static lv_obj_t*     energy_bar_left;
static lv_obj_t*     energy_bar_right;
static const int32_t ENERGY_MIN_W = -2000;
static const int32_t ENERGY_MAX_W = 6000;
static const int32_t ENERGY_BAR_W = 200;
static const int32_t ENERGY_BAR_H = 50;

/* Simple helper to create a small colored circle used for indicators */
static lv_obj_t* create_circle(lv_obj_t* parent, lv_color_t color, lv_coord_t size) {
    lv_obj_t* obj = lv_obj_create(parent);
    lv_obj_set_size(obj, size, size);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    return obj;
}

static void time_timer_cb(lv_timer_t* t) {
    time_t     now    = time(NULL);
    struct tm* tm_now = localtime(&now);
    char       buf[32];
    if (tm_now) {
        snprintf(buf, sizeof(buf), "%02d:%02d", tm_now->tm_hour, tm_now->tm_min);
        lv_label_set_text(time_label, buf);
    }
}

/* Update energy bar label (power in w). The displayed text is NOT clamped; only the fill size is constrained to the bar range. */
void dashboard_set_power(int w) {
    if (!energy_bar_cont)
        return;

    /* Keep raw value for label/display */
    energy_power_w = w;

    /* Format and show the raw value (can be outside min/max).
       Use explicit sign and absolute parts so values between -1000 and 0 show "-0.xxx" correctly. */
    char buf[32];
    int  v     = energy_power_w;
    int  abs_v = v < 0 ? -v : v;
    int  whole = (int)(abs_v / 1000);
    int  frac  = (int)(abs_v % 1000);
    snprintf(buf, sizeof(buf), "%c%d.%03d kW", v < 0 ? '-' : '+', whole, frac);
    lv_label_set_text(energy_bar_label, buf);

    int       left_value  = 0;
    int       right_value = 0;
    const int zero_line_x = ENERGY_BAR_W * (-ENERGY_MIN_W) / (ENERGY_MAX_W + (-ENERGY_MIN_W));
    const int left_size   = zero_line_x;

    if (energy_power_w > 0) {
        /* Right fill */
        lv_coord_t avail_right = ENERGY_BAR_W - zero_line_x;
        right_value            = (lv_coord_t)((int64_t)energy_power_w * avail_right / ENERGY_MAX_W);
        if (right_value < 0)
            right_value = 0;
        if (right_value > avail_right)
            right_value = avail_right;
    } else {
        /* Left fill */
        lv_coord_t avail_left = zero_line_x;
        left_value            = (lv_coord_t)((int64_t)(-energy_power_w) * avail_left / (-ENERGY_MIN_W));
        if (left_value < 0)
            left_value = 0;
        if (left_value > avail_left)
            left_value = avail_left;
    }

    lv_obj_set_size(energy_bar_left, left_value, ENERGY_BAR_H - 8);
    lv_obj_align(energy_bar_left, LV_ALIGN_LEFT_MID, left_size - left_value, 0);
    lv_obj_set_size(energy_bar_right, right_value, ENERGY_BAR_H - 8);
}

/* Animation timer: sweep power from -3000..9000 */
static void energy_anim_timer_cb(lv_timer_t* t) {
    static int32_t v     = -3000;
    static int     dir   = 1; /* 1: up, -1: down */
    const int32_t  min_v = -3000;
    const int32_t  max_v = 9000;
    const int32_t  step  = 100; /* change per tick (w) */

    v += dir * step;
    if (v >= max_v) {
        v   = max_v;
        dir = -1;
    } else if (v <= min_v) {
        v   = min_v;
        dir = 1;
    }

    /* Update the display with the swept value */
    dashboard_set_power(v);
}

/* Draw top-left and top-right icons */
static void draw_icons(lv_obj_t* scr) {
    // int offset = 8;
    int offset = 2;

    lv_obj_t* img_left = lv_img_create(scr);
    lv_img_set_src(img_left, &left_png);
    lv_obj_align(img_left, LV_ALIGN_TOP_LEFT, offset, offset);

    lv_obj_t* img_right = lv_img_create(scr);
    lv_img_set_src(img_right, &right_png);
    lv_obj_align(img_right, LV_ALIGN_TOP_RIGHT, -offset, offset);

    lv_obj_move_foreground(img_left);
    lv_obj_move_foreground(img_right);
}

/* Draw vertical separators at left_x and right_x */
static void draw_separators(lv_obj_t* scr) {
    /* Compute layout and draw separators */
    int              line_size = 2;
    int              padding   = 2; // 留给能量条 的padding
    const lv_coord_t mid_w     = ENERGY_BAR_W;
    const lv_coord_t left_x    = (SCREEN_W - mid_w - line_size) / 2 - padding;
    const lv_coord_t right_x   = left_x + mid_w + padding * 2;

    lv_obj_t* line1 = lv_obj_create(scr);
    lv_obj_set_size(line1, line_size, SCREEN_H);
    lv_obj_set_style_bg_color(line1, lv_color_hex(0x888888), 0);
    lv_obj_set_style_border_width(line1, 0, 0);
    lv_obj_set_pos(line1, left_x, 0);

    lv_obj_t* line2 = lv_obj_create(scr);
    lv_obj_set_size(line2, line_size, SCREEN_H);
    lv_obj_set_style_bg_color(line2, lv_color_hex(0x888888), 0);
    lv_obj_set_style_border_width(line2, 0, 0);
    lv_obj_set_pos(line2, right_x, 0);

    const lv_coord_t top_h    = 40;
    lv_obj_t*        top_line = lv_obj_create(scr);
    lv_obj_set_size(top_line, SCREEN_W, line_size);
    lv_obj_set_style_bg_color(top_line, lv_color_hex(0x888888), 0);
    lv_obj_set_style_border_width(top_line, 0, 0);
    lv_obj_set_pos(top_line, 0, top_h);
}

/* Draw energy bar at bottom center (only border and centered label) */
static void draw_energy_bar(lv_obj_t* scr) {
    // -2/6,  8分度，25% 回收分区 占比
    const lv_coord_t left_x      = (SCREEN_W - ENERGY_BAR_W) / 2;
    const int        zero_line_x = ENERGY_BAR_W * (-ENERGY_MIN_W) / (ENERGY_MAX_W + (-ENERGY_MIN_W));
    const int        left_size   = zero_line_x;
    const int        right_size  = ENERGY_BAR_W - left_size;

    /* Container: transparent background with black border only */
    energy_bar_cont = lv_obj_create(scr);
    lv_obj_set_size(energy_bar_cont, ENERGY_BAR_W, ENERGY_BAR_H);
    lv_obj_set_style_bg_opa(energy_bar_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(energy_bar_cont, 4, 0);
    lv_obj_set_style_border_color(energy_bar_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(energy_bar_cont, 8, 0);
    lv_obj_set_pos(energy_bar_cont, left_x, SCREEN_H - ENERGY_BAR_H - 5);
    lv_obj_clear_flag(energy_bar_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(energy_bar_cont, 0, 0);
    lv_obj_set_style_clip_corner(energy_bar_cont, true, 0);

    int left_value  = 0;
    int right_value = 0;

    energy_bar_left = lv_obj_create(energy_bar_cont);
    lv_obj_set_size(energy_bar_left, left_value, ENERGY_BAR_H - 8);
    lv_obj_align(energy_bar_left, LV_ALIGN_LEFT_MID, left_size - left_value, 0);
    lv_obj_set_style_bg_color(energy_bar_left, lv_color_hex(0x67c23a), 0); // 绿色回收
    lv_obj_set_style_radius(energy_bar_left, 0, 0);
    lv_obj_set_style_border_width(energy_bar_left, 0, 0);
    lv_obj_set_style_bg_opa(energy_bar_left, LV_OPA_COVER, 0);
    lv_obj_clear_flag(energy_bar_left, LV_OBJ_FLAG_SCROLLABLE);

    energy_bar_right = lv_obj_create(energy_bar_cont);
    lv_obj_set_size(energy_bar_right, right_value, ENERGY_BAR_H - 8);
    lv_obj_align(energy_bar_right, LV_ALIGN_LEFT_MID, left_size, 0);
    lv_obj_set_style_bg_color(energy_bar_right, lv_color_hex(0xf56c6c), 0); // 红色能量
    lv_obj_set_style_radius(energy_bar_right, 0, 0);
    lv_obj_set_style_border_width(energy_bar_right, 0, 0);
    lv_obj_set_style_bg_opa(energy_bar_right, LV_OPA_COVER, 0);
    lv_obj_clear_flag(energy_bar_right, LV_OBJ_FLAG_SCROLLABLE);

    // 0刻度地方 画一条竖线
    lv_obj_t* zero_line = lv_obj_create(energy_bar_cont);
    lv_obj_set_size(zero_line, 5, ENERGY_BAR_H);
    lv_obj_set_style_bg_color(zero_line, lv_color_hex(0xAAAAAA), 0); // gray color
    lv_obj_set_style_border_width(zero_line, 0, 0);
    lv_obj_align(zero_line, LV_ALIGN_LEFT_MID, zero_line_x, 0);

    energy_bar_label = lv_label_create(energy_bar_cont);
    lv_obj_align(energy_bar_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(energy_bar_label, lv_color_hex(0x000000), 0);
    /* Make the label larger and keep it above the fill rectangles */
    lv_obj_set_style_text_font(energy_bar_label, &lv_font_montserrat_30, 0);
    lv_obj_clear_flag(energy_bar_label, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(energy_bar_label);

    // /* Initialize bar to zero power */
    dashboard_set_power(0);

    // /* Start animation timer to sweep -3000..9000 for demo */
    lv_timer_create(energy_anim_timer_cb, 100, NULL);
}

static void draw_current_time(lv_obj_t* scr) {
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

/* Small helper to create a title/value pair at (x,y) */
static void create_kv_pair(lv_obj_t* parent, lv_obj_t** title_out, lv_obj_t** value_out, const char* title, const char* value, lv_coord_t x, lv_coord_t y) {
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
static void draw_side_panels(lv_obj_t* scr) {
    const lv_coord_t left_x  = (SCREEN_W - ENERGY_BAR_W) / 2;
    const lv_coord_t right_x = left_x + ENERGY_BAR_W;
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

void dashboard_create(void) {
    /* Create a clean screen */
    lv_obj_t* scr = lv_obj_create(NULL);
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
