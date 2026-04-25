/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#ifdef __cplusplus

#include "ui/DisplayLayout.h"

// Designated initializers are a C99/C++20 feature used here as a GCC/Clang
// extension to make layout definitions self-documenting without comments.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

namespace DisplayLayout {

// 128x64 display: Radioddity GD-77, DM-1801, MD-9600, Module17
constexpr Params params = {
    .heights = {
        .top        = 11,
        .top_pad    = 1,
        .line1      = 10,
        .line2      = 10,
        .line3      = 10,
        .line3_large = 16,
        .line4      = 10,
        .menu       = 10,
        .bottom     = 15,
        .bottom_pad = 0,
    },
    .padding = {
        .status_v     = 1,
        .small_line_v = 1,
        .big_line_v   = 0,
        .horizontal   = 4,
        .text_descent = 1,
    },
    .screen_h = 64,
    .fonts = {
        .top        = {FONT_SIZE_6PT,  SYMBOLS_SIZE_6PT},
        .line1      = {FONT_SIZE_6PT,  SYMBOLS_SIZE_6PT},
        .line2      = {FONT_SIZE_6PT,  SYMBOLS_SIZE_6PT},
        .line3      = {FONT_SIZE_6PT,  SYMBOLS_SIZE_6PT},
        .line3_large = FONT_SIZE_10PT,
        .line4      = {FONT_SIZE_6PT,  SYMBOLS_SIZE_6PT},
        .bottom     = FONT_SIZE_6PT,
        .input      = FONT_SIZE_8PT,
        .menu       = FONT_SIZE_6PT,
        .message    = FONT_SIZE_6PT,
    },
};

#pragma GCC diagnostic pop

static_assert(CONFIG_SCREEN_HEIGHT > 63 && CONFIG_SCREEN_HEIGHT < 128,
              "Wrong layout for this screen height!");

constexpr Metrics selected_layout = make_metrics(params);

} // namespace DisplayLayout

#endif // __cplusplus
