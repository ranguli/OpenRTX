/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "core/event.h"

/**
 * Abstract base class for UI screens.
 *
 * Mirrors the OpMode hierarchy used in rtx/: concrete screens are
 * stack-allocated static instances; dispatch goes through a Screen* pointer.
 * No RTTI, no exceptions, no dynamic_cast — consistent with the rest of the
 * embedded codebase.
 */
class Screen
{
public:
    virtual void enter() {}
    virtual void exit()  {}
    virtual bool handleInput(event_t ev) = 0;
    virtual void render() = 0;
    virtual ~Screen() = default;
};
