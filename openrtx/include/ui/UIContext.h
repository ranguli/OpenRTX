/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UI_CONTEXT_H
#define UI_CONTEXT_H

#ifdef CONFIG_UI_MODULE17
#include "ui/ui_mod17.h"
#else
#include "ui/ui_default.h"
#endif

/**
 * Shared context passed to InputControl and Screen methods.
 *
 * Holds a reference to the global layout and a snapshot of the
 * per-frame UI state.  Screens read and modify ui_state; the
 * caller copies it back after handleInput() / draw() returns.
 */
struct UIContext {
    const layout_t &layout;
    ui_state_t ui_state;

    explicit UIContext(const layout_t &l) : layout(l), ui_state{}
    {
    }
};

#endif // UI_CONTEXT_H
