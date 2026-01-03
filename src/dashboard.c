/**
 * dashboard.c
 * A simple LVGL dashboard for the e-bike display (480x272)
 */

#include "dashboard.h"
#include "../img/icons.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

/* Declare custom font */
// LV_FONT_DECLARE(lv_font_jetbrains_mono_30)
LV_FONT_DECLARE(lv_font_pf_din_mono_30)
LV_FONT_DECLARE(lv_font_jetbrains_mono_26)
LV_FONT_DECLARE(lv_font_jetbrains_mono_36)
LV_FONT_DECLARE(lv_font_jetbrains_mono_extra_bold_42)

/* Global screen size constants */
const lv_coord_t SCREEN_W = 480;
const lv_coord_t SCREEN_H = 272;

static lv_obj_t *left_turn_icon;
static lv_obj_t *right_turn_icon;
static lv_obj_t *high_beam_icon;
static lv_obj_t *time_label;

/* Theme state */
static bool dashboard_night_mode;
static lv_color_t theme_bg;
static lv_color_t theme_text_main;
static lv_color_t theme_text_dim;
static lv_color_t theme_line;
static lv_color_t theme_border;
static lv_color_t theme_text_alert;
static lv_color_t theme_needle;
static lv_color_t theme_energy_regen;   /* Energy bar: regeneration (green) */
static lv_color_t theme_energy_consume; /* Energy bar: consumption (red) */

static lv_obj_t *scr_root;
static lv_obj_t *sep_line_left;
static lv_obj_t *sep_line_right;
static lv_obj_t *sep_line_top;
static lv_obj_t *zero_line;
static lv_obj_t *meter_center_circle;

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
static lv_obj_t *right_batt_cap_title;
static lv_obj_t *right_batt_cap_value;
static lv_obj_t *right_range_title;
static lv_obj_t *right_range_value;
static lv_obj_t *right_maxp_title;
static lv_obj_t *right_maxp_value;
static lv_obj_t *right_hist_avg_title;
static lv_obj_t *right_hist_avg_value;
static lv_obj_t *right_trip_avg_title;
static lv_obj_t *right_trip_avg_value;

/* Meter widget references for animation */
static lv_obj_t *meter_widget;
static lv_obj_t *meter_needle_line;
static lv_obj_t *meter_center_label;
static lv_obj_t *meter_unit_label;

/* Meter section styles (must be static/global for LVGL) */
static lv_style_t meter_blue_style;
static lv_style_t meter_red_style;

/* Energy bar (bottom center) */
static lv_obj_t *energy_bar_cont;
static lv_obj_t *energy_bar_label;
static int32_t energy_power_w;
static lv_obj_t *energy_bar_left;
static lv_obj_t *energy_bar_right;
static const int32_t ENERGY_MIN_W = -2000;
static const int32_t ENERGY_MAX_W = 6000;
static const int32_t ENERGY_BAR_W = 200;
static const int32_t ENERGY_BAR_H = 50;

/* Set theme color variables based on night mode */
static void dashboard_init_theme_colors(void) {
  if (dashboard_night_mode) {
    theme_bg = lv_color_hex(0x111418);
    theme_text_main = lv_color_hex(0xE6E6E6);
    theme_text_dim = lv_color_hex(0x9AA0A6);
    theme_line = lv_color_hex(0x3A3F45);
    theme_border = lv_color_hex(0xC8C8C8);
    theme_needle = lv_color_hex(0xB0B0B0);
  } else {
    theme_bg = lv_color_hex(0xFFFFFF);
    theme_text_main = lv_color_hex(0x000000);
    theme_text_dim = lv_color_hex(0x666666);
    theme_line = lv_color_hex(0x888888);
    theme_border = lv_color_hex(0x000000);
    theme_needle = lv_palette_main(LV_PALETTE_GREY);
  }
  /* These colors are the same for both themes */
  theme_text_alert = lv_palette_main(LV_PALETTE_RED);
  theme_energy_regen = lv_color_hex(0x67C23A);
  theme_energy_consume = lv_color_hex(0xF56C6C);
}

/* Apply theme colors to all UI elements */
static void dashboard_apply_theme(void) {
  dashboard_init_theme_colors();

  /* Screen background */
  lv_obj_set_style_bg_color(scr_root, theme_bg, 0);

  /* Separator lines */
  lv_obj_set_style_bg_color(sep_line_left, theme_line, 0);
  lv_obj_set_style_bg_color(sep_line_right, theme_line, 0);
  lv_obj_set_style_bg_color(sep_line_top, theme_line, 0);

  /* Time label */
  lv_obj_set_style_text_color(time_label, theme_text_main, 0);

  /* Left panel */
  lv_obj_set_style_text_color(left_odo_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(left_trip_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(left_ride_time_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(left_max_speed_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(left_used_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(left_odo_value, theme_text_main, 0);
  lv_obj_set_style_text_color(left_trip_value, theme_text_main, 0);
  lv_obj_set_style_text_color(left_ride_time_value, theme_text_main, 0);
  lv_obj_set_style_text_color(left_max_speed_value, theme_text_main, 0);
  lv_obj_set_style_text_color(left_used_value, theme_text_main, 0);

  /* Right panel */
  lv_obj_set_style_text_color(right_batt_cap_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(right_range_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(right_maxp_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(right_hist_avg_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(right_trip_avg_title, theme_text_dim, 0);
  lv_obj_set_style_text_color(right_batt_cap_value, theme_text_main, 0);
  lv_obj_set_style_text_color(right_range_value, theme_text_main, 0);
  lv_obj_set_style_text_color(right_maxp_value, theme_text_main, 0);
  lv_obj_set_style_text_color(right_hist_avg_value, theme_text_main, 0);
  lv_obj_set_style_text_color(right_trip_avg_value, theme_text_main, 0);

  /* Energy bar */
  lv_obj_set_style_border_color(energy_bar_cont, theme_border, 0);
  lv_obj_set_style_text_color(energy_bar_label, theme_text_main, 0);
  lv_obj_set_style_bg_color(zero_line, theme_line, 0);

  /* Meter */
  lv_obj_set_style_bg_color(meter_center_circle, theme_bg, 0);
  lv_obj_set_style_text_color(meter_center_label, theme_text_main, 0);
  lv_obj_set_style_text_color(meter_unit_label, theme_text_dim, 0);
  lv_obj_set_style_arc_color(meter_widget, theme_line, LV_PART_MAIN);
  lv_obj_set_style_line_color(meter_widget, theme_line, LV_PART_ITEMS);
  lv_obj_set_style_line_color(meter_widget, theme_line, LV_PART_INDICATOR);
  lv_obj_set_style_text_color(meter_widget, theme_text_dim, LV_PART_MAIN);
  lv_obj_set_style_line_color(meter_needle_line, theme_needle, LV_PART_MAIN);
}

void dashboard_set_night_mode(bool enable) {
  dashboard_night_mode = enable;
  dashboard_apply_theme();
}

/* Simple helper to create a small colored circle used for indicators */
static lv_obj_t *create_circle(lv_obj_t *parent, lv_color_t color,
                               lv_coord_t size) {
  lv_obj_t *obj = lv_obj_create(parent);
  lv_obj_set_size(obj, size, size);
  lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(obj, color, 0);
  lv_obj_set_style_border_width(obj, 0, 0);
  return obj;
}

static void time_timer_cb(lv_timer_t *t) {
  time_t now = time(NULL);
  struct tm *tm_now = localtime(&now);
  char buf[32];
  static bool toggle = false;
  toggle = !toggle;
  if (tm_now) {
    if (toggle) {
      snprintf(buf, sizeof(buf), "%02d:%02d", tm_now->tm_hour, tm_now->tm_min);
    } else {
      snprintf(buf, sizeof(buf), "%02d %02d", tm_now->tm_hour, tm_now->tm_min);
    }
    lv_label_set_text(time_label, buf);
  }
}

/* Update energy bar label (power in w). The displayed text is NOT clamped; only
 * the fill size is constrained to the bar range. */
void dashboard_set_power(int w) {
  if (!energy_bar_cont)
    return;

  /* Keep raw value for label/display */
  energy_power_w = w;

  /* Format and show the raw value (can be outside min/max).
     Use explicit sign and absolute parts so values between -1000 and 0 show
     "-0.xxx" correctly. */
  char buf[32];
  int v = energy_power_w;
  int abs_v = v < 0 ? -v : v;
  int whole = (int)(abs_v / 1000);
  int frac = (int)(abs_v % 1000);
  snprintf(buf, sizeof(buf), "%c%d.%03d kW", v < 0 ? '-' : ' ', whole, frac);
  lv_label_set_text(energy_bar_label, buf);

  int left_value = 0;
  int right_value = 0;
  const int zero_line_x =
      ENERGY_BAR_W * (-ENERGY_MIN_W) / (ENERGY_MAX_W + (-ENERGY_MIN_W));
  const int left_size = zero_line_x;

  if (energy_power_w > 0) {
    /* Right fill */
    lv_coord_t avail_right = ENERGY_BAR_W - zero_line_x;
    right_value =
        (lv_coord_t)((int64_t)energy_power_w * avail_right / ENERGY_MAX_W);
    if (right_value < 0)
      right_value = 0;
    if (right_value > avail_right)
      right_value = avail_right;
  } else {
    /* Left fill */
    lv_coord_t avail_left = zero_line_x;
    left_value =
        (lv_coord_t)((int64_t)(-energy_power_w) * avail_left / (-ENERGY_MIN_W));
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
static void energy_anim_timer_cb(lv_timer_t *t) {
  static int32_t v = -3000;
  static int dir = 1; /* 1: up, -1: down */
  const int32_t min_v = -3000;
  const int32_t max_v = 9000;
  const int32_t step = 100; /* change per tick (w) */

  v += dir * step;
  if (v >= max_v) {
    v = max_v;
    dir = -1;
  } else if (v <= min_v) {
    v = min_v;
    dir = 1;
  }

  /* Update the display with the swept value */
  dashboard_set_power(v);
}

/* Animation timer: sweep meter needle from 0 to 80 and back */
static void meter_anim_timer_cb(lv_timer_t *t) {
  static int32_t v = 0;
  static int dir = 1; /* 1: up, -1: down */
  const int32_t min_v = 0;
  const int32_t max_v = 80;
  const int32_t step = 1; /* change per tick */

  if (!meter_widget || !meter_needle_line) {
    return;
  }

  v += dir * step;
  if (v >= max_v) {
    v = max_v;
    dir = -1;
  } else if (v <= min_v) {
    v = min_v;
    dir = 1;
  }

  /* Update the needle value */
  lv_scale_set_line_needle_value(meter_widget, meter_needle_line, -10, v);

  /* Update center label with current value */
  if (meter_center_label) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", v);
    lv_label_set_text(meter_center_label, buf);

    /* Change color to red if speed exceeds 60 */
    if (v > 60) {
      lv_obj_set_style_text_color(meter_center_label, theme_text_alert, 0);
    } else {
      lv_obj_set_style_text_color(meter_center_label, theme_text_main, 0);
    }
  }
}

static void update_icons_timer_cb(lv_timer_t *t) {
  static bool toggle = false;
  static uint32_t state = 0;
  toggle = !toggle;
  state++;

  if (state > 20) {
    state = 0;
  }

  lv_obj_set_style_img_opa(left_turn_icon, LV_OPA_TRANSP, 0);
  lv_obj_set_style_img_opa(right_turn_icon, LV_OPA_TRANSP, 0);

  if (state < 10) {
    if (toggle) {
      lv_obj_set_style_img_opa(left_turn_icon, LV_OPA_TRANSP, 0);
    } else {
      lv_obj_set_style_img_opa(left_turn_icon, LV_OPA_COVER, 0);
    }
  } else if (state < 20) {
    if (toggle) {
      lv_obj_set_style_img_opa(right_turn_icon, LV_OPA_TRANSP, 0);
    } else {
      lv_obj_set_style_img_opa(right_turn_icon, LV_OPA_COVER, 0);
    }
  }

  if (state % 12 > 6) {
    lv_obj_set_style_img_opa(high_beam_icon, LV_OPA_TRANSP, 0);
  } else {
    lv_obj_set_style_img_opa(high_beam_icon, LV_OPA_COVER, 0);
  }
}

/* Draw top-left and top-right icons */
static void draw_icons(lv_obj_t *scr) {

  // int icon_spacing = 4; // 图标之间的间距
  int margin = 2;

  // 左转灯图标（靠近左上角）
  left_turn_icon = lv_img_create(scr);
  lv_img_set_src(left_turn_icon, &left_turn);
  lv_obj_align(left_turn_icon, LV_ALIGN_TOP_LEFT, margin, margin);
  // lv_obj_set_style_img_opa(left_turn_icon, LV_OPA_TRANSP, 0); // 默认隐藏

  // 右转灯图标（靠近右上角）
  right_turn_icon = lv_img_create(scr);
  lv_img_set_src(right_turn_icon, &right_turn);
  lv_obj_align(right_turn_icon, LV_ALIGN_TOP_RIGHT, -margin, margin);
  // lv_obj_set_style_img_opa(right_turn_icon, LV_OPA_TRANSP, 0); // 默认隐藏

  // 高光灯图标（在左转灯右侧）
  high_beam_icon = lv_img_create(scr);
  lv_img_set_src(high_beam_icon, &high_beam);
  lv_obj_align(high_beam_icon, LV_ALIGN_TOP_LEFT, margin + 36 + margin, margin);
  // lv_obj_set_style_img_opa(high_beam_icon, LV_OPA_TRANSP, 0); // 默认隐藏

  lv_obj_move_foreground(left_turn_icon);
  lv_obj_move_foreground(high_beam_icon);
  lv_obj_move_foreground(right_turn_icon);

  lv_timer_create(update_icons_timer_cb, 500, NULL);
}
/* Draw vertical separators at left_x and right_x */
static void draw_separators(lv_obj_t *scr) {
  /* Compute layout and draw separators */
  int line_size = 2;
  int padding = 2; // 留给能量条 的padding
  const lv_coord_t mid_w = ENERGY_BAR_W;
  const lv_coord_t left_x = (SCREEN_W - mid_w - line_size) / 2 - padding;
  const lv_coord_t right_x = left_x + mid_w + padding * 2;
  const lv_coord_t top_h = 40;

  sep_line_left = lv_obj_create(scr);
  lv_obj_set_size(sep_line_left, line_size, SCREEN_H - top_h);
  lv_obj_set_style_bg_color(sep_line_left, theme_line, 0);
  lv_obj_set_style_border_width(sep_line_left, 0, 0);
  lv_obj_set_pos(sep_line_left, left_x, top_h);

  sep_line_right = lv_obj_create(scr);
  lv_obj_set_size(sep_line_right, line_size, SCREEN_H - top_h);
  lv_obj_set_style_bg_color(sep_line_right, theme_line, 0);
  lv_obj_set_style_border_width(sep_line_right, 0, 0);
  lv_obj_set_pos(sep_line_right, right_x, top_h);

  sep_line_top = lv_obj_create(scr);
  lv_obj_set_size(sep_line_top, SCREEN_W, line_size);
  lv_obj_set_style_bg_color(sep_line_top, theme_line, 0);
  lv_obj_set_style_border_width(sep_line_top, 0, 0);
  lv_obj_set_pos(sep_line_top, 0, top_h);
}

/* Draw energy bar at bottom center (only border and centered label) */
static void draw_energy_bar(lv_obj_t *scr) {
  // -2/6,  8分度，25% 回收分区 占比
  const lv_coord_t left_x = (SCREEN_W - ENERGY_BAR_W) / 2;
  const int zero_line_x =
      ENERGY_BAR_W * (-ENERGY_MIN_W) / (ENERGY_MAX_W + (-ENERGY_MIN_W));
  const int left_size = zero_line_x;
  const int right_size = ENERGY_BAR_W - left_size;

  /* Container: transparent background with border only */
  energy_bar_cont = lv_obj_create(scr);
  lv_obj_set_size(energy_bar_cont, ENERGY_BAR_W, ENERGY_BAR_H);
  lv_obj_set_style_bg_opa(energy_bar_cont, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(energy_bar_cont, 4, 0);
  lv_obj_set_style_border_color(energy_bar_cont, theme_border, 0);
  lv_obj_set_style_radius(energy_bar_cont, 8, 0);
  lv_obj_set_pos(energy_bar_cont, left_x, SCREEN_H - ENERGY_BAR_H - 5);
  lv_obj_clear_flag(energy_bar_cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(energy_bar_cont, 0, 0);
  lv_obj_set_style_clip_corner(energy_bar_cont, true, 0);

  int left_value = 0;
  int right_value = 0;

  energy_bar_left = lv_obj_create(energy_bar_cont);
  lv_obj_set_size(energy_bar_left, left_value, ENERGY_BAR_H - 8);
  lv_obj_align(energy_bar_left, LV_ALIGN_LEFT_MID, left_size - left_value, 0);
  lv_obj_set_style_bg_color(energy_bar_left, theme_energy_regen, 0);
  lv_obj_set_style_radius(energy_bar_left, 0, 0);
  lv_obj_set_style_border_width(energy_bar_left, 0, 0);
  lv_obj_set_style_bg_opa(energy_bar_left, LV_OPA_COVER, 0);
  lv_obj_clear_flag(energy_bar_left, LV_OBJ_FLAG_SCROLLABLE);

  energy_bar_right = lv_obj_create(energy_bar_cont);
  lv_obj_set_size(energy_bar_right, right_value, ENERGY_BAR_H - 8);
  lv_obj_align(energy_bar_right, LV_ALIGN_LEFT_MID, left_size, 0);
  lv_obj_set_style_bg_color(energy_bar_right, theme_energy_consume, 0);
  lv_obj_set_style_radius(energy_bar_right, 0, 0);
  lv_obj_set_style_border_width(energy_bar_right, 0, 0);
  lv_obj_set_style_bg_opa(energy_bar_right, LV_OPA_COVER, 0);
  lv_obj_clear_flag(energy_bar_right, LV_OBJ_FLAG_SCROLLABLE);

  /* Zero mark line */
  zero_line = lv_obj_create(energy_bar_cont);
  lv_obj_set_size(zero_line, 5, ENERGY_BAR_H);
  lv_obj_set_style_bg_color(zero_line, theme_line, 0);
  lv_obj_set_style_border_width(zero_line, 0, 0);
  lv_obj_align(zero_line, LV_ALIGN_LEFT_MID, zero_line_x, 0);

  energy_bar_label = lv_label_create(energy_bar_cont);
  lv_obj_align(energy_bar_label, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_color(energy_bar_label, theme_text_main, 0);
  /* Make the label larger and keep it above the fill rectangles */
  lv_obj_set_style_text_font(energy_bar_label, &lv_font_pf_din_mono_30, 0);
  lv_obj_clear_flag(energy_bar_label, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_move_foreground(energy_bar_label);

  // /* Initialize bar to zero power */
  dashboard_set_power(0);

  // /* Start animation timer to sweep -3000..9000 for demo */
  lv_timer_create(energy_anim_timer_cb, 100, NULL);
}

static void draw_current_time(lv_obj_t *scr) {
  time_label = lv_label_create(scr);
  lv_label_set_text(time_label, "00:00");
  lv_obj_clear_flag(time_label, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, 2);
  lv_obj_set_style_text_color(time_label, theme_text_main, 0);
  /* larger clock font */
  // lv_obj_set_style_text_font(time_label, &lv_font_montserrat_36, 0);
  lv_obj_set_style_text_font(time_label, &lv_font_jetbrains_mono_36, 0);
  // lv_obj_set_scrollbar_mode(time_label, LV_SCROLLBAR_MODE_OFF);

  lv_timer_create(time_timer_cb, 500, NULL);
}

/* Small helper to create a title/value pair at (x,y) */
static void create_kv_pair(lv_obj_t *parent, lv_obj_t **title_out,
                           lv_obj_t **value_out, const char *title,
                           const char *value, lv_coord_t x, lv_coord_t y) {
  int title_value_spacing = 14;

  *title_out = lv_label_create(parent);
  lv_label_set_text(*title_out, title);
  lv_obj_set_style_text_color(*title_out, theme_text_dim, 0);
  /* slightly smaller title font */
  lv_obj_set_style_text_font(*title_out, &lv_font_montserrat_14, 0);
  lv_obj_set_pos(*title_out, x, y);

  *value_out = lv_label_create(parent);
  lv_label_set_text(*value_out, value);
  lv_obj_set_style_text_color(*value_out, theme_text_main, 0);
  /* larger value font */
  lv_obj_set_style_text_font(*value_out, &lv_font_jetbrains_mono_26, 0);
  /* smaller gap between title and value */
  lv_obj_set_pos(*value_out, x, y + title_value_spacing);
}

/* Draw left and right side panels with requested info */
static void draw_side_panels(lv_obj_t *scr) {
  const lv_coord_t left_x = (SCREEN_W - ENERGY_BAR_W) / 2;
  const lv_coord_t right_x = left_x + ENERGY_BAR_W;
  /* move panels slightly up to make room for increased pair spacing */
  const lv_coord_t top_h = 44;
  /* slightly increase spacing between kv pairs */
  const lv_coord_t spacing = 46;

  /* Left side (x=8) */
  lv_coord_t lx = 8;
  create_kv_pair(scr, &left_odo_title, &left_odo_value, "ODO km", "00000.0", lx,
                 top_h + spacing * 0);
  create_kv_pair(scr, &left_trip_title, &left_trip_value, "TRIP km", "000.0",
                 lx, top_h + spacing * 1);
  create_kv_pair(scr, &left_ride_time_title, &left_ride_time_value, "RIDE TIME",
                 "00:00:00", lx, top_h + spacing * 2);
  create_kv_pair(scr, &left_max_speed_title, &left_max_speed_value,
                 "MAX SPD km/h", "42", lx, top_h + spacing * 3);
  create_kv_pair(scr, &left_used_title, &left_used_value, "USED kWh", "40.0",
                 lx, top_h + spacing * 4);

  /* Right side (aligned to right column) */
  lv_coord_t rx = right_x + 8;
  create_kv_pair(scr, &right_range_title, &right_range_value, "RANGE km",
                 "100", rx, top_h + spacing * 0); // 预估剩余续航
  create_kv_pair(scr, &right_hist_avg_title, &right_hist_avg_value, "AVG Wh/km",
                 "12.0", rx, top_h + spacing * 1);
  create_kv_pair(scr, &right_trip_avg_title, &right_trip_avg_value,
                 "TRIP Wh/km", "34.0", rx, top_h + spacing * 2);
  create_kv_pair(scr, &right_maxp_title, &right_maxp_value, "PEAK kW", "4.321",
                 rx, top_h + spacing * 3);
  create_kv_pair(scr, &right_batt_cap_title, &right_batt_cap_value, "BATT CAP kWh",
                 "42.0", rx, top_h + spacing * 4); // 电池容量
}


void draw_meter(lv_obj_t *scr)
{
    /* Create a scale widget (replaces the old meter widget) */
    lv_obj_t *meter = lv_scale_create(scr);
    lv_obj_center(meter);
    lv_obj_set_size(meter, 180, 180);

    /* Set to round inner mode for circular gauge */
    lv_scale_set_mode(meter, LV_SCALE_MODE_ROUND_INNER);

    lv_scale_set_range(meter, 0, 80);

    /* Set angle range (270 degrees) and rotation (135 degrees = bottom center) */
    lv_scale_set_angle_range(meter, 270);
    lv_scale_set_rotation(meter, 135);

    /* Set tick count: 41 total ticks, major tick every 5 ticks (so 9 major ticks) */
    lv_scale_set_total_tick_count(meter, 41);
    lv_scale_set_major_tick_every(meter, 5);

    /* Enable labels */
    lv_scale_set_label_show(meter, true);

    /* Set tick line width - make ticks thicker */
    // lv_obj_set_style_line_width(meter, 3, LV_PART_INDICATOR);  /* Major ticks width */
    // lv_obj_set_style_line_width(meter, 2, LV_PART_ITEMS);     /* Minor ticks width */
    lv_obj_set_style_length(meter, 15, LV_PART_INDICATOR);     //大刻度长度
    lv_obj_set_style_length(meter, 10, LV_PART_ITEMS);     //小刻度长度
    lv_obj_set_style_arc_width(meter, 5, LV_PART_MAIN); //外圈粗细


    /* Add a blue section for 0-20 */
    lv_scale_section_t * blue_section = lv_scale_add_section(meter);
    lv_scale_set_section_range(meter, blue_section, 0, 20);

    /* Setup style for blue section */
    lv_style_init(&meter_blue_style);
    lv_style_set_arc_color(&meter_blue_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_line_color(&meter_blue_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_scale_set_section_style_main(meter, blue_section, &meter_blue_style);
    lv_scale_set_section_style_indicator(meter, blue_section, &meter_blue_style);

    /* Add a red section for 60-80 */
    lv_scale_section_t * red_section = lv_scale_add_section(meter);
    lv_scale_set_section_range(meter, red_section, 60, 80);

    /* Setup style for red section */
    lv_style_init(&meter_red_style);
    lv_style_set_arc_color(&meter_red_style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_line_color(&meter_red_style, lv_palette_main(LV_PALETTE_RED));
    lv_scale_set_section_style_main(meter, red_section, &meter_red_style);
    lv_scale_set_section_style_indicator(meter, red_section, &meter_red_style);

    /* Add a needle line indicator */
    lv_obj_t * needle_line = lv_line_create(meter);
    lv_obj_set_style_line_width(needle_line, 4, LV_PART_MAIN);
    lv_obj_set_style_line_color(needle_line, theme_needle, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(needle_line, true, LV_PART_MAIN);
    /* Optional: set pad_right to offset needle start (may not directly affect line points) */
    lv_obj_set_style_pad_right(needle_line, 30, LV_PART_MAIN);
    lv_scale_set_line_needle_value(meter, needle_line, -10, 0);

    /* Create a circle to cover the center area and hide the needle's center part */
    meter_center_circle = lv_obj_create(scr);
    lv_obj_set_size(meter_center_circle, 80, 80);
    lv_obj_center(meter_center_circle);
    lv_obj_set_style_radius(meter_center_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(meter_center_circle, theme_bg, 0);
    lv_obj_set_style_bg_opa(meter_center_circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(meter_center_circle, 0, LV_PART_MAIN);
    /* Move circle to foreground to cover the needle */
    lv_obj_move_foreground(meter_center_circle);
    lv_obj_clear_flag(meter_center_circle, LV_OBJ_FLAG_SCROLLABLE);

    /* Create a container for center labels (value and unit) */
    lv_obj_t *center_container = lv_obj_create(meter_center_circle);
    lv_obj_set_size(center_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(center_container);
    lv_obj_set_style_bg_opa(center_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(center_container, 0, 0);
    lv_obj_set_layout(center_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(center_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(center_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(center_container, 0, 0);
    lv_obj_set_style_pad_row(center_container, 2, 0);

    /* Add center label to display current value */
    meter_center_label = lv_label_create(center_container);
    lv_label_set_text(meter_center_label, "0");
    lv_obj_set_style_text_font(meter_center_label, &lv_font_jetbrains_mono_extra_bold_42, 0);
    lv_obj_set_style_text_color(meter_center_label, theme_text_main, 0);
    lv_obj_set_style_text_align(meter_center_label, LV_TEXT_ALIGN_CENTER, 0);

    /* Add unit label below the value */
    meter_unit_label = lv_label_create(center_container);
    lv_label_set_text(meter_unit_label, "km/h");
    lv_obj_set_style_text_font(meter_unit_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(meter_unit_label, theme_text_dim, 0);
    lv_obj_set_style_text_align(meter_unit_label, LV_TEXT_ALIGN_CENTER, 0);


    /* Save references for animation */
    meter_widget = meter;
    meter_needle_line = needle_line;

    /* Start animation timer: update every 50ms for smooth animation */
    lv_timer_create(meter_anim_timer_cb, 50, NULL);
}

void change_theme_timer_cb(lv_timer_t *t) {

  dashboard_set_night_mode(!dashboard_night_mode);
}

void dashboard_create(void) {
  /* Create a clean screen */
  scr_root = lv_obj_create(NULL);
  lv_scr_load(scr_root);
  lv_obj_set_scrollbar_mode(scr_root, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_bg_opa(scr_root, LV_OPA_COVER, 0);

  /* Initialize theme colors first (needed by draw functions) */
  dashboard_night_mode = true;
  dashboard_init_theme_colors();

  /* Draw all UI components */
  draw_separators(scr_root);
  draw_current_time(scr_root);
  draw_icons(scr_root);
  draw_side_panels(scr_root);
  draw_energy_bar(scr_root);
  draw_meter(scr_root);

  /* Apply theme to all created components */
  dashboard_apply_theme();

  lv_timer_create(change_theme_timer_cb, 10000, NULL);
}
