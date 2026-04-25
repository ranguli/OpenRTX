/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

/*
 * The graphical user interface (GUI) works by splitting the screen in
 * horizontal rows, with row height depending on vertical resolution.
 *
 * The general screen layout is composed by an upper status bar at the
 * top of the screen and a lower status bar at the bottom.
 * The central portion of the screen is filled by two big text/number rows
 * And a small row.
 *
 * Below is shown the row height for two common display densities.
 *
 *        160x128 display (MD380)            Recommended font size
 *      ┌─────────────────────────┐
 *      │  top_status_bar (16px)  │  8 pt (11 px) font with 2 px vertical padding
 *      │      top_pad (4px)      │  4 px padding
 *      │      Line 1 (20px)      │  8 pt (11 px) font with 4 px vertical padding
 *      │      Line 2 (20px)      │  8 pt (11 px) font with 4 px vertical padding
 *      │                         │
 *      │      Line 3 (40px)      │  16 pt (xx px) font with 6 px vertical padding
 *      │ RSSI+squelch bar (20px) │  20 px
 *      │      bottom_pad (4px)   │  4 px padding
 *      └─────────────────────────┘
 *
 *         128x64 display (GD-77)
 *      ┌─────────────────────────┐
 *      │  top_status_bar (11 px) │  6 pt (9 px) font with 1 px vertical padding
 *      │      top_pad (1px)      │  1 px padding
 *      │      Line 1 (10px)      │  6 pt (9 px) font without vertical padding
 *      │      Line 2 (10px)      │  6 pt (9 px) font with 2 px vertical padding
 *      │      Line 3 (18px)      │  12 pt (xx px) font with 0 px vertical padding
 *      │ RSSI+squelch bar (11px) │  11 px
 *      │      bottom_pad (1px)   │  1 px padding
 *      └─────────────────────────┘
 *
 *         128x48 display (RD-5R)
 *      ┌─────────────────────────┐
 *      │  top_status_bar (11 px) │  6 pt (9 px) font with 1 px vertical padding
 *      ├─────────────────────────┤  1 px line
 *      │      Line 2 (10px)      │  8 pt (11 px) font with 4 px vertical padding
 *      │      Line 3 (18px)      │  8 pt (11 px) font with 4 px vertical padding
 *      └─────────────────────────┘
 */

#include <cstdint>
#include "core/graphics.h"

#ifndef __cplusplus
#error This header is C++ only!
#endif

namespace DisplayLayout {

/**
 * Per-resolution parameters used to construct a Metrics object.
 */
struct Params {
    struct Heights {
        uint16_t top;
        uint16_t top_pad;
        uint16_t line1;
        uint16_t line2;
        uint16_t line3;
        uint16_t line3_large;
        uint16_t line4;
        uint16_t menu;
        uint16_t bottom;
        uint16_t bottom_pad;
    };

    struct Padding {
        uint16_t status_v;
        uint16_t small_line_v;
        uint16_t big_line_v;
        uint16_t horizontal;
        uint16_t text_descent; // compensate for fonts printing below the start position
    };

    struct LineFonts {
        fontSize_t   size;
        symbolSize_t symbols;
    };

    struct Fonts {
        LineFonts  top;
        LineFonts  line1;
        LineFonts  line2;
        LineFonts  line3;
        fontSize_t line3_large;
        LineFonts  line4;
        fontSize_t bottom;
        fontSize_t input;
        fontSize_t menu;
        fontSize_t message;
    };

    Heights  heights;
    Padding  padding;
    uint16_t screen_h;
    Fonts    fonts;
};

/**
 * Fully computed UI layout: row heights, text positions, and fonts.
 * Replaces the C layout_t struct.
 */
struct Metrics {
    uint16_t hline_h;
    uint16_t top_h;
    uint16_t line1_h;
    uint16_t line2_h;
    uint16_t line3_h;
    uint16_t line3_large_h;
    uint16_t line4_h;
    uint16_t menu_h;
    uint16_t bottom_h;
    uint16_t bottom_pad;
    uint16_t status_v_pad;
    uint16_t horizontal_pad;
    uint16_t text_v_offset;
    point_t top_pos;
    point_t line1_pos;
    point_t line2_pos;
    point_t line3_pos;
    point_t line3_large_pos;
    point_t line4_pos;
    point_t bottom_pos;
    fontSize_t   top_font;
    symbolSize_t top_symbol_size;
    fontSize_t   line1_font;
    symbolSize_t line1_symbol_size;
    fontSize_t   line2_font;
    symbolSize_t line2_symbol_size;
    fontSize_t   line3_font;
    symbolSize_t line3_symbol_size;
    fontSize_t   line3_large_font;
    fontSize_t   line4_font;
    symbolSize_t line4_symbol_size;
    fontSize_t   bottom_font;
    fontSize_t   input_font;
    fontSize_t   menu_font;
    fontSize_t   message_font;
};

constexpr Metrics make_metrics(const Params& p)
{
    Metrics m{};

    // Pass-through dimensions
    m.hline_h        = 1;
    m.top_h          = p.heights.top;
    m.line1_h        = p.heights.line1;
    m.line2_h        = p.heights.line2;
    m.line3_h        = p.heights.line3;
    m.line3_large_h  = p.heights.line3_large;
    m.line4_h        = p.heights.line4;
    m.menu_h         = p.heights.menu;
    m.bottom_h       = p.heights.bottom;
    m.bottom_pad     = p.heights.bottom_pad;
    m.status_v_pad   = p.padding.status_v;
    m.horizontal_pad = p.padding.horizontal;
    m.text_v_offset  = p.padding.text_descent;

    // Text baseline positions: bottom edge of each row, adjusted for font
    // descent and per-row vertical padding.
    // body_top is where the line rows begin, below the top bar and its padding.
    const int16_t x        = static_cast<int16_t>(p.padding.horizontal);
    const int16_t descent  = static_cast<int16_t>(p.padding.text_descent);
    const int16_t sv       = static_cast<int16_t>(p.padding.status_v);
    const int16_t slvp     = static_cast<int16_t>(p.padding.small_line_v);
    const int16_t blvp     = static_cast<int16_t>(p.padding.big_line_v);
    const int16_t body_top = static_cast<int16_t>(p.heights.top + p.heights.top_pad);

    m.top_pos         = {x, static_cast<int16_t>(p.heights.top - sv - descent)};
    m.line1_pos       = {x, static_cast<int16_t>(body_top + p.heights.line1 - slvp - descent)};
    m.line2_pos       = {x, static_cast<int16_t>(body_top + p.heights.line1 + p.heights.line2 - slvp - descent)};
    m.line3_pos       = {x, static_cast<int16_t>(body_top + p.heights.line1 + p.heights.line2 + p.heights.line3 - slvp - descent)};
    m.line3_large_pos = {x, static_cast<int16_t>(body_top + p.heights.line1 + p.heights.line2 + p.heights.line3_large - blvp - descent)};
    m.line4_pos       = {x, static_cast<int16_t>(body_top + p.heights.line1 + p.heights.line2 + p.heights.line3 + p.heights.line4 - slvp - descent)};
    m.bottom_pos      = {x, static_cast<int16_t>(p.screen_h - p.heights.bottom_pad - sv - descent)};

    // Fonts
    m.top_font          = p.fonts.top.size;
    m.top_symbol_size   = p.fonts.top.symbols;
    m.line1_font        = p.fonts.line1.size;
    m.line1_symbol_size = p.fonts.line1.symbols;
    m.line2_font        = p.fonts.line2.size;
    m.line2_symbol_size = p.fonts.line2.symbols;
    m.line3_font        = p.fonts.line3.size;
    m.line3_symbol_size = p.fonts.line3.symbols;
    m.line3_large_font  = p.fonts.line3_large;
    m.line4_font        = p.fonts.line4.size;
    m.line4_symbol_size = p.fonts.line4.symbols;
    m.bottom_font       = p.fonts.bottom;
    m.input_font        = p.fonts.input;
    m.menu_font         = p.fonts.menu;
    m.message_font      = p.fonts.message;

    return m;
}

} // namespace DisplayLayout
