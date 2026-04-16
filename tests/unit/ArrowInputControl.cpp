/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <catch2/catch_test_macros.hpp>
#include <cstring>

#include "ui/ArrowInputControl.h"
#include "ui/UIContext.h"

extern "C" {
#include "core/input.h"
}

#include "ui/ui_mod17.h"

extern layout_t layout;

// Helper: build an event_t carrying a keyboard message with the given key(s).
static event_t make_key_event(uint32_t keys)
{
    kbd_msg_t msg;
    msg.value = 0;
    msg.keys = keys;
    msg.long_press = 0;

    event_t ev;
    ev.type = EVENT_KBD;
    ev.payload = msg.value;
    return ev;
}

// ---- start() ----------------------------------------------------------------

TEST_CASE("Arrow start() clears buffer", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    memset(buf, 'X', sizeof(buf));
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    for (int i = 0; i <= 9; i++) {
        INFO("index " << i);
        REQUIRE(buf[i] == '\0');
    }
}

// ---- ENTER / ESC ------------------------------------------------------------

TEST_CASE("Arrow ENTER on empty buffer returns Confirmed", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    InputResult r = ctrl.handleKey(ctx, make_key_event(KEY_ENTER));
    REQUIRE(r == InputResult::Confirmed);
    REQUIRE(buf[0] == '\0');
}

TEST_CASE("Arrow ESC returns Cancelled", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    InputResult r = ctrl.handleKey(ctx, make_key_event(KEY_ESC));
    REQUIRE(r == InputResult::Cancelled);
}

// ---- Arrow key character selection ------------------------------------------

TEST_CASE("Arrow UP selects first symbol", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    // First UP should select symbols[1] (second symbol, since start is at 0)
    InputResult r = ctrl.handleKey(ctx, make_key_event(KEY_UP));
    REQUIRE(r == InputResult::Ongoing);
    REQUIRE(buf[0] == arrowCallsignSymbols[1]);
}

TEST_CASE("Arrow UP cycles through symbols", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    uint8_t num_symbols = strlen(arrowCallsignSymbols);

    // Press UP num_symbols times — should wrap back to first symbol
    for (uint8_t i = 0; i < num_symbols; i++)
        ctrl.handleKey(ctx, make_key_event(KEY_UP));

    REQUIRE(buf[0] == arrowCallsignSymbols[0]);
}

TEST_CASE("Arrow DOWN cycles backwards through symbols", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    // DOWN from initial position 0 should wrap to last symbol
    InputResult r = ctrl.handleKey(ctx, make_key_event(KEY_DOWN));
    REQUIRE(r == InputResult::Ongoing);

    uint8_t num_symbols = strlen(arrowCallsignSymbols);
    REQUIRE(buf[0] == arrowCallsignSymbols[num_symbols - 1]);
}

// ---- Cursor movement --------------------------------------------------------

TEST_CASE("Arrow RIGHT advances cursor", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    // Select 'A' at position 0
    ctrl.handleKey(ctx, make_key_event(KEY_UP)); // symbols[1] = 'A'

    // Move right to position 1
    ctrl.handleKey(ctx, make_key_event(KEY_RIGHT));

    // Select 'B' at position 1
    ctrl.handleKey(ctx, make_key_event(KEY_UP));
    ctrl.handleKey(ctx, make_key_event(KEY_UP)); // symbols[2] = 'B'

    REQUIRE(buf[0] == arrowCallsignSymbols[1]);
    REQUIRE(buf[1] == arrowCallsignSymbols[2]);
}

TEST_CASE("Arrow LEFT deletes and moves cursor back", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    // Type two characters
    ctrl.handleKey(ctx, make_key_event(KEY_UP)); // pos 0: symbols[1]
    ctrl.handleKey(ctx, make_key_event(KEY_RIGHT));
    ctrl.handleKey(ctx, make_key_event(KEY_UP)); // pos 1: symbols[1]

    // LEFT should delete position 1 and move cursor back
    ctrl.handleKey(ctx, make_key_event(KEY_LEFT));
    REQUIRE(buf[1] == '\0');
    REQUIRE(buf[0] == arrowCallsignSymbols[1]);
}

TEST_CASE("Arrow LEFT at position 0 does not underflow", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    // Select a character at pos 0
    ctrl.handleKey(ctx, make_key_event(KEY_UP));

    // LEFT at position 0 — should not crash or change position
    InputResult r = ctrl.handleKey(ctx, make_key_event(KEY_LEFT));
    REQUIRE(r == InputResult::Ongoing);
}

// ---- ENTER confirms after typing --------------------------------------------

TEST_CASE("Arrow ENTER confirms after typing characters", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    // Type 'A' at pos 0
    ctrl.handleKey(ctx, make_key_event(KEY_UP));
    // Move right, type 'A' at pos 1
    ctrl.handleKey(ctx, make_key_event(KEY_RIGHT));
    ctrl.handleKey(ctx, make_key_event(KEY_UP));

    InputResult r = ctrl.handleKey(ctx, make_key_event(KEY_ENTER));
    REQUIRE(r == InputResult::Confirmed);
    REQUIRE(buf[0] == arrowCallsignSymbols[1]);
    REQUIRE(buf[1] == arrowCallsignSymbols[1]);
    REQUIRE(buf[2] == '\0');
}

// ---- Max length boundary ----------------------------------------------------

TEST_CASE("Arrow input stops at max length", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[4]; // maxLen=2 → buf must be at least 3 bytes
    ctrl.start(buf, 2, arrowCallsignSymbols, FONT_SIZE_8PT);

    // Type at pos 0
    ctrl.handleKey(ctx, make_key_event(KEY_UP));
    // Move to pos 1
    ctrl.handleKey(ctx, make_key_event(KEY_RIGHT));
    // Type at pos 1
    ctrl.handleKey(ctx, make_key_event(KEY_UP));

    // RIGHT should not advance past maxLen - 1
    ctrl.handleKey(ctx, make_key_event(KEY_RIGHT));

    // UP/DOWN at the boundary should still work on the last valid position
    InputResult r = ctrl.handleKey(ctx, make_key_event(KEY_UP));
    REQUIRE(r == InputResult::Ongoing);

    // Confirm
    r = ctrl.handleKey(ctx, make_key_event(KEY_ENTER));
    REQUIRE(r == InputResult::Confirmed);
    REQUIRE(buf[2] == '\0');
}

// ---- reset() ----------------------------------------------------------------

TEST_CASE("Arrow reset() restores cursor to position 0", "[ui][input][arrow]")
{
    UIContext ctx(layout);
    ArrowInputControl ctrl;

    char buf[16];
    ctrl.start(buf, 9, arrowCallsignSymbols, FONT_SIZE_8PT);

    // Type and advance
    ctrl.handleKey(ctx, make_key_event(KEY_UP));
    ctrl.handleKey(ctx, make_key_event(KEY_RIGHT));
    ctrl.handleKey(ctx, make_key_event(KEY_UP));

    ctrl.reset();

    // After reset, UP should affect position 0 again
    ctrl.handleKey(ctx, make_key_event(KEY_UP));
    REQUIRE(buf[0] == arrowCallsignSymbols[1]);
}
