/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the COPYRIGHT file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#ifndef __COLORS_H__
#define __COLORS_H__

// Colour for normal text
#define TXT_COL_R 112
#define TXT_COL_G 39
#define TXT_COL_B 0

// Colour for selected text in list windows
#define SEL_COL_R 255
#define SEL_COL_G 255
#define SEL_COL_B 255

// Colour for highlighted text in list windows
#define HIL_COL_R ((TXT_COL_R+2*255)/3)
#define HIL_COL_G ((TXT_COL_G+2*255)/3)
#define HIL_COL_B ((TXT_COL_B+2*255)/3)

// colour for partially solved levels
#define SO1_COL_R 221
#define SO1_COL_G 179
#define SO1_COL_B 0

// colour for solved levels
#define SOL_COL_R ((TXT_COL_R+ 70)/2)
#define SOL_COL_G ((TXT_COL_G+200)/2)
#define SOL_COL_B ((TXT_COL_B+ 70)/2)

// the colour for the help text and the arrows in the help window
#define HLP_COL_R 221
#define HLP_COL_G 179
#define HLP_COL_B 0

// colour for separator in list window
#define SEP_COL_R 120
#define SEP_COL_G 90
#define SEP_COL_B 60

// background color (windows boxes)
#define BG_COL_R 168
#define BG_COL_G 120
#define BG_COL_B 80

// transparency of tutorial background
#define TUT_COL_A 210

// text color for time when time is over
#define TIME_COL_R 255
#define TIME_COL_G 0
#define TIME_COL_B 0

// cursor color foreground (level editor)
#define CURS_FG_COL_R 255
#define CURS_FG_COL_G 255
#define CURS_FG_COL_B 255

// cursor color background (level editor)
#define CURS_BG_COL_R 127
#define CURS_BG_COL_G 127
#define CURS_BG_COL_B 127

#endif

