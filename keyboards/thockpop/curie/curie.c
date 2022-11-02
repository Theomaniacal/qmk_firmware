  
/* Copyright 2022 THOCKPOP LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "curie.h"
#include <qp.h>
#include "raw_hid.h"
#include "fonts/montserrat11.qff.h"
#include "fonts/montserrat14.qff.h"
#include "fonts/lato22.qff.h"
#include "graphics/signature.qgf.h"

#define ENCODER_LONG_PRESS_TIME 300 // 0.3 second for press to count as long press
#define ANIM_SPEED_MAX 60
#define ANIM_SPEED_MIN 10
#define DISPLAY_MENU_MODE_AMOUNT 3 // Used to represent how many OLED modes cannot be saved
#define ENC_MENU_MODE_AMOUNT 1 // Used to represent how many Encoder modes cannot be saved
#define LCD_TIMEOUT 30000

static painter_device_t display;
static painter_font_handle_t small_font;
static painter_font_handle_t med_font;
static painter_font_handle_t large_font;

static painter_image_handle_t signature;

static bool should_redraw_display = true;
static bool encoder_is_down = false;
static bool encoder_secondary_push_active = false;
static bool encoder_is_short_pressed = false;
static bool encoder_is_long_pressed = false;
static bool should_menu_select_wrap = false;
static uint32_t encoder_press_timer = 0;
static int8_t active_select_index = 0;
static int8_t max_select_index = 0;
static bool is_alt_tab_active = false;
static uint16_t alt_tab_timer = 0;

static enum encoder_modes {
    Select,
    Media,
    Window,
    Scroll,
    Lighting
} enc_mode, saved_enc_mode;

static enum display_modes {
    Menu,
    Display_Menu,
    Enc_Menu,
    Logo,
    WPM,
    System,
    Weather,
    WPM2,
    NA2,
    NA3,
    NA4,
    Blank,
    Off
} display_mode, saved_display_mode;

// DISPLAY STUFF -----------------
char wpm_str[10];

/*void raw_hid_receive(uint8_t *data, uint8_t length) {
    // Your code goes here. data is the packet received from host.
}*/

// DISPLAY Draw Functions
void draw_text_centered(painter_device_t device, uint8_t x, uint8_t y, painter_font_handle_t font, char* str, uint8_t hue_fg, uint8_t sat_fg, uint8_t val_fg, uint8_t hue_bg, uint8_t sat_bg, uint8_t val_bg) {
    int16_t text_width = qp_textwidth(font, str);
    qp_drawtext_recolor(device, x - (text_width / 2), y - (font->line_height / 2), font, str, hue_fg, sat_fg, val_fg, hue_bg, sat_bg, val_bg);
}

void draw_button(painter_device_t device, uint8_t x, uint8_t y, uint8_t width, uint8_t height, painter_font_handle_t font, char* str, uint8_t hue_fg, uint8_t sat_fg, uint8_t val_fg, uint8_t hue_bg, uint8_t sat_bg, uint8_t val_bg, uint8_t hue_filled_fg, uint8_t sat_filled_fg, uint8_t val_filled_fg, uint8_t hue_filled_bg, uint8_t sat_filled_bg, uint8_t val_filled_bg, bool filled) {
    uint8_t half_width = width / 2;
    uint8_t half_height = height / 2;

    if (filled) {
        qp_rect(device, x - half_width + 4, y - half_height, x + half_width - 4, y + half_height, hue_filled_bg, sat_filled_bg, val_filled_bg, true);

        qp_line(device, x - half_width + 3, y - half_height + 1, x - half_width + 3, y + half_height - 1, hue_filled_bg, sat_filled_bg, val_filled_bg);
        qp_line(device, x + half_width - 3, y - half_height + 1, x + half_width - 3, y + half_height - 1, hue_filled_bg, sat_filled_bg, val_filled_bg);

        qp_line(device, x - half_width + 2, y - half_height + 1, x - half_width + 2, y + half_height - 1, hue_filled_bg, sat_filled_bg, val_filled_bg);
        qp_line(device, x + half_width - 2, y - half_height + 1, x + half_width - 2, y + half_height - 1, hue_filled_bg, sat_filled_bg, val_filled_bg);

        qp_line(device, x - half_width + 1, y - half_height + 2, x - half_width + 1, y + half_height - 2, hue_filled_bg, sat_filled_bg, val_filled_bg);
        qp_line(device, x + half_width - 1, y - half_height + 2, x + half_width - 1, y + half_height - 2, hue_filled_bg, sat_filled_bg, val_filled_bg);

        qp_line(device, x - half_width, y - half_height + 4, x - half_width, y + half_height - 4, hue_filled_bg, sat_filled_bg, val_filled_bg);
        qp_line(device, x + half_width, y - half_height + 4, x + half_width, y + half_height - 4, hue_filled_bg, sat_filled_bg, val_filled_bg);



        qp_line(device, x - half_width + 4, y - half_height, x + half_width - 4, y - half_height, hue_fg, sat_fg, val_fg);
        qp_line(device, x - half_width + 4, y + half_height, x + half_width - 4, y + half_height, hue_fg, sat_fg, val_fg);

        qp_line(device, x - half_width + 2, y - half_height + 1, x - half_width + 3, y - half_height + 1,  hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width - 2, y - half_height + 1, x + half_width - 3, y - half_height + 1,  hue_fg, sat_fg, val_fg);
        qp_line(device, x - half_width + 2, y + half_height - 1, x - half_width + 3, y + half_height - 1,  hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width - 2, y + half_height - 1, x + half_width - 3, y + half_height - 1,  hue_fg, sat_fg, val_fg);

        qp_line(device, x - half_width + 1, y - half_height + 2, x - half_width + 1, y - half_height + 3,  hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width - 1, y - half_height + 2, x + half_width - 1, y - half_height + 3,  hue_fg, sat_fg, val_fg);
        qp_line(device, x - half_width + 1, y + half_height - 2, x - half_width + 1, y + half_height - 3,  hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width - 1, y + half_height - 2, x + half_width - 1, y + half_height - 3,  hue_fg, sat_fg, val_fg);

        qp_line(device, x - half_width, y - half_height + 4, x - half_width, y + half_height - 4, hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width, y - half_height + 4, x + half_width, y + half_height - 4, hue_fg, sat_fg, val_fg); 

        draw_text_centered(device, x, y, font, str, hue_filled_fg, sat_filled_fg, val_filled_fg, hue_filled_bg, sat_filled_bg, val_filled_bg);
    } else {
        qp_line(device, x - half_width + 4, y - half_height, x + half_width - 4, y - half_height, hue_fg, sat_fg, val_fg);
        qp_line(device, x - half_width + 4, y + half_height, x + half_width - 4, y + half_height, hue_fg, sat_fg, val_fg);

        qp_line(device, x - half_width + 2, y - half_height + 1, x - half_width + 3, y - half_height + 1,  hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width - 2, y - half_height + 1, x + half_width - 3, y - half_height + 1,  hue_fg, sat_fg, val_fg);
        qp_line(device, x - half_width + 2, y + half_height - 1, x - half_width + 3, y + half_height - 1,  hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width - 2, y + half_height - 1, x + half_width - 3, y + half_height - 1,  hue_fg, sat_fg, val_fg);

        qp_line(device, x - half_width + 1, y - half_height + 2, x - half_width + 1, y - half_height + 3,  hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width - 1, y - half_height + 2, x + half_width - 1, y - half_height + 3,  hue_fg, sat_fg, val_fg);
        qp_line(device, x - half_width + 1, y + half_height - 2, x - half_width + 1, y + half_height - 3,  hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width - 1, y + half_height - 2, x + half_width - 1, y + half_height - 3,  hue_fg, sat_fg, val_fg);

        qp_line(device, x - half_width, y - half_height + 4, x - half_width, y + half_height - 4, hue_fg, sat_fg, val_fg);
        qp_line(device, x + half_width, y - half_height + 4, x + half_width, y + half_height - 4, hue_fg, sat_fg, val_fg); 

        draw_text_centered(device, x, y, font, str, hue_fg, sat_fg, val_fg, hue_bg, sat_bg, val_bg);
    }
}

static void change_encoder_mode(enum encoder_modes mode) {
    enc_mode = mode;
    should_redraw_display = true;

    if (mode != Select) {
        saved_enc_mode = mode;
    }
}

static void change_display_mode(enum display_modes mode) {
    display_mode = mode;
    should_redraw_display = true;

    switch (mode) {
        case Menu:
            max_select_index = 1; // Last enum element lets us know the max index
            break;
        case Display_Menu:
            max_select_index = Off - DISPLAY_MENU_MODE_AMOUNT; // Last enum element lets us know the max index
            break;
        case Enc_Menu:
            max_select_index = Lighting - ENC_MENU_MODE_AMOUNT; // Last enum element lets us know the max index
            break;
        default:
            saved_display_mode = mode;
            break;
    }
}

void increase_active_select_index(int8_t amount) {
    should_redraw_display = true;

    if (active_select_index + amount > max_select_index) { // If active index is greater than max index, wrap the selection to first index
        active_select_index = should_menu_select_wrap ? 0 : max_select_index;
    } else if (active_select_index + amount < 0) { // If active index is less than 0, wrap the selection to last index
        active_select_index = should_menu_select_wrap ? max_select_index : 0;
    } else {
        active_select_index += amount;
    }

}

// Handle encoder presses
void enc_state_update(void) {
    if (encoder_is_down && timer_elapsed32(encoder_press_timer) >= ENCODER_LONG_PRESS_TIME) {
        encoder_is_down = false;
        encoder_press_timer = timer_read32();

        encoder_is_long_pressed = true;
    }

    if (encoder_is_long_pressed) {
        encoder_is_long_pressed = false;
        switch (enc_mode) {
            case Select:
                change_encoder_mode(saved_enc_mode);
                change_display_mode(saved_display_mode);
                break;

            default:
                active_select_index = 0;
                change_encoder_mode(Select);
                change_display_mode(Menu);
                
                break;
        }
    } else if (encoder_is_short_pressed) {
        encoder_is_short_pressed = false;
        switch (enc_mode) {
            case Select:
                switch (display_mode) {
                    case Menu:
                        switch (active_select_index) {
                            case 0:
                                active_select_index = saved_display_mode - DISPLAY_MENU_MODE_AMOUNT; // Minus 3 because the first 3 OLED modes are menus
                                change_display_mode(Display_Menu);
                                change_encoder_mode(Select);
                                break;
                            case 1:
                                active_select_index = saved_enc_mode - ENC_MENU_MODE_AMOUNT; // Minus 1 because the first encoder mode is Select
                                change_display_mode(Enc_Menu);
                                change_encoder_mode(Select);
                                break;
                        }
                        break;
                    case Display_Menu:
                        if (active_select_index != Blank - DISPLAY_MENU_MODE_AMOUNT) {
                            change_display_mode(active_select_index + DISPLAY_MENU_MODE_AMOUNT);
                            change_encoder_mode(saved_enc_mode);
                        }


                        /*switch (active_select_index)
                        {
                            case 0:
                                change_display_mode(Logo);
                                change_encoder_mode(saved_enc_mode);
                                break;
                            case 1:
                                change_display_mode(WPM);
                                change_encoder_mode(saved_enc_mode);
                                break;
                            case 2:
                                change_display_mode(System);
                                change_encoder_mode(saved_enc_mode);
                                break;
                            case 3:
                                change_display_mode(Weather);
                                change_encoder_mode(saved_enc_mode);
                                break;
                            case 4:
                                // Do nothing. Used so user has to move selection once more right to confirm "Display Off"
                                break;
                            case 5:
                                change_display_mode(Off);
                                change_encoder_mode(saved_enc_mode);
                                break;
                        }*/
                        break;

                    case Enc_Menu:
                        change_encoder_mode(active_select_index + ENC_MENU_MODE_AMOUNT);
                        change_display_mode(saved_display_mode);
                        
                        /*switch (active_select_index) {
                            case 0:
                                change_encoder_mode(Media);
                                break;
                            case 1:
                                change_encoder_mode(Window);
                                break;
                            case 2:
                                change_encoder_mode(Scroll);
                                break;
                            case 3:
                                change_encoder_mode(Lighting);
                                break;
                        }*/
                        break;

                    default:
                        break;
                }
                break;
            case Media:
                if (encoder_secondary_push_active) {
                    encoder_secondary_push_active = false;
                    tap_code(KC_MUTE);
                } else {
                    tap_code(KC_MPLY);
                }

                break;
            case Window:
                    if (encoder_secondary_push_active) {
                        encoder_secondary_push_active = false;
                        tap_code(KC_MUTE);
                    } else {
                        if (is_alt_tab_active) {
                            tap_code(KC_ENT);
                        } else {
                            tap_code(KC_MPLY);
                        }
                        
                    }
                break;
            case Scroll:
                if (encoder_secondary_push_active) {
                    encoder_secondary_push_active = false;
                    tap_code(KC_MUTE);
                } else {
                    tap_code(KC_MPLY);
                }
                break;
            case Lighting:
                if (encoder_secondary_push_active) {
                    encoder_secondary_push_active = false;
                    rgblight_step();
                } else {
                    rgblight_toggle();
                }
                break;
        }
    }
}

void keyboard_post_init_kb(void) {
    enc_mode = Media;
    saved_enc_mode = enc_mode;

    display_mode = Logo;
    saved_display_mode = display_mode;

    setPinOutput(LCD_BL_PIN);
    writePinHigh(LCD_BL_PIN);

    wait_ms(150);

    display = qp_st7735_make_spi_device(80, 160, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, LCD_SPI_DIV, LCD_SPI_MODE); // Create the display
    qp_init(display, QP_ROTATION_270);   // Initialise the display

    small_font = qp_load_font_mem(font_montserrat11);
    med_font = qp_load_font_mem(font_montserrat14);
    large_font = qp_load_font_mem(font_lato22);

    signature = qp_load_image_mem(gfx_signature);

    qp_power(display, true);
    qp_clear(display);

    keyboard_post_init_user();
}

void suspend_power_down_user(void) {
    qp_power(display, false);
    writePinLow(LCD_BL_PIN);
}

void suspend_wakeup_init_user(void) {
    qp_power(display, true);
    writePinHigh(LCD_BL_PIN);
}

void housekeeping_task_kb(void) {
    if (last_input_activity_elapsed() < LCD_TIMEOUT) {
        qp_power(display, true);
    } else {
        qp_power(display, false);
        change_encoder_mode(saved_enc_mode);
        change_display_mode(saved_display_mode);
    }

    static uint32_t last_draw = 0;
    if (timer_elapsed32(last_draw) > 33) { // Throttle to 30fps
        last_draw = timer_read32();
    } else {
        return;
    }

    if (!should_redraw_display) { return; }
    should_redraw_display = false;
    qp_rect(display, 0, 0, 160, 81, HSV_BLACK, true);

    switch (display_mode) {
        case Logo: {
            //qp_drawimage(display, 0, 0, signature);
            qp_drawimage_recolor(display, 0, 0, signature, 0, 0, 255, rgblight_get_hue(), 150, 150);

            //draw_text_centered(display, 80, 30, large_font, "C  U  R  I  E", HSV_WHITE, HSV_BLACK);
            //draw_text_centered(display, 80, 50, med_font, "SOUTHPAW", HSV_WHITE, HSV_BLACK);

            break;
        }

        case Menu:
            draw_button(display, 44, 30, 68, 20, small_font, "DISPLAY", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 0);
            draw_button(display, 116, 30, 68, 20, small_font, "ENCODER", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 1);

            switch (saved_display_mode) {
                case Logo: { draw_text_centered(display, 44, 50, small_font, "Logo", 0, 0, 230, HSV_BLACK); break; }
                case WPM: { draw_text_centered(display, 44, 50, small_font, "WPM", 0, 0, 230, HSV_BLACK); break; }
                case System: { draw_text_centered(display, 44, 50, small_font, "System", 0, 0, 230, HSV_BLACK); break; }
                case Weather: { draw_text_centered(display, 44, 50, small_font, "Weather", 0, 0, 230, HSV_BLACK); break; }
                case WPM2: { draw_text_centered(display, 44, 50, small_font, "WPM2", 0, 0, 230, HSV_BLACK); break; }
                case NA2: { draw_text_centered(display, 44, 50, small_font, "NA2", 0, 0, 230, HSV_BLACK); break; }
                case NA3: { draw_text_centered(display, 44, 50, small_font, "NA3", 0, 0, 230, HSV_BLACK); break; }
                case NA4: { draw_text_centered(display, 44, 50, small_font, "NA4", 0, 0, 230, HSV_BLACK); break; }
                case Off: { draw_text_centered(display, 44, 50, small_font, "Off", 0, 0, 230, HSV_BLACK); break; }
                default: { draw_text_centered(display, 44, 50, small_font, "Error", 0, 0, 230, HSV_BLACK); break; }

            }

            switch (saved_enc_mode) {
                case Media: { draw_text_centered(display, 116, 50, small_font, "Media", 0, 0, 230, HSV_BLACK); break; }
                case Window: { draw_text_centered(display, 116, 50, small_font, "Window", 0, 0, 230, HSV_BLACK); break; }
                case Scroll: { draw_text_centered(display, 116, 50, small_font, "Scroll", 0, 0, 230, HSV_BLACK); break; }
                case Lighting: { draw_text_centered(display, 116, 50, small_font, "Lighting", 0, 0, 230, HSV_BLACK); break; }
                default: { draw_text_centered(display, 116, 50, small_font, "Error", 0, 0, 230, HSV_BLACK); break; }

            }

            break;

        case Display_Menu:
            if (active_select_index < 4) { // 0, 1, 2, 3
                draw_text_centered(display, LCD_WIDTH / 2, 16, small_font, "Display", 0, 0, 230, HSV_BLACK);

                draw_button(display, 44, 35, 68, 20, small_font, "LOGO", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 0);
                draw_button(display, 116, 35, 68, 20, small_font, "WPM", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 1);
                draw_button(display, 44, 55, 68, 20, small_font, "SYSTEM", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 2);
                draw_button(display, 116, 55, 68, 20, small_font, "WEATHER", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 3);

                //draw_arrow(OLED_WIDTH - 8, OLED_HEIGHT / 2 + 4, 6, true); // right arrow         
            } else if (active_select_index < 8) { // 4, 5, 6, 7
                draw_text_centered(display, LCD_WIDTH / 2, 16, small_font, "Display", 0, 0, 230, HSV_BLACK);

                draw_button(display, 44, 35, 68, 20, small_font, "WPM2", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 4);
                draw_button(display, 116, 35, 68, 20, small_font, "NA2", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 5);
                draw_button(display, 44, 55, 68, 20, small_font, "NA3", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 6);
                draw_button(display, 116, 55, 68, 20, small_font, "NA4", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 7);

                //draw_arrow(7, OLED_HEIGHT / 2 + 4, 6, false); // left arrow
                //draw_arrow(OLED_WIDTH - 8, OLED_HEIGHT / 2 + 4, 6, true); // right arrow
            } else { // 8, 9
                draw_button(display, LCD_WIDTH / 2, LCD_HEIGHT / 2, 90, 20, small_font, "DISPLAY OFF", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 9);
                
                //draw_arrow(15, OLED_HEIGHT / 2 + 4, 6, false);  // left arrow
            }

            break;

        case Enc_Menu:
            draw_text_centered(display, LCD_WIDTH / 2, 16, small_font, "Encoder", 0, 0, 230, HSV_BLACK);

            draw_button(display, 44, 35, 68, 20, small_font, "MEDIA", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 0);
            draw_button(display, 116, 35, 68, 20, small_font, "WINDOW", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 1);
            draw_button(display, 44, 55, 68, 20, small_font, "SCROLL", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 2);
            draw_button(display, 116, 55, 68, 20, small_font, "LIGHTING", HSV_WHITE, HSV_BLACK, HSV_WHITE, HSV_BLUE, active_select_index == 3);

            break;

        case WPM: {
            int16_t current_wpm = get_current_wpm();
            if (current_wpm >= 240) {
                draw_text_centered(display, LCD_WIDTH / 2, LCD_HEIGHT / 2, large_font, "MAX", HSV_WHITE, HSV_BLACK);
            } else {
                sprintf(wpm_str, "%d", current_wpm);
                draw_text_centered(display, LCD_WIDTH / 2, LCD_HEIGHT / 2, large_font, wpm_str, HSV_WHITE, HSV_BLACK);
            }

            should_redraw_display = true;
            break;
        }

        case System:
            draw_text_centered(display, LCD_WIDTH / 2, LCD_HEIGHT / 2, small_font, "System", HSV_WHITE, HSV_BLACK);
            break;

        case Weather:
            draw_text_centered(display, LCD_WIDTH / 2, LCD_HEIGHT / 2, small_font, "Weather", HSV_WHITE, HSV_BLACK);
            break;

        case WPM2: {
            uint8_t current_wpm = get_current_wpm();
            uint8_t num_of_hor_lines = current_wpm / 5;
            if (num_of_hor_lines >= 4) { num_of_hor_lines = 4; }
            uint8_t num_of_vert_lines = (current_wpm - 65) / 5;
            if (num_of_vert_lines >= 14) { num_of_vert_lines = 14; }
            uint8_t num_of_redlines = (current_wpm - 140) / 5;
            if (num_of_redlines >= 30) { num_of_redlines = 30; }
            uint8_t starting_x_pos = 40;
            uint8_t starting_y_pos = 60;

            for (int i = 0; i < num_of_hor_lines; i++) {
                //draw_line_h(starting_x_pos + i, (starting_y_pos) - (i * 3), 11, true);
                //draw_line_h(starting_x_pos + 1 + i, (starting_y_pos) - 1 - (i * 3), 11, true);
                qp_line(display, starting_x_pos + i, starting_y_pos - (i * 3), starting_x_pos + i + 11, starting_y_pos - (i * 3), HSV_WHITE);
                qp_line(display, starting_x_pos + 1 + i, starting_y_pos - 1 - (i * 3), starting_x_pos + i + 11, starting_y_pos - 1 - (i * 3), HSV_WHITE);
            }

            if (current_wpm >= 30) {
                //draw_line_h(starting_x_pos + 4, starting_y_pos - 12, 12, true);
                //draw_line_h(starting_x_pos + 5, starting_y_pos - 13, 8, true);
                //draw_line_h(starting_x_pos + 5, starting_y_pos - 14, 4, true);
                qp_line(display, starting_x_pos + 4, starting_y_pos - 12, starting_x_pos + 16, starting_y_pos - 12, HSV_WHITE);
                qp_line(display, starting_x_pos + 5, starting_y_pos - 13, starting_x_pos + 13, starting_y_pos - 13, HSV_WHITE);
                qp_line(display, starting_x_pos + 5, starting_y_pos - 14, starting_x_pos + 9, starting_y_pos - 14, HSV_WHITE);
            }
            
            if (current_wpm >= 35) {
                //draw_line_h(starting_x_pos + 13, starting_y_pos - 14, 4, true);
                //draw_line_h(starting_x_pos + 9, starting_y_pos - 15, 6, true);
                //draw_line_h(starting_x_pos + 6, starting_y_pos - 16, 7, true);
                //draw_line_h(starting_x_pos + 7, starting_y_pos - 17, 4, true);
                //draw_line_h(starting_x_pos + 7, starting_y_pos - 18, 2, true);
                qp_line(display, starting_x_pos + 13, starting_y_pos - 14, starting_x_pos + 17, starting_y_pos - 14, HSV_WHITE);
                qp_line(display, starting_x_pos + 9, starting_y_pos - 15, starting_x_pos + 15, starting_y_pos - 15, HSV_WHITE);
                qp_line(display, starting_x_pos + 6, starting_y_pos - 16, starting_x_pos + 13, starting_y_pos - 16, HSV_WHITE);
                qp_line(display, starting_x_pos + 7, starting_y_pos - 17, starting_x_pos + 11, starting_y_pos - 17, HSV_WHITE);
                qp_line(display, starting_x_pos + 7, starting_y_pos - 18, starting_x_pos + 9, starting_y_pos - 18, HSV_WHITE);
            }

            if (current_wpm >= 40) {
                //oled_write_pixel(starting_x_pos + 17, starting_y_pos - 15, true);
                //draw_line_h(starting_x_pos + 15, starting_y_pos - 16, 2, true);
                //draw_line_h(starting_x_pos + 13, starting_y_pos - 17, 3, true);
                //draw_line_h(starting_x_pos + 11, starting_y_pos - 18, 4, true);
                //draw_line_h(starting_x_pos + 9, starting_y_pos - 19, 5, true);
                //draw_line_h(starting_x_pos + 8, starting_y_pos - 20, 4, true);
                //draw_line_h(starting_x_pos + 9, starting_y_pos - 21, 2, true);
                qp_line(display, starting_x_pos + 17, starting_y_pos - 15, starting_x_pos + 17, starting_y_pos - 15, HSV_WHITE);
                qp_line(display, starting_x_pos + 15, starting_y_pos - 16, starting_x_pos + 17, starting_y_pos - 16, HSV_WHITE);
                qp_line(display, starting_x_pos + 13, starting_y_pos - 17, starting_x_pos + 16, starting_y_pos - 17, HSV_WHITE);
                qp_line(display, starting_x_pos + 11, starting_y_pos - 18, starting_x_pos + 15, starting_y_pos - 18, HSV_WHITE);
                qp_line(display, starting_x_pos + 9, starting_y_pos - 19, starting_x_pos + 14, starting_y_pos - 19, HSV_WHITE);
                qp_line(display, starting_x_pos + 8, starting_y_pos - 20, starting_x_pos + 12, starting_y_pos - 20, HSV_WHITE);
                qp_line(display, starting_x_pos + 9, starting_y_pos - 21, starting_x_pos + 11, starting_y_pos - 21, HSV_WHITE);
            }

            if (current_wpm >= 45) {
                //draw_line_v(starting_x_pos + 11, starting_y_pos - 23, 2, true);
                //draw_line_v(starting_x_pos + 12, starting_y_pos - 24, 4, true);
                //draw_line_v(starting_x_pos + 13, starting_y_pos - 24, 4, true);
                //draw_line_v(starting_x_pos + 14, starting_y_pos - 23, 4, true);
                //draw_line_v(starting_x_pos + 15, starting_y_pos - 21, 3, true);
                //draw_line_v(starting_x_pos + 16, starting_y_pos - 20, 3, true);
                //draw_line_v(starting_x_pos + 17, starting_y_pos - 19, 3, true);
                //draw_line_v(starting_x_pos + 18, starting_y_pos - 18, 3, true);
                //draw_line_v(starting_x_pos + 19, starting_y_pos - 17, 1, true);
                qp_line(display, starting_x_pos + 11, starting_y_pos - 23, starting_x_pos + 11, starting_y_pos - 21, HSV_WHITE);
                qp_line(display, starting_x_pos + 12, starting_y_pos - 24, starting_x_pos + 12, starting_y_pos - 20, HSV_WHITE);
                qp_line(display, starting_x_pos + 13, starting_y_pos - 24, starting_x_pos + 13, starting_y_pos - 20, HSV_WHITE);
                qp_line(display, starting_x_pos + 14, starting_y_pos - 23, starting_x_pos + 14, starting_y_pos - 19, HSV_WHITE);
                qp_line(display, starting_x_pos + 15, starting_y_pos - 21, starting_x_pos + 15, starting_y_pos - 18, HSV_WHITE);
                qp_line(display, starting_x_pos + 16, starting_y_pos - 20, starting_x_pos + 16, starting_y_pos - 17, HSV_WHITE);
                qp_line(display, starting_x_pos + 17, starting_y_pos - 19, starting_x_pos + 17, starting_y_pos - 16, HSV_WHITE);
                qp_line(display, starting_x_pos + 18, starting_y_pos - 18, starting_x_pos + 18, starting_y_pos - 15, HSV_WHITE);
                qp_line(display, starting_x_pos + 19, starting_y_pos - 17, starting_x_pos + 19, starting_y_pos - 16, HSV_WHITE);
            }
            
            if (current_wpm >= 50) {
                //draw_line_v(starting_x_pos + 14, starting_y_pos - 25, 1, true);
                //draw_line_v(starting_x_pos + 15, starting_y_pos - 26, 3, true);
                //draw_line_v(starting_x_pos + 16, starting_y_pos - 26, 5, true);
                //draw_line_v(starting_x_pos + 17, starting_y_pos - 24, 4, true);
                //draw_line_v(starting_x_pos + 18, starting_y_pos - 22, 3, true);
                //draw_line_v(starting_x_pos + 19, starting_y_pos - 20, 2, true);
                //draw_line_v(starting_x_pos + 20, starting_y_pos - 18, 1, true);

                qp_line(display, starting_x_pos + 14, starting_y_pos - 25, starting_x_pos + 14, starting_y_pos - 24, HSV_WHITE);
                qp_line(display, starting_x_pos + 15, starting_y_pos - 26, starting_x_pos + 15, starting_y_pos - 23, HSV_WHITE);
                qp_line(display, starting_x_pos + 16, starting_y_pos - 26, starting_x_pos + 16, starting_y_pos - 21, HSV_WHITE);
                qp_line(display, starting_x_pos + 17, starting_y_pos - 24, starting_x_pos + 17, starting_y_pos - 20, HSV_WHITE);
                qp_line(display, starting_x_pos + 18, starting_y_pos - 22, starting_x_pos + 18, starting_y_pos - 19, HSV_WHITE);
                qp_line(display, starting_x_pos + 19, starting_y_pos - 20, starting_x_pos + 19, starting_y_pos - 18, HSV_WHITE);
                qp_line(display, starting_x_pos + 20, starting_y_pos - 18, starting_x_pos + 20, starting_y_pos - 17, HSV_WHITE);
            }

            if (current_wpm >= 55) {
                //draw_line_v(starting_x_pos + 18, starting_y_pos - 27, 3, true);
                //draw_line_v(starting_x_pos + 19, starting_y_pos - 27, 5, true);
                //draw_line_v(starting_x_pos + 20, starting_y_pos - 28, 8, true);
                //draw_line_v(starting_x_pos + 21, starting_y_pos - 24, 6, true);
                //draw_line_v(starting_x_pos + 22, starting_y_pos - 20, 2, true);

                qp_line(display, starting_x_pos + 18, starting_y_pos - 27, starting_x_pos + 18, starting_y_pos - 24, HSV_WHITE);
                qp_line(display, starting_x_pos + 19, starting_y_pos - 27, starting_x_pos + 19, starting_y_pos - 22, HSV_WHITE);
                qp_line(display, starting_x_pos + 20, starting_y_pos - 28, starting_x_pos + 20, starting_y_pos - 20, HSV_WHITE);
                qp_line(display, starting_x_pos + 21, starting_y_pos - 24, starting_x_pos + 21, starting_y_pos - 18, HSV_WHITE);
                qp_line(display, starting_x_pos + 22, starting_y_pos - 20, starting_x_pos + 22, starting_y_pos - 18, HSV_WHITE);
            }

            /*if (current_wpm >= 60) {
                draw_line_v(starting_x_pos + 22, starting_y_pos - 28, 4, true);
                draw_line_v(starting_x_pos + 23, starting_y_pos - 28, 8, true);
                draw_line_v(starting_x_pos + 24, starting_y_pos - 25, 6, true);
                draw_line_v(starting_x_pos + 25, starting_y_pos - 21, 2, true);
            }

            if (current_wpm >= 65) {
                for (int i = 0; i < num_of_vert_lines; i++)
                {
                    draw_line_v(starting_x_pos + 25 + (i * 3), starting_y_pos - 28, 3, true);
                    draw_line_v(starting_x_pos + 26 + (i * 3), starting_y_pos - 28, 7, true);
                    draw_line_v(starting_x_pos + 27 + (i * 3), starting_y_pos - 25, 6, true);
                    draw_line_v(starting_x_pos + 28 + (i * 3), starting_y_pos - 21, 2, true);
                }
            }

            if (current_wpm >= 140) {
                draw_line_v(starting_x_pos + 66, starting_y_pos - 28, 3, true);
                draw_line_v(starting_x_pos + 67, starting_y_pos - 28, 7, true);

                for (int i = 0; i < num_of_redlines; i++)
                {
                    draw_line_v(starting_x_pos + 68 + i, starting_y_pos - 28, 10, true);

                }
            }

            // Draw speedometer border
            draw_line_h(starting_x_pos - 5, starting_y_pos, 17, true);

            draw_line_v(starting_x_pos + 12, starting_y_pos - 3, 3, true);
            draw_line_v(starting_x_pos + 13, starting_y_pos - 6, 3, true);
            draw_line_v(starting_x_pos + 14, starting_y_pos - 9, 3, true);
            draw_line_v(starting_x_pos + 15, starting_y_pos - 11, 2, true);
            draw_line_v(starting_x_pos + 16, starting_y_pos - 13, 2, true);
            draw_line_v(starting_x_pos + 17, starting_y_pos - 14, 1, true);
            draw_line_v(starting_x_pos + 18, starting_y_pos - 15, 1, true);
            draw_line_v(starting_x_pos + 19, starting_y_pos - 16, 1, true);
            draw_line_v(starting_x_pos + 20, starting_y_pos - 17, 1, true);

            draw_line_h(starting_x_pos + 21, starting_y_pos - 18, 2, true);
            draw_line_h(starting_x_pos + 23, starting_y_pos - 19, 46, true);
            
            // Write WPM counter
            sprintf(wpm_str, "%d", current_wpm);

            if (current_wpm >= 240) {
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 3), OLED_HEIGHT / 2 + 7, "MAX", false);
            } else if (current_wpm >= 100) {
                sprintf(wpm_str, "%d", current_wpm);
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 3), OLED_HEIGHT / 2 + 7, wpm_str, false);
            } else if (current_wpm >= 10) {
                sprintf(wpm_str, "%d", current_wpm);
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 2), OLED_HEIGHT / 2 + 7, wpm_str, false);
            } else if (current_wpm > 0) {
                sprintf(wpm_str, "%d", current_wpm);
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 1), OLED_HEIGHT / 2 + 7, wpm_str, false);
            }

            write_chars_at_pixel_xy(OLED_WIDTH / 2 - (5 * 3), OLED_HEIGHT / 2 - 3, " WPM ", true);*/

            should_redraw_display = true;
            break;            
        }

        case NA2:
            draw_text_centered(display, LCD_WIDTH / 2, LCD_HEIGHT / 2, small_font, "NA2", HSV_WHITE, HSV_BLACK);
            break;

        case NA3:
            draw_text_centered(display, LCD_WIDTH / 2, LCD_HEIGHT / 2, small_font, "NA3", HSV_WHITE, HSV_BLACK);
            break;

        case NA4:
            draw_text_centered(display, LCD_WIDTH / 2, LCD_HEIGHT / 2, small_font, "NA4", HSV_WHITE, HSV_BLACK);
            break;

        case Off:
            break;

        default:
            break;
    }

    qp_flush(display);
}

// ENCODER STUFF -----------------
// Differentiate encoder short and long press
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case ENC_PUSH:
            if (record->event.pressed) {
                encoder_press_timer = timer_read32();
                encoder_is_down = true;
            } else {
                if (encoder_is_down) {
                    encoder_is_short_pressed = true;
                }

                encoder_is_down = false;
            }

            return false;

        case ENC_SEC:
            if (record->event.pressed) {
                encoder_press_timer = timer_read32();
                encoder_is_down = true;
                encoder_secondary_push_active = true;
            } else {
                if (encoder_is_down)
                {
                    encoder_is_short_pressed = true;
                }

                encoder_is_down = false;
            }

            return false;
        default:
            return true;
    }

    return process_record_user(keycode, record);
}

// Handle encoder knob rotation CW and CCW
bool encoder_update_kb(uint8_t index, bool clockwise) {
    if (!encoder_update_user(index, clockwise)) { return false; }

    qp_power(display, true);

    uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    switch(enc_mode) {
        case Select:
            if (layer == 0) { // If layer 0
                if (clockwise) {
                    increase_active_select_index(1);
                } else {
                    increase_active_select_index(-1);
                }
            } else { // If layer 1
                if (clockwise) {
                    increase_active_select_index(1);
                } else {
                    increase_active_select_index(-1);
                }
            }

            break;
        case Media:
            if (layer == 0) { // If layer 0
                if (clockwise) {
                    tap_code_delay(KC_VOLU, 10);
                } else {
                    tap_code_delay(KC_VOLD, 10);
                }
            } else { // If layer 1
                if (clockwise) {
                    tap_code(KC_MNXT);
                } else {
                    tap_code(KC_MPRV);
                }
            }

            break;
        case Window:
            if (layer == 0) { // If layer 0
                if (clockwise) {
                    if (!is_alt_tab_active) {
                        is_alt_tab_active = true;
                        register_code(KC_LALT);
                    }

                    alt_tab_timer = timer_read();
                    tap_code16(KC_TAB);
                } else {
                    if (!is_alt_tab_active) {
                        is_alt_tab_active = true;
                        register_code(KC_LALT);
                    }

                    alt_tab_timer = timer_read();
                    tap_code16(S(KC_TAB));
                }
            } else { // If layer 1{
                if (clockwise) {
                  tap_code16(C(KC_TAB));
                } else {
                  tap_code16(S(C(KC_TAB)));
                }
            }
            break;
        case Scroll:
            if (layer == 0) {
                if (clockwise) {
                    tap_code(KC_DOWN);
                } else {
                    tap_code(KC_UP);
                }
            } else {
                if (clockwise) {
                    tap_code(KC_PGDN);
                } else {
                    tap_code(KC_PGUP);
                }
            }

            break;
        case Lighting:
            if (layer == 0) {
                if (clockwise) {
                    rgblight_increase_hue();
                } else {
                    rgblight_decrease_hue();
                }
            } else {
                if (clockwise) {
                    rgblight_increase_sat();
                } else {
                    rgblight_decrease_sat();
                }
            }
            break;
    }

    housekeeping_task_user();

    return true;
}

void matrix_scan_user(void) {
    enc_state_update();

    if (is_alt_tab_active) {
        if (timer_elapsed(alt_tab_timer) > 1250) {
            unregister_code(KC_LALT);
            is_alt_tab_active = false;
        }
    }
}

// Lighting LED STUFF -----------------
#ifdef RGBLIGHT_ENABLE

const rgblight_segment_t PROGMEM my_capslock_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {32, 1, HSV_BLACK},
    {33, 14, HSV_RED},
    {0, 5, HSV_RED}
);
const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(my_capslock_layer);

void keyboard_post_init_user(void) {
    // Enable the LED layers
    rgblight_layers = my_rgb_layers;
}

bool led_update_user(led_t led_state) {
    rgblight_set_layer_state(0, led_state.caps_lock);

    return true;
}

#endif // RGBLIGHT_ENABLE