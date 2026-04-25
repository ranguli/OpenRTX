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

// 160x128 display: Tytera MD380, MD-UV380, CS7000, CS7000-PLUS, DM-1701
constexpr Params params = {
    .heights = {
        .top        = 16,
        .top_pad    = 4,
        .line1      = 20,
        .line2      = 20,
        .line3      = 20,
        .line3_large = 40,
        .line4      = 20,
        .menu       = 16,
        .bottom     = 23,
        .bottom_pad = 4,
    },
    .padding = {
        .status_v     = 2,
        .small_line_v = 2,
        .big_line_v   = 6,
        .horizontal   = 4,
        .text_descent = 1,
    },
    .screen_h = 128,
    .fonts = {
        .top        = {FONT_SIZE_8PT,  SYMBOLS_SIZE_8PT},
        .line1      = {FONT_SIZE_8PT,  SYMBOLS_SIZE_8PT},
        .line2      = {FONT_SIZE_8PT,  SYMBOLS_SIZE_8PT},
        .line3      = {FONT_SIZE_8PT,  SYMBOLS_SIZE_8PT},
        .line3_large = FONT_SIZE_16PT,
        .line4      = {FONT_SIZE_8PT,  SYMBOLS_SIZE_8PT},
        .bottom     = FONT_SIZE_8PT,
        .input      = FONT_SIZE_12PT,
        .menu       = FONT_SIZE_8PT,
        .message    = FONT_SIZE_6PT,
    },
};

#pragma GCC diagnostic pop

static_assert(CONFIG_SCREEN_HEIGHT > 127, "Wrong layout for this screen height!");

constexpr Metrics selected_layout = make_metrics(params);

} // namespace DisplayLayout

#endif // __cplusplus
