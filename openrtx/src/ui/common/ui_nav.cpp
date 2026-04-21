/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include "ui/ui_nav.h"

static Screen *navCurrent  = nullptr;
static Screen *navLastMain = nullptr;

void ui_nav_init(Screen *initial)
{
    navCurrent  = initial;
    navLastMain = initial;
}

void ui_nav_transition(Screen *next)
{
    navCurrent->exit();
    next->enter();
    navCurrent = next;
}

void    ui_nav_setLastMain(Screen *s) { navLastMain = s; }
Screen* ui_nav_current()              { return navCurrent; }
Screen* ui_nav_lastMain()             { return navLastMain; }

extern "C" void ui_terminate()
{
    if(navCurrent)
        navCurrent->exit();
}
