/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UI_DRAW_H
#define UI_DRAW_H

/*
 * Forward declarations for all _ui_draw* functions implemented in ui_main.c
 * and ui_menu.c.  These are C functions called from C++ translation units, so
 * the extern "C" linkage specification is mandatory.
 *
 * Each variant (default, module17) includes this header and declares only its
 * variant-specific draw functions in its own extern "C" block.
 */

// ui_state_t is defined in the variant header (ui_default.h or ui_mod17.h).
// A forward declaration is sufficient here since all parameters are pointers.
struct ui_state_t;

#ifdef __cplusplus
extern "C" {
#endif

/* ---- ui_main.c ---- */
void _ui_drawMainBackground();
void _ui_drawVFOMiddle();
void _ui_drawMEMMiddle();
void _ui_drawVFOBottom();
void _ui_drawMEMBottom();
void _ui_drawMainVFO(ui_state_t *ui_state);
void _ui_drawMainVFOInput(ui_state_t *ui_state);
void _ui_drawMainMEM(ui_state_t *ui_state);

/* ---- ui_menu.c ---- */
void _ui_drawMenuTop(ui_state_t *ui_state);
void _ui_drawMenuSettings(ui_state_t *ui_state);
void _ui_drawMenuInfo(ui_state_t *ui_state);
void _ui_drawMenuAbout(ui_state_t *ui_state);
void _ui_drawSettingsDisplay(ui_state_t *ui_state);
void _ui_drawSettingsM17(ui_state_t *ui_state);
void _ui_drawSettingsReset2Defaults(ui_state_t *ui_state);
bool _ui_drawMacroMenu(ui_state_t *ui_state);

#ifdef CONFIG_GPS
void _ui_drawMenuGPS();
void _ui_drawSettingsGPS(ui_state_t *ui_state);
#endif

#ifdef CONFIG_RTC
void _ui_drawSettingsTimeDate();
void _ui_drawSettingsTimeDateSet(ui_state_t *ui_state);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* UI_DRAW_H */
