/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <pthread.h>
#include "core/event.h"
#include "core/ui.h"
#include "ui/ui_common.h"
#include "ui/ui_events.h"

namespace {
    uint8_t evQueue_rdPos = 0;
    uint8_t evQueue_wrPos = 0;
    event_t evQueue[MAX_NUM_EVENTS];
    pthread_mutex_t evQueue_mutex = PTHREAD_MUTEX_INITIALIZER;
}

extern "C" {

bool ui_pushEvent(const uint8_t type, const uint32_t data)
{
    pthread_mutex_lock(&evQueue_mutex);

    uint8_t newHead = (evQueue_wrPos + 1) % MAX_NUM_EVENTS;
    bool ok = (newHead != evQueue_rdPos);

    if(ok)
    {
        event_t event;
        event.type    = type;
        event.payload = data;
        evQueue[evQueue_wrPos] = event;
        evQueue_wrPos = newHead;
    }

    pthread_mutex_unlock(&evQueue_mutex);
    return ok;
}

bool ui_popEvent(event_t *ev)
{
    pthread_mutex_lock(&evQueue_mutex);

    bool ok = (evQueue_wrPos != evQueue_rdPos);
    if(ok)
    {
        uint8_t newTail = (evQueue_rdPos + 1) % MAX_NUM_EVENTS;
        *ev           = evQueue[evQueue_rdPos];
        evQueue_rdPos = newTail;
    }

    pthread_mutex_unlock(&evQueue_mutex);
    return ok;
}

} // extern "C"
