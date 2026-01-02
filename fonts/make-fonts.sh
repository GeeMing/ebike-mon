#!/usr/bin/env bash
set -e

OUT_DIR=out

# use script path as cwd
SCRIPT_DIR=$(dirname "$0")
cd "$SCRIPT_DIR"

gen_font () {
    local TTF="$1"
    local SIZE="$2"
    local NAME="$3"

    local C_FILE="${OUT_DIR}/${NAME}.c"
    local H_FILE="${OUT_DIR}/${NAME}.h"

    echo "Generating ${NAME}..."

    npx lv_font_conv \
        --font "$TTF" \
        --size "$SIZE" \
        --bpp 4 \
        --format lvgl \
        -r 0x20-0x7E \
        --lv-font-name "$NAME" \
        --no-compress \
        -o "$C_FILE"
        # --symbols "0123456789.-+%kmWhAV " \
}

gen_font "JetBrainsMonoNL-SemiBold.ttf" 36 lv_font_jetbrains_mono_36
gen_font "JetBrainsMonoNL-SemiBold.ttf" 26 lv_font_jetbrains_mono_26
# gen_font "JetBrainsMonoNL-SemiBold.ttf" 30 lv_font_jetbrains_mono_30
gen_font "PF Din Mono.ttf"              30 lv_font_pf_din_mono_30
