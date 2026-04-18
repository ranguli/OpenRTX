/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include "core/event.h"
#include "core/ui.h"
#include "ui/ui_common.h"
#include "ui/ui_events.h"

namespace {
    uint8_t evQueue_rdPos = 0;
    uint8_t evQueue_wrPos = 0;
    event_t evQueue[MAX_NUM_EVENTS];
}

extern "C" {

bool ui_pushEvent(const uint8_t type, const uint32_t data)
{
    uint8_t newHead = (evQueue_wrPos + 1) % MAX_NUM_EVENTS;

    if(newHead == evQueue_rdPos) return false;

    event_t event;
    event.type    = type;
    event.payload = data;

    evQueue[evQueue_wrPos] = event;
    evQueue_wrPos = newHead;

    return true;
}

bool ui_popEvent(event_t *ev)
{
    if(evQueue_wrPos == evQueue_rdPos) return false;

    uint8_t newTail = (evQueue_rdPos + 1) % MAX_NUM_EVENTS;
    *ev           = evQueue[evQueue_rdPos];
    evQueue_rdPos = newTail;

    return true;
}

} // extern "C"
