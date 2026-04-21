/*
Copyright 2021-2022 Alin M Elena <alinm.elena@gmail.com>

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

#include "quantum.h"


#define LAYOUT( \
    L00,    L01,    L02,  L03,  L04, L05,                R51,  R52,  R53,  R54 ,R55,  R56, \
    L10,    L11,    L12,  L13,  L14, L15, L16,    R60,   R61,  R62,  R63,  R64, R65,  R66, \
    L20,    L21,    L22,  L23,  L24 ,L25, L26,    R70,   R71,  R72,  R73,  R74, R75,  R76, \
    L30,    L31,    L32,  L33,  L34 ,L35 ,L36,    R80,   R81,  R82,  R83,  R84, R85,  R86, \
                          L43,  L44 ,L45 ,               R91,  R92,  R93 \
) { \
    { L00,    L01,    L02,  L03,  L04, L05, KC_NO }, \
    { L10,    L11,    L12,  L13,  L14, L15, L16 }, \
    { L20,    L21,    L22,  L23,  L24, L25, L26}, \
    { L30,    L31,    L32,  L33,  L34, L35, L36}, \
    { KC_NO,  KC_NO,  KC_NO,L43,  L44, L45, KC_NO}, \
    { KC_NO,  R51,    R52,  R53,  R54, R55, R56}, \
    { R60,    R61,    R62,  R63,  R64, R65, R66}, \
    { R70,    R71,    R72,  R73,  R74, R75, R76}, \
    { R80,    R81,    R82,  R83,  R84, R85, R86}, \
    { KC_NO,  R91,    R92,  R93,  KC_NO, KC_NO, KC_NO }, \
}



