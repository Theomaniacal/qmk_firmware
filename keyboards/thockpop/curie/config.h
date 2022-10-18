/*
Copyright 2022 THOCKPOP LLC

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID       0x7470 // "tp"
#define PRODUCT_ID      0x0001
#define DEVICE_VER      0x0001
#define MANUFACTURER    thockpop
#define PRODUCT         Curie

/* key matrix size */
#define MATRIX_ROWS 10
#define MATRIX_COLS 10

/* key matrix pins */
#define MATRIX_ROW_PINS { 19, 17, 14, 9, 11, 28, 16, 13, 10, 12 }
#define MATRIX_COL_PINS { 18, 15, 5, 6, 7, 8, 4, 3, 2, 1 }

/* COL2ROW or ROW2COL */
#define DIODE_DIRECTION COL2ROW

/* RGB settings */
#define RGB_DI_PIN GP29
#define RGBLED_NUM 47
#define DRIVER_LED_TOTAL 47
#define RGBLIGHT_LAYERS
#define RGBLIGHT_SLEEP // If defined, the RGB lighting will be switched off when the host goes to sleep */
#define RGBLIGHT_LIMIT_VAL 150 // The maximum brightness level
#define RGBLIGHT_DEFAULT_HUE 185 // Violet
#define RGBLIGHT_DEFAULT_SAT 100
#define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_STATIC_LIGHT
#define RGBLIGHT_EFFECT_BREATHING
#define RGBLIGHT_EFFECT_RAINBOW_SWIRL

/* encoders */
#define ENCODERS_PAD_A { GP20 }
#define ENCODERS_PAD_B { GP21 }
#define ENCODER_RESOLUTION 4

/* OLED */
#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_UPDATE_INTERVAL 40
#define OLED_TIMEOUT 60000
#define OLED_FADE_OUT