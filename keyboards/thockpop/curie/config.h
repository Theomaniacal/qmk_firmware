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

/* key matrix size */
#define MATRIX_ROWS 10
#define MATRIX_COLS 10

/* key matrix pins */
#define MATRIX_ROW_PINS { 19, 17, 14, 9, 11, 28, 16, 13, 10, 12 }
#define MATRIX_COL_PINS { 18, 15, 5, 6, 7, 8, 4, 3, 2, 1 }

/* COL2ROW or ROW2COL */
#define DIODE_DIRECTION COL2ROW

/* RGB settings */
#define RGBLIGHT_LAYERS
#define RGBLIGHT_SLEEP // If defined, the RGB lighting will be switched off when the host goes to sleep */
#define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_STATIC_LIGHT

/* LCD */
#define SPI_DRIVER   SPID1
#define SPI_SCK_PIN  GP26
#define SPI_MOSI_PIN GP27
#define SPI_MISO_PIN GP0 // MISO not used, so we set to a leftover pin.
#define LCD_CS_PIN   GP25
#define LCD_DC_PIN   GP23
#define LCD_BL_PIN   GP22
#define LCD_RST_PIN  GP24
#define LCD_SPI_DIV 2
#define LCD_SPI_MODE 0
#define LCD_WIDTH 160
#define LCD_HEIGHT 80