/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UI_EVENTS_H
#define UI_EVENTS_H

#include "core/event.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Pop one event from the UI event queue.
 *
 * @param ev: output parameter filled with the popped event.
 * @return true if an event was available, false if the queue was empty.
 */
bool ui_popEvent(event_t *ev);

#ifdef __cplusplus
}
#endif

#endif /* UI_EVENTS_H */
