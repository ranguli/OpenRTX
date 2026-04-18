/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UI_NAV_H
#define UI_NAV_H

#include "ui/Screen.hpp"

/*
 * Shared navigation state for both UI variants (default and module17).
 * Holds the active screen pointer and the "last main screen" used for
 * back-navigation from menus.  Both variants call ui_nav_init() at startup
 * and use ui_nav_transition() instead of a file-static transition() helper.
 */

void    ui_nav_init(Screen *initial);
void    ui_nav_transition(Screen *next);
void    ui_nav_setLastMain(Screen *s);
Screen* ui_nav_current();
Screen* ui_nav_lastMain();

#endif /* UI_NAV_H */
