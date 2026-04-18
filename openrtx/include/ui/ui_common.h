/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UI_COMMON_H
#define UI_COMMON_H

#include "core/graphics.h"
#include <stdbool.h>
#include <stdint.h>

// Maximum menu entry length
#define MAX_ENTRY_LEN 21
// Frequency digits
#define FREQ_DIGITS 7
// Time & Date digits
#define TIMEDATE_DIGITS 10
// Max number of UI events
#define MAX_NUM_EVENTS 16

enum SetRxTx
{
    SET_RX = 0,
    SET_TX
};

enum backupRestoreItems
{
    BR_BACKUP = 0,
    BR_RESTORE
};

#ifdef __cplusplus
extern "C" {
#endif

extern const color_t color_black;
extern const color_t color_grey;
extern const color_t color_white;
extern const color_t yellow_fab413;

#ifdef __cplusplus
}
#endif

#endif /* UI_COMMON_H */
