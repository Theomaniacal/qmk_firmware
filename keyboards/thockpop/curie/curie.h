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

#pragma once

#include "quantum.h"

#define LAYOUT_southpaw_all( \
    K00,           K51,     K02, K52, K03, K53, K04, K54, K05, K55, K06, K56, K07, K57, K08, K58, K09, \
         K60, K11, K61,     K12, K62, K13, K63, K14, K64, K15, K65, K16, K66, K17, K67, K18, K68, K19, \
    K20, K70, K21, K71,     K22, K72, K23, K73, K24, K74, K25, K75, K26, K76, K27, K77,      K78, K29, \
         K80, K31, K81,     K32,      K33, K83, K34, K84, K35, K85, K36, K86, K37, K87, K38, K88, K39, \
    K40, K90, K41,          K42, K92, K43,                K45,           K96,      K97, K48, K98, K49  \
) { \
    { K00,   KC_NO, K02,   K03,   K04,   K05,   K06,   K07,   K08,   K09   }, \
    { KC_NO, K11,   K12,   K13,   K14,   K15,   K16,   K17,   K18,   K19   }, \
    { K20,   K21,   K22,   K23,   K24,   K25,   K26,   K27,   KC_NO, K29   }, \
    { KC_NO, K31,   K32,   K33,   K34,   K35,   K36,   K37,   K38,   K39   }, \
    { K40,   K41,   K42,   K43,   KC_NO, K45,   KC_NO, KC_NO, K48,   K49   }, \
    { KC_NO, K51,   K52,   K53,   K54,   K55,   K56,   K57,   K58,   KC_NO }, \
    { K60,   K61,   K62,   K63,   K64,   K65,   K66,   K67,   K68,   KC_NO }, \
    { K70,   K71,   K72,   K73,   K74,   K75,   K76,   K77,   K78,   KC_NO }, \
    { K80,   K81,   KC_NO, K83,   K84,   K85,   K86,   K87,   K88,   KC_NO }, \
    { K90,   KC_NO, K92,   KC_NO, KC_NO, KC_NO, K96,   K97,   K98,   KC_NO }  \
}

enum my_keycodes {
    ENC_SEC = SAFE_RANGE,
    ENC_PUSH
};