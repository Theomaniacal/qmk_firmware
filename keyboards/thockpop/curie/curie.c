  
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
#include <string.h>
#include "curie.h"
#include "animations/spiral.c"
#include OLED_FONT_H

#define ENCODER_LONG_PRESS_TIME 300 // 0.3 second for press to count as long press
#define ANIM_SPEED_MAX 60
#define ANIM_SPEED_MIN 10
#define OLED_MENU_MODE_AMOUNT 3 // Used to represent how many OLED modes cannot be saved
#define ENC_MENU_MODE_AMOUNT 1 // Used to represent how many Encoder modes cannot be saved

static bool should_redraw_oled = true;
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

static enum encoder_modes
{
    Select,
    Media,
    Window,
    Scroll,
    Underglow
} enc_mode, saved_enc_mode;

static enum oled_modes
{
    Menu,
    OLED_Menu,
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
} oled_mode, saved_oled_mode;

// OLED STUFF -----------------
char wpm_str[10];

// OLED Draw Functions

static void draw_line_h(uint8_t x, uint8_t y, uint8_t length, bool on)
{
     for (uint8_t i = 0; i < length; i++)
     {
         oled_write_pixel(x + i, y, on);
     }
}

 static void draw_line_v(uint8_t x, uint8_t y, uint8_t length, bool on)
 {
     for (uint8_t i = 0; i < length; i++) {
         oled_write_pixel(x, y + i, on);
     }
 }

 static void draw_rect_soft(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on)
 {
    draw_line_h(x + 1, y, width - 2, on);
    draw_line_h(x + 1, y + height - 1, width - 2, on);

    draw_line_v(x, y + 1, height - 2, on);
    draw_line_v(x + width - 1, y + 1, height - 2, on);
}

void write_char_at_pixel_xy(uint8_t x, uint8_t y, const char data, bool invert)
{
    uint8_t i, j, temp;
    uint8_t cast_data = (uint8_t)data;
    
    const uint8_t *glyph = &font[((uint8_t)cast_data - OLED_FONT_START) * OLED_FONT_WIDTH];
    temp = pgm_read_byte(glyph);
    for (i = 0; i < OLED_FONT_WIDTH ; i++) {
        for (j = 0; j < OLED_FONT_HEIGHT; j++) {
            if (temp & 0x01)
            {
                oled_write_pixel(x + i, y + j, !invert);
            } 
            else
            {
                oled_write_pixel(x + i, y + j, invert);
            }

            temp >>= 1;
        }
        temp = pgm_read_byte(++glyph);
    }
}

void write_chars_at_pixel_xy(uint8_t x, uint8_t y, const char *data, bool invert)
{
    uint8_t c = data[0];
    uint8_t offset = 0;
    while (c != 0) {
        write_char_at_pixel_xy(x + offset, y, c, invert);
        data++;
        c = data[0];
        offset += 6;
    }
}


void draw_rect_filled_soft(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on)
{
    for (int i = x; i < x + width; i++) {
        if (i == x || i == (x + width - 1))
            draw_line_v(i, y + 1, height - 2, on);
        else
            draw_line_v(i, y, height, on);
    }
}

void draw_text_rectangle(uint8_t x, uint8_t y, uint8_t width, char* str, bool filled)
{
    uint8_t left_x = x - (strlen(str) * 3);
    if (filled) {
        draw_rect_filled_soft(x, y, width, 11, true);
        write_chars_at_pixel_xy(left_x + (width / 2), y + 2, str, true);
    } else {
        draw_rect_soft(x, y, width, 11, true);
        write_chars_at_pixel_xy(left_x + (width / 2), y + 2, str, false);
    }
}

void draw_text_rectangle_center(uint8_t x, uint8_t y, uint8_t width, char* str, bool filled)
{
    uint8_t left_x = x - (strlen(str) * 3);

    if (filled) {
        draw_rect_filled_soft(x - (width / 2), y, width, 11, true);
        write_chars_at_pixel_xy(left_x, y + 2, str, true);
    } else {
        draw_rect_soft(x - (width / 2), y, width, 11, true);
        write_chars_at_pixel_xy(left_x, y + 2, str, false);
    }
}

void draw_arrow(uint8_t x, uint8_t y, uint8_t size, bool forward)
{
    if (forward)
    {
        for (int i = size; i >= 0; i--)
        {
            int8_t increment = size - i;
            draw_line_v(x - (size / 2) + increment, y - (size / 2) + increment / 2, size + 1 - increment, true);
        }
    }
    else
    {
        for (int i = size; i >= 0; i--)
        {
            int8_t increment = size - i;
            draw_line_v(x + (size / 2) - increment, y - (size / 2) + increment / 2, size + 1 - increment, true);
        }
    }
}

int16_t map(int16_t x, int16_t x1, int16_t x2, int16_t y1, int16_t y2)
{
    return (x - x1) * (y2 - y1) / (x2 - x1) + y1;
}

/*
static void render_logo(void)
{
    static const char PROGMEM thockpop_logo[] = {
        31,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,192,192,192,224,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1, 31,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 96,240,248,248,248,240,224,240,224,192,  0,  0,  1,127,255,255,255,255,255,254,126,254,254,254,252,248,224,  0,192,240,248,248,252,252,124,252,252,248,248,240,192,  0,128,192,224,224,240,240,240,240,224,236,254,254,254,255,255,252,224,240,248,248,248, 60,  0,  0,  0,  0,  0,224,248,248,252,252,126,126,126,252,252,248,240,224,224,240,248,252,252,126,126,126,252,252,248,240,224,192,240,248,252,252,254,126,126,252,252,252,248,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3, 15, 31,127,255,255,255,253,248,248,248,240,223,127,127,127,127,127, 56,  7, 63, 63, 63, 63, 31, 28,  7, 31, 63, 63,127,126,124,126,127, 63, 63, 31,  7, 14, 63,255,255,255,255,243,243,243,225,193,  0,  1, 15,127,255,255,255,255,255,255,127,126,126,126,126,120,  0,255,255,255,255,255,254,126,126, 63, 63, 31, 15,  7,  7, 15, 31, 63, 63,126,126,126, 63, 63, 31, 15,  7,255,255,255,255,255,255,126,126, 63, 63, 31, 31,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
        0,248,128,128,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  3,  3,  3,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  3,  3,  3,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  7,  7,  7,  7,  7,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  7,  7,  7,  7,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
        0,128,128,128,128,248,
    };

    oled_write_raw_P(thockpop_logo, sizeof(thockpop_logo));
}
*/

static void render_animation(uint8_t frame) {
    oled_write_raw_P(spiral_anim[frame], sizeof(spiral_anim[frame]));
}

static void change_encoder_mode(enum encoder_modes mode)
{
    enc_mode = mode;
    should_redraw_oled = true;

    if (mode != Select)
    {
        saved_enc_mode = mode;
    }
}

static void change_oled_mode(enum oled_modes mode)
{
    oled_mode = mode;
    should_redraw_oled = true;

    switch (mode)
    {
        case Menu:
            max_select_index = Enc_Menu; // Last enum element lets us know the max index
            break;
        case OLED_Menu:
            max_select_index = Off - OLED_MENU_MODE_AMOUNT; // Last enum element lets us know the max index
            break;
        case Enc_Menu:
            max_select_index = Underglow - ENC_MENU_MODE_AMOUNT; // Last enum element lets us know the max index
            break;
        default:
            saved_oled_mode = mode;
            break;
    }
}

// Handle encoder presses
void enc_state_update(void)
{
    if (encoder_is_down && timer_elapsed32(encoder_press_timer) >= ENCODER_LONG_PRESS_TIME)
    {
        encoder_is_down = false;
        encoder_press_timer = timer_read32();

        encoder_is_long_pressed = true;
    }

    if (encoder_is_long_pressed)
    {
        encoder_is_long_pressed = false;
        switch (enc_mode)
        {
            case Select:
                change_encoder_mode(saved_enc_mode);
                change_oled_mode(saved_oled_mode);
                break;

            default:
                active_select_index = 0;
                change_encoder_mode(Select);
                change_oled_mode(Menu);
                
                break;
        }
    }
    else if (encoder_is_short_pressed)
    {
        encoder_is_short_pressed = false;
        switch (enc_mode)
        {
            case Select:
                switch (oled_mode)
                {
                    case Menu:
                        switch (active_select_index)
                        {
                            case 0:
                                active_select_index = saved_oled_mode - OLED_MENU_MODE_AMOUNT; // Minus 3 because the first 3 OLED modes are menus
                                change_oled_mode(OLED_Menu);
                                change_encoder_mode(Select);
                                break;
                            case 1:
                                active_select_index = saved_enc_mode - ENC_MENU_MODE_AMOUNT; // Minus 1 because the first encoder mode is Select
                                change_oled_mode(Enc_Menu);
                                change_encoder_mode(Select);
                                break;
                        }
                        break;
                    case OLED_Menu:
                        if (active_select_index != Blank - OLED_MENU_MODE_AMOUNT)
                        {
                            change_oled_mode(active_select_index + OLED_MENU_MODE_AMOUNT);
                            change_encoder_mode(saved_enc_mode);
                        }


                        /*switch (active_select_index)
                        {
                            case 0:
                                change_oled_mode(Logo);
                                change_encoder_mode(saved_enc_mode);
                                break;
                            case 1:
                                change_oled_mode(WPM);
                                change_encoder_mode(saved_enc_mode);
                                break;
                            case 2:
                                change_oled_mode(System);
                                change_encoder_mode(saved_enc_mode);
                                break;
                            case 3:
                                change_oled_mode(Weather);
                                change_encoder_mode(saved_enc_mode);
                                break;
                            case 4:
                                // Do nothing. Used so user has to move selection once more right to confirm "Display Off"
                                break;
                            case 5:
                                change_oled_mode(Off);
                                change_encoder_mode(saved_enc_mode);
                                break;
                        }*/
                        break;

                    case Enc_Menu:
                        change_encoder_mode(active_select_index + ENC_MENU_MODE_AMOUNT);
                        change_oled_mode(saved_oled_mode);
                        
                        /*switch (active_select_index)
                        {
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
                                change_encoder_mode(Underglow);
                                break;
                        }*/
                        break;

                    default:
                        break;
                }
                break;
            case Media:
                if (encoder_secondary_push_active)
                {
                    encoder_secondary_push_active = false;
                    tap_code(KC_MUTE);
                }
                else
                {
                    tap_code(KC_MPLY);
                }

                break;
            case Window:
                    if (encoder_secondary_push_active)
                    {
                        encoder_secondary_push_active = false;
                        tap_code(KC_MUTE);
                    }
                    else
                    {
                        if (is_alt_tab_active)
                        {
                            tap_code(KC_ENT);
                        }
                        else
                        {
                            tap_code(KC_MPLY);
                        }
                        
                    }
                break;
            case Scroll:
                if (encoder_secondary_push_active)
                {
                    encoder_secondary_push_active = false;
                    tap_code(KC_MUTE);
                }
                else
                {
                    tap_code(KC_MPLY);
                }
                break;
            case Underglow:
                if (encoder_secondary_push_active)
                {
                    encoder_secondary_push_active = false;
                    rgblight_step();
                }
                else
                {
                    rgblight_toggle();
                }
                break;
        }
    }
}

// Handle OLED draw
bool oled_task_kb(void)
{
    if (!should_redraw_oled) { return false; }
    should_redraw_oled = false;

    oled_clear();

    switch (oled_mode)
    {
        case Logo:
            //render_logo();
            write_chars_at_pixel_xy(OLED_WIDTH / 2 - (14 * 3), OLED_HEIGHT / 2 - 3, "Curie Southpaw", false);
            break;

        case Menu:
            draw_text_rectangle_center(2 * OLED_WIDTH / 7, 8, 46, "DISPLAY", active_select_index == 0);
            draw_text_rectangle_center(5 * OLED_WIDTH / 7, 8, 46, "ENCODER", active_select_index == 1);
            switch (saved_oled_mode)
            {
                case Logo: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (4 * 3), 19, "Logo", false); break; }
                case WPM: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (3 * 3), 19, "WPM", false); break; }
                case System: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (6 * 3), 19, "System", false); break; }
                case Weather: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (7 * 3), 19, "Weather", false); break; }
                case WPM2: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (4 * 3), 19, "WPM2", false); break; }
                case NA2: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (3 * 3), 19, "NA2", false); break; }
                case NA3: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (3 * 3), 19, "NA3", false); break; }
                case NA4: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (3 * 3), 19, "NA4", false); break; }
                case Off: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (3 * 3), 19, "Off", false); break; }
                default: { write_chars_at_pixel_xy(2 * OLED_WIDTH / 7 - (5 * 3), 19, "Error", false); break; }

            }

            switch (saved_enc_mode)
            {
                case Media: { write_chars_at_pixel_xy(5 * OLED_WIDTH / 7 - (5 * 3), 19, "Media", false); break; }
                case Window: { write_chars_at_pixel_xy(5 * OLED_WIDTH / 7 - (6 * 3), 19, "Window", false); break; }
                case Scroll: { write_chars_at_pixel_xy(5 * OLED_WIDTH / 7 - (6 * 3), 19, "Scroll", false); break; }
                case Underglow: { write_chars_at_pixel_xy(5 * OLED_WIDTH / 7 - (4 * 3), 19, "Glow", false); break; }
                default: { write_chars_at_pixel_xy(5 * OLED_WIDTH / 7 - (5 * 3), 19, "Error", false); break; }

            }

            break;

        case OLED_Menu:
            if (active_select_index < 4) // 0, 1, 2, 3
            {
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (7 * 3), 0, "Display", false);
                draw_text_rectangle_center(OLED_WIDTH / 2 - 25, 9, 48, "LOGO", active_select_index == 0);
                draw_text_rectangle_center(OLED_WIDTH / 2 + 25, 9, 48, "WPM", active_select_index == 1);
                draw_text_rectangle_center(OLED_WIDTH / 2 - 25, 21, 48, "SYSTEM", active_select_index == 2);
                draw_text_rectangle_center(OLED_WIDTH / 2 + 25, 21, 48, "WEATHER", active_select_index == 3);
                draw_arrow(OLED_WIDTH - 8, OLED_HEIGHT / 2 + 4, 6, true); // right arrow         
            }
            else if (active_select_index < 8) // 4, 5, 6, 7
            {
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (7 * 3), 0, "Display", false);
                draw_text_rectangle_center(OLED_WIDTH / 2 - 25, 9, 48, "WPM2", active_select_index == 4);
                draw_text_rectangle_center(OLED_WIDTH / 2 + 25, 9, 48, "NA2", active_select_index == 5);
                draw_text_rectangle_center(OLED_WIDTH / 2 - 25, 21, 48, "NA3", active_select_index == 6);
                draw_text_rectangle_center(OLED_WIDTH / 2 + 25, 21, 48, "NA4", active_select_index == 7);
                draw_arrow(7, OLED_HEIGHT / 2 + 4, 6, false); // left arrow
                draw_arrow(OLED_WIDTH - 8, OLED_HEIGHT / 2 + 4, 6, true); // right arrow
            }
            else // 8, 9
            {
                draw_text_rectangle_center(OLED_WIDTH / 2, 15, 74, "DISPLAY OFF", active_select_index == 9);
                draw_arrow(15, OLED_HEIGHT / 2 + 4, 6, false);  // left arrow
            }

            break;

        case Enc_Menu:
            write_chars_at_pixel_xy(OLED_WIDTH / 2 - (7 * 3), 0, "Encoder", false);
            draw_text_rectangle_center(OLED_WIDTH / 2 - 25, 9, 48, "MEDIA", active_select_index == 0);
            draw_text_rectangle_center(OLED_WIDTH / 2 + 25, 9, 48, "WINDOW", active_select_index == 1);
            draw_text_rectangle_center(OLED_WIDTH / 2 - 25, 21, 48, "SCROLL", active_select_index == 2);
            draw_text_rectangle_center(OLED_WIDTH / 2 + 25, 21, 48, "GLOW", active_select_index == 3);
            break;

        case WPM:
        {
            uint8_t current_wpm = get_current_wpm();
            uint8_t anim_speed = map(current_wpm, 0, 230, 1, 5);
            uint8_t frame_elapse = timer_read() * anim_speed / 180;
            
            render_animation(frame_elapse % 4);

            if (current_wpm >= 240)
            {
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 3), OLED_HEIGHT / 2 - 3, "MAX", false);
            }
            else if (current_wpm >= 100)
            {
                sprintf(wpm_str, "%d", current_wpm);
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 3), OLED_HEIGHT / 2 - 3, wpm_str, false);
            }
            else if (current_wpm >= 10)
            {
                sprintf(wpm_str, "%d", current_wpm);
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 2), OLED_HEIGHT / 2 - 3, wpm_str, false);
            }
            else if (current_wpm > 0)
            {
                sprintf(wpm_str, "%d", current_wpm);
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 1), OLED_HEIGHT / 2 - 3, wpm_str, false);
            }

            should_redraw_oled = true;
            break;
        }


        case System:
            draw_text_rectangle_center(OLED_WIDTH / 2, 10, 46, "System", false);
            break;

        case Weather:
            draw_text_rectangle_center(OLED_WIDTH / 2, 10, 50, "Weather", false);
            break;

        case WPM2:
        {
            uint8_t current_wpm = get_current_wpm();
            uint8_t num_of_hor_lines = current_wpm / 5;
            if (num_of_hor_lines >= 4) { num_of_hor_lines = 4; }
            uint8_t num_of_vert_lines = (current_wpm - 65) / 5;
            if (num_of_vert_lines >= 14) { num_of_vert_lines = 14; }
            uint8_t num_of_redlines = (current_wpm - 140) / 5;
            if (num_of_redlines >= 30) { num_of_redlines = 30; }
            uint8_t starting_x_pos = 20;
            uint8_t starting_y_pos = OLED_HEIGHT - 1;

            for (int i = 0; i < num_of_hor_lines; i++)
            {
                draw_line_h(starting_x_pos + i, (starting_y_pos) - (i * 3), 11, true);
                draw_line_h(starting_x_pos + 1 + i, (starting_y_pos) - 1 - (i * 3), 11, true);
            }

            if (current_wpm >= 30)
            {
                draw_line_h(starting_x_pos + 4, starting_y_pos - 12, 12, true);
                draw_line_h(starting_x_pos + 5, starting_y_pos - 13, 8, true);
                draw_line_h(starting_x_pos + 5, starting_y_pos - 14, 4, true);
            }
            
            if (current_wpm >= 35)
            {
                draw_line_h(starting_x_pos + 13, starting_y_pos - 14, 4, true);
                draw_line_h(starting_x_pos + 9, starting_y_pos - 15, 6, true);
                draw_line_h(starting_x_pos + 6, starting_y_pos - 16, 7, true);
                draw_line_h(starting_x_pos + 7, starting_y_pos - 17, 4, true);
                draw_line_h(starting_x_pos + 7, starting_y_pos - 18, 2, true);
            }

            if (current_wpm >= 40)
            {
                oled_write_pixel(starting_x_pos + 17, starting_y_pos - 15, true);
                draw_line_h(starting_x_pos + 15, starting_y_pos - 16, 2, true);
                draw_line_h(starting_x_pos + 13, starting_y_pos - 17, 3, true);
                draw_line_h(starting_x_pos + 11, starting_y_pos - 18, 4, true);
                draw_line_h(starting_x_pos + 9, starting_y_pos - 19, 5, true);
                draw_line_h(starting_x_pos + 8, starting_y_pos - 20, 4, true);
                draw_line_h(starting_x_pos + 9, starting_y_pos - 21, 2, true);
            }

            if (current_wpm >= 45)
            {
                draw_line_v(starting_x_pos + 11, starting_y_pos - 23, 2, true);
                draw_line_v(starting_x_pos + 12, starting_y_pos - 24, 4, true);
                draw_line_v(starting_x_pos + 13, starting_y_pos - 24, 4, true);
                draw_line_v(starting_x_pos + 14, starting_y_pos - 23, 4, true);
                draw_line_v(starting_x_pos + 15, starting_y_pos - 21, 3, true);
                draw_line_v(starting_x_pos + 16, starting_y_pos - 20, 3, true);
                draw_line_v(starting_x_pos + 17, starting_y_pos - 19, 3, true);
                draw_line_v(starting_x_pos + 18, starting_y_pos - 18, 3, true);
                draw_line_v(starting_x_pos + 19, starting_y_pos - 17, 1, true);
            }

            if (current_wpm >= 50)
            {
                draw_line_v(starting_x_pos + 14, starting_y_pos - 25, 1, true);
                draw_line_v(starting_x_pos + 15, starting_y_pos - 26, 3, true);
                draw_line_v(starting_x_pos + 16, starting_y_pos - 26, 5, true);
                draw_line_v(starting_x_pos + 17, starting_y_pos - 24, 4, true);
                draw_line_v(starting_x_pos + 18, starting_y_pos - 22, 3, true);
                draw_line_v(starting_x_pos + 19, starting_y_pos - 20, 2, true);
                draw_line_v(starting_x_pos + 20, starting_y_pos - 18, 1, true);
            }

            if (current_wpm >= 55)
            {
                draw_line_v(starting_x_pos + 18, starting_y_pos - 27, 3, true);
                draw_line_v(starting_x_pos + 19, starting_y_pos - 27, 5, true);
                draw_line_v(starting_x_pos + 20, starting_y_pos - 28, 8, true);
                draw_line_v(starting_x_pos + 21, starting_y_pos - 24, 6, true);
                draw_line_v(starting_x_pos + 22, starting_y_pos - 20, 2, true);
            }

            if (current_wpm >= 60)
            {
                draw_line_v(starting_x_pos + 22, starting_y_pos - 28, 4, true);
                draw_line_v(starting_x_pos + 23, starting_y_pos - 28, 8, true);
                draw_line_v(starting_x_pos + 24, starting_y_pos - 25, 6, true);
                draw_line_v(starting_x_pos + 25, starting_y_pos - 21, 2, true);
            }

            if (current_wpm >= 65)
            {
                for (int i = 0; i < num_of_vert_lines; i++)
                {
                    draw_line_v(starting_x_pos + 25 + (i * 3), starting_y_pos - 28, 3, true);
                    draw_line_v(starting_x_pos + 26 + (i * 3), starting_y_pos - 28, 7, true);
                    draw_line_v(starting_x_pos + 27 + (i * 3), starting_y_pos - 25, 6, true);
                    draw_line_v(starting_x_pos + 28 + (i * 3), starting_y_pos - 21, 2, true);
                }
            }

            if (current_wpm >= 140)
            {
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

            if (current_wpm >= 240)
            {
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 3), OLED_HEIGHT / 2 + 7, "MAX", false);
            }
            else if (current_wpm >= 100)
            {
                sprintf(wpm_str, "%d", current_wpm);
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 3), OLED_HEIGHT / 2 + 7, wpm_str, false);
            }
            else if (current_wpm >= 10)
            {
                sprintf(wpm_str, "%d", current_wpm);
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 2), OLED_HEIGHT / 2 + 7, wpm_str, false);
            }
            else if (current_wpm > 0)
            {
                sprintf(wpm_str, "%d", current_wpm);
                write_chars_at_pixel_xy(OLED_WIDTH / 2 - (3 * 1), OLED_HEIGHT / 2 + 7, wpm_str, false);
            }

            write_chars_at_pixel_xy(OLED_WIDTH / 2 - (5 * 3), OLED_HEIGHT / 2 - 3, " WPM ", true);

            should_redraw_oled = true;
            break;            
        }

        case NA2:
            draw_text_rectangle_center(OLED_WIDTH / 2, 10, 46, "NA2", false);
            break;

        case NA3:
            draw_text_rectangle_center(OLED_WIDTH / 2, 10, 46, "NA3", false);
            break;

        case NA4:
            draw_text_rectangle_center(OLED_WIDTH / 2, 10, 46, "NA4", false);
            break;

        case Off:
            break;

        default:
            break;
    }

    oled_render();

    return false;
}

void suspend_power_down_user(void)
{
    oled_off(); // Turn off OLED when computer shuts off
}

// ENCODER STUFF -----------------
// Differentiate encoder short and long press
bool process_record_kb(uint16_t keycode, keyrecord_t *record)
{
    switch (keycode)
    {
        case ENC_PUSH:
            if (record->event.pressed)
            {
                encoder_press_timer = timer_read32();
                encoder_is_down = true;
            }
            else
            {
                if (encoder_is_down)
                {
                    encoder_is_short_pressed = true;
                }

                encoder_is_down = false;
            }

            return false;

        case ENC_SEC:
            if (record->event.pressed)
            {
                encoder_press_timer = timer_read32();
                encoder_is_down = true;
                encoder_secondary_push_active = true;
            }
            else
            {
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

void increase_active_select_index(int8_t amount)
{
    should_redraw_oled = true;

    if (active_select_index + amount > max_select_index) // If active index is greater than max index, wrap the selection to first index
    {
        active_select_index = should_menu_select_wrap ? 0 : max_select_index;
    }
    else if (active_select_index + amount < 0) // If active index is less than 0, wrap the selection to last index
    {
        active_select_index = should_menu_select_wrap ? max_select_index : 0;
    }
    else
    {
        active_select_index += amount;
    }

}

// Handle encoder knob rotation CW and CCW
bool encoder_update_kb(uint8_t index, bool clockwise)
{
    if (!encoder_update_user(index, clockwise)) { return false; }

    oled_on();

    uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    switch(enc_mode)
    {
        case Select:
            if (layer == 0) // If layer 0
            {
                if (clockwise)
                {
                    increase_active_select_index(1);
                }
                else
                {
                    increase_active_select_index(-1);
                }
            }
            else // If layer 1
            {
                if (clockwise)
                {
                    increase_active_select_index(1);
                }
                else
                {
                    increase_active_select_index(-1);
                }
            }

            break;
        case Media:
            if (layer == 0) // If layer 0
            {
                if (clockwise)
                {
                    tap_code_delay(KC_VOLU, 10);
                }
                else
                {
                    tap_code_delay(KC_VOLD, 10);
                }
            }
            else // If layer 1
            {
                if (clockwise)
                {
                    tap_code(KC_MNXT);
                }
                else
                {
                    tap_code(KC_MPRV);
                }
            }

            break;
        case Window:
            if (layer == 0) // If layer 0
            {
                if (clockwise)
                {
                  if (!is_alt_tab_active)
                  {
                    is_alt_tab_active = true;
                    register_code(KC_LALT);
                  }

                  alt_tab_timer = timer_read();
                  tap_code16(KC_TAB);
                }
                else
                {
                  if (!is_alt_tab_active)
                  {
                    is_alt_tab_active = true;
                    register_code(KC_LALT);
                  }

                  alt_tab_timer = timer_read();
                  tap_code16(S(KC_TAB));
                }
            }
            else // If layer 1
            {
                if (clockwise)
                {
                  tap_code16(C(KC_TAB));
                }
                else
                {
                  tap_code16(S(C(KC_TAB)));
                }
            }
            break;
        case Scroll:
            if (layer == 0)
            {
                if (clockwise)
                {
                    tap_code(KC_DOWN);
                }
                else
                {
                    tap_code(KC_UP);
                }
            }
            else
            {
                if (clockwise)
                {
                    tap_code(KC_PGDN);
                }
                else
                {
                    tap_code(KC_PGUP);
                }
            }

            break;
        case Underglow:
            if (layer == 0)
            {
                if (clockwise)
                {
                    rgblight_increase_hue();
                }
                else
                {
                    rgblight_decrease_hue();
                }
            }
            else
            {
                if (clockwise)
                {
                    rgblight_increase_sat();
                }
                else
                {
                    rgblight_decrease_sat();
                }
            }
            break;
    }

    return true;
}

void keyboard_post_init_kb(void)
{
    enc_mode = Media;
    saved_enc_mode = enc_mode;

    oled_mode = WPM2;
    saved_oled_mode = oled_mode;
}

void matrix_scan_user(void)
{
    enc_state_update();

    if (is_alt_tab_active)
    {
        if (timer_elapsed(alt_tab_timer) > 1250)
        {
            unregister_code(KC_LALT);
            is_alt_tab_active = false;
        }
    }
}

// UNDERGLOW LED STUFF -----------------
#ifdef RGBLIGHT_ENABLE

const rgblight_segment_t PROGMEM my_capslock_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {47, 10, HSV_RED}, // Light 10 LEDs, starting with LED 47
    {5, 1, HSV_OFF},
    {7, 1, HSV_OFF}
);
const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(my_capslock_layer);

void keyboard_post_init_user(void)
{
    // Enable the LED layers
    rgblight_layers = my_rgb_layers;
}

bool led_update_user(led_t led_state)
{
    rgblight_set_layer_state(0, led_state.caps_lock);

    return true;
}

#endif // RGBLIGHT_ENABLE