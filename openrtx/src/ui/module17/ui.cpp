/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <stdint.h>
#include "ui/ui_mod17.h"
#include "rtx/rtx.h"
#include "interfaces/platform.h"
#include "interfaces/display.h"
#include "interfaces/cps_io.h"
#include "interfaces/nvmem.h"
#include "interfaces/delays.h"
#include <string.h>
#include "core/battery.h"
#include "core/input.h"
#include "hwconfig.h"
#include "ui/ui_events.h"
#include "ui/ui_strings.h"
#include "ui/Screen.hpp"
#include "ui/ui_draw.h"
#include "ui/ui_nav.h"

// module17-variant-specific draw functions and helpers
extern "C" {
void _ui_drawMainTop();                          // no-arg variant for Module17
void _ui_drawSettingsModule17(ui_state_t* ui_state);
void state_resetSettingsAndVfo();
} // extern "C" module17-specific

const char *menu_items[] =
{
    "Settings",
#ifdef CONFIG_GPS
    "GPS",
#endif
    "Info",
    "About",
    "Shutdown"
};

const char *settings_items[] =
{
    "Display",
#ifdef CONFIG_RTC
    "Time & Date",
#endif
#ifdef CONFIG_GPS
    "GPS",
#endif
    "M17",
    "Module 17",
    "Default Settings"
};

const char *display_items[] =
{
    "Brightness",
};

const char *m17_items[] =
{
    "Callsign",
    "Meta Txt",
    "CAN",
    "CAN RX Check"
};

const char *module17_items[] =
{
    "Mic Gain",
    "PTT In",
    "PTT Out",
    "TX Phase",
    "RX Phase",
    "TX Softpot",
    "RX Softpot"
};

#ifdef CONFIG_GPS
const char *settings_gps_items[] =
{
    "GPS Enabled",
    "GPS Set Time",
    "UTC Timezone"
};
#endif

const char *info_items[] =
{
    "",
    "Used heap",
    "Hw Version",
    "HMI",
    "BB Tuning Pot",
};

const char *authors[] =
{
    "Niccolo' IU2KIN",
    "Silvano IU2KWO",
    "Federico IU2NUO",
    "Mathis DB9MAT",
    "Morgan ON4MOD",
    "Marco DM4RCO"
};

static const char symbols_callsign[] = "_ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890/-.";

// Calculate number of menu entries
const uint8_t menu_num = sizeof(menu_items)/sizeof(menu_items[0]);
const uint8_t settings_num = sizeof(settings_items)/sizeof(settings_items[0]);
const uint8_t display_num = sizeof(display_items)/sizeof(display_items[0]);
#ifdef CONFIG_GPS
const uint8_t settings_gps_num = sizeof(settings_gps_items)/sizeof(settings_gps_items[0]);
#endif
const uint8_t m17_num = sizeof(m17_items)/sizeof(m17_items[0]);
const uint8_t module17_num = sizeof(module17_items)/sizeof(module17_items[0]);
const uint8_t info_num = sizeof(info_items)/sizeof(info_items[0]);
const uint8_t author_num = sizeof(authors)/sizeof(authors[0]);

const color_t color_black = {0, 0, 0, 255};
const color_t color_grey = {60, 60, 60, 255};
const color_t color_white = {255, 255, 255, 255};
const color_t yellow_fab413 = {250, 180, 19, 255};

state_t last_state;
static ui_state_t ui_state;
static bool   *gSyncRtx = nullptr;

// Forward declarations of static helpers used in Screen::handleInput bodies
static void _ui_changeBrightness(int variation);
static void _ui_changeCAN(int variation);
static void _ui_changeWiper(uint16_t *wiper, int variation);
static void _ui_changeMicGain(int variation);
static void _ui_menuUp(uint8_t menu_entries);
static void _ui_menuDown(uint8_t menu_entries);
static void _ui_menuBack(Screen *prev);
static void _ui_textInputReset(char *buf);
static void _ui_textInputArrows(char *buf, uint8_t max_len, kbd_msg_t msg);
static void _ui_textInputConfirm(char *buf);

static constexpr layout_t _ui_calculateLayout()
{
    // Horizontal line height
    const uint16_t hline_h = 1;
    // Compensate for fonts printing below the start position
    const uint16_t text_v_offset = 1;

    // Calculate UI layout for the Module 17

    // Height and padding shown in diagram at beginning of file
    const uint16_t top_h = 11;
    const uint16_t top_pad = 1;
    const uint16_t line1_h = 10;
    const uint16_t line2_h = 10;
    const uint16_t line3_h = 10;
    const uint16_t line4_h = 10;
    const uint16_t line5_h = 10;
    const uint16_t menu_h = 10;
    const uint16_t bottom_h = 15;
    const uint16_t bottom_pad = 0;
    const uint16_t status_v_pad = 1;
    const uint16_t small_line_v_pad = 1;
    const uint16_t big_line_v_pad = 0;
    const uint16_t horizontal_pad = 4;

    // Top bar font: 6 pt
    const fontSize_t   top_font = FONT_SIZE_6PT;
    const symbolSize_t top_symbol_size = SYMBOLS_SIZE_6PT;
    // Middle line fonts: 5, 8, 8 pt
    const fontSize_t line1_font = FONT_SIZE_6PT;
    const symbolSize_t line1_symbol_size = SYMBOLS_SIZE_6PT;
    const fontSize_t line2_font = FONT_SIZE_6PT;
    const symbolSize_t line2_symbol_size = SYMBOLS_SIZE_6PT;
    const fontSize_t line3_font = FONT_SIZE_6PT;
    const symbolSize_t line3_symbol_size = SYMBOLS_SIZE_6PT;
    const fontSize_t line4_font = FONT_SIZE_6PT;
    const symbolSize_t line4_symbol_size = SYMBOLS_SIZE_6PT;
    const fontSize_t line5_font = FONT_SIZE_6PT;
    const symbolSize_t line5_symbol_size = SYMBOLS_SIZE_6PT;
    // Bottom bar font: 6 pt
    const fontSize_t bottom_font = FONT_SIZE_6PT;
    // TimeDate/Frequency input font
    const fontSize_t input_font = FONT_SIZE_8PT;
    // Menu font
    const fontSize_t menu_font = FONT_SIZE_6PT;
    // Message font
    const fontSize_t message_font = FONT_SIZE_6PT;
    // Mode screen frequency font: 9 pt
    const fontSize_t mode_font_big = FONT_SIZE_9PT;
    // Mode screen details font: 6 pt
    const fontSize_t mode_font_small = FONT_SIZE_6PT;

    // Calculate printing positions
    point_t top_pos    = {horizontal_pad, top_h - status_v_pad - text_v_offset};
    point_t line1_pos  = {horizontal_pad, top_h + top_pad + line1_h - small_line_v_pad - text_v_offset};
    point_t line2_pos  = {horizontal_pad, top_h + top_pad + line1_h + line2_h - small_line_v_pad - text_v_offset};
    point_t line3_pos  = {horizontal_pad, top_h + top_pad + line1_h + line2_h + line3_h - big_line_v_pad - text_v_offset};
    point_t line4_pos  = {horizontal_pad, top_h + top_pad + line1_h + line2_h + line3_h + line4_h - big_line_v_pad - text_v_offset};
    point_t line5_pos  = {horizontal_pad, top_h + top_pad + line1_h + line2_h + line3_h + line4_h + line5_h - big_line_v_pad - text_v_offset};
    point_t bottom_pos = {horizontal_pad, CONFIG_SCREEN_HEIGHT - bottom_pad - status_v_pad - text_v_offset};

    layout_t new_layout =
    {
        hline_h,
        top_h,
        line1_h,
        line2_h,
        line3_h,
        line4_h,
        line5_h,
        menu_h,
        bottom_h,
        bottom_pad,
        status_v_pad,
        horizontal_pad,
        text_v_offset,
        top_pos,
        line1_pos,
        line2_pos,
        line3_pos,
        line4_pos,
        line5_pos,
        bottom_pos,
        top_font,
        top_symbol_size,
        line1_font,
        line1_symbol_size,
        line2_font,
        line2_symbol_size,
        line3_font,
        line3_symbol_size,
        line4_font,
        line4_symbol_size,
        line5_font,
        line5_symbol_size,
        bottom_font,
        input_font,
        menu_font,
        message_font,
        mode_font_big,
        mode_font_small
    };
    return new_layout;
}

const layout_t layout = _ui_calculateLayout();

// ---- Screen subclasses ----

class VfoScreen : public Screen {
public:
    bool handleInput(event_t ev) override;
    void render() override { _ui_drawMainVFO(&ui_state); }
};

class MenuTopScreen : public Screen {
public:
    bool handleInput(event_t ev) override;
    void render() override { _ui_drawMenuTop(&ui_state); }
};

class MenuSettingsScreen : public Screen {
public:
    bool handleInput(event_t ev) override;
    void render() override { _ui_drawMenuSettings(&ui_state); }
};

class MenuInfoScreen : public Screen {
public:
    bool handleInput(event_t ev) override;
    void render() override { _ui_drawMenuInfo(&ui_state); }
};

class MenuAboutScreen : public Screen {
public:
    bool handleInput(event_t ev) override;
    void render() override { _ui_drawMenuAbout(&ui_state); }
};

class SettingsDisplayScreen : public Screen {
public:
    bool handleInput(event_t ev) override;
    void render() override { _ui_drawSettingsDisplay(&ui_state); }
};

class SettingsM17Screen : public Screen {
public:
    bool handleInput(event_t ev) override;
    void render() override { _ui_drawSettingsM17(&ui_state); }
};

class SettingsModule17Screen : public Screen {
public:
    bool handleInput(event_t ev) override;
    void render() override { _ui_drawSettingsModule17(&ui_state); }
};

class SettingsReset2DefaultsScreen : public Screen {
public:
    bool handleInput(event_t ev) override;
    void render() override { _ui_drawSettingsReset2Defaults(&ui_state); }
};

static VfoScreen                    vfoScreen;
static MenuTopScreen                menuTopScreen;
static MenuSettingsScreen           menuSettingsScreen;
static MenuInfoScreen               menuInfoScreen;
static MenuAboutScreen              menuAboutScreen;
static SettingsDisplayScreen        settingsDisplayScreen;
static SettingsM17Screen            settingsM17Screen;
static SettingsModule17Screen       settingsModule17Screen;
static SettingsReset2DefaultsScreen settingsReset2DefaultsScreen;

static void transition(Screen *next)
{
    ui_nav_transition(next);
    ui_state.menu_selected = 0;
}

// ---- handleInput implementations ----

bool VfoScreen::handleInput(event_t ev)
{
    if(ev.type != EVENT_KBD) return true;
    kbd_msg_t msg;
    msg.value = ev.payload;

    if(ui_state.edit_mode)
    {
        if(msg.keys & KEY_ENTER)
        {
            _ui_textInputConfirm(ui_state.new_callsign);
            strncpy(state.settings.m17_dest, ui_state.new_callsign, 10);
            *gSyncRtx = true;
            ui_state.edit_mode = false;
        }
        else if(msg.keys & KEY_ESC)
            ui_state.edit_mode = false;
        else
            _ui_textInputArrows(ui_state.new_callsign, 9, msg);
    }
    else if(ui_state.edit_message)
    {
        if(msg.keys & KEY_ENTER)
        {
            _ui_textInputConfirm(ui_state.new_message);
            strncpy(state.settings.M17_meta_text, ui_state.new_message, 52);
            ui_state.edit_message = false;
            *gSyncRtx = true;
        }
        else if(msg.keys & KEY_ESC)
            ui_state.edit_message = false;
        else
            _ui_textInputArrows(ui_state.new_message, 52, msg);
    }
    else
    {
        if(msg.keys & KEY_ENTER)
        {
            ui_nav_setLastMain(ui_nav_current());
            transition(&menuTopScreen);
        }
        else if(msg.keys & KEY_RIGHT)
            ui_state.edit_mode = true;
    }
    return true;
}

bool MenuTopScreen::handleInput(event_t ev)
{
    if(ev.type != EVENT_KBD) return true;
    kbd_msg_t msg;
    msg.value = ev.payload;

    if(msg.keys & KEY_UP || msg.keys & KNOB_LEFT)
        _ui_menuUp(menu_num);
    else if(msg.keys & KEY_DOWN || msg.keys & KNOB_RIGHT)
        _ui_menuDown(menu_num);
    else if(msg.keys & KEY_ENTER)
    {
        switch(ui_state.menu_selected)
        {
            case M_SETTINGS: transition(&menuSettingsScreen); break;
            case M_INFO:     transition(&menuInfoScreen);     break;
            case M_ABOUT:    transition(&menuAboutScreen);    break;
            case M_SHUTDOWN: state.devStatus = SHUTDOWN;      break;
        }
    }
    else if(msg.keys & KEY_ESC)
        _ui_menuBack(ui_nav_lastMain());
    return true;
}

bool MenuSettingsScreen::handleInput(event_t ev)
{
    if(ev.type != EVENT_KBD) return true;
    kbd_msg_t msg;
    msg.value = ev.payload;

    if(msg.keys & KEY_UP || msg.keys & KNOB_LEFT)
        _ui_menuUp(settings_num);
    else if(msg.keys & KEY_DOWN || msg.keys & KNOB_RIGHT)
        _ui_menuDown(settings_num);
    else if(msg.keys & KEY_ENTER)
    {
        switch(ui_state.menu_selected)
        {
            case S_DISPLAY:        transition(&settingsDisplayScreen);        break;
            case S_M17:            transition(&settingsM17Screen);            break;
            case S_MOD17:          transition(&settingsModule17Screen);       break;
            case S_RESET2DEFAULTS: transition(&settingsReset2DefaultsScreen); break;
            default:               break;
        }
    }
    else if(msg.keys & KEY_ESC)
        _ui_menuBack(&menuTopScreen);
    return true;
}

bool MenuInfoScreen::handleInput(event_t ev)
{
    if(ev.type != EVENT_KBD) return true;
    kbd_msg_t msg;
    msg.value = ev.payload;

    if(msg.keys & KEY_UP || msg.keys & KNOB_LEFT)
        _ui_menuUp(info_num);
    else if(msg.keys & KEY_DOWN || msg.keys & KNOB_RIGHT)
        _ui_menuDown(info_num);
    else if(msg.keys & KEY_ESC)
        _ui_menuBack(&menuTopScreen);
    return true;
}

bool MenuAboutScreen::handleInput(event_t ev)
{
    if(ev.type != EVENT_KBD) return true;
    kbd_msg_t msg;
    msg.value = ev.payload;

    if(msg.keys & KEY_UP || msg.keys & KNOB_LEFT)
    {
        if(ui_state.menu_selected > 0)
            ui_state.menu_selected -= 1;
    }
    else if(msg.keys & KEY_DOWN || msg.keys & KNOB_RIGHT)
        ui_state.menu_selected += 1;
    else if(msg.keys & KEY_ESC)
        _ui_menuBack(&menuTopScreen);
    return true;
}

bool SettingsDisplayScreen::handleInput(event_t ev)
{
    if(ev.type != EVENT_KBD) return true;
    kbd_msg_t msg;
    msg.value = ev.payload;

    if(msg.keys & KEY_LEFT)
    {
        switch(ui_state.menu_selected)
        {
            case D_BRIGHTNESS: _ui_changeBrightness(-5); break;
            default: break;
        }
    }
    else if(msg.keys & KEY_RIGHT)
    {
        switch(ui_state.menu_selected)
        {
            case D_BRIGHTNESS: _ui_changeBrightness(+5); break;
            default: break;
        }
    }
    else if(msg.keys & KEY_UP || msg.keys & KNOB_LEFT)
        _ui_menuUp(display_num);
    else if(msg.keys & KEY_DOWN || msg.keys & KNOB_RIGHT)
        _ui_menuDown(display_num);
    else if(msg.keys & KEY_ESC)
    {
        nvm_writeSettings(&state.settings);
        _ui_menuBack(&menuSettingsScreen);
    }
    return true;
}

bool SettingsM17Screen::handleInput(event_t ev)
{
    if(ev.type != EVENT_KBD) return true;
    kbd_msg_t msg;
    msg.value = ev.payload;

    if(ui_state.edit_mode)
    {
        if(msg.keys & KEY_ENTER)
        {
            _ui_textInputConfirm(ui_state.new_callsign);
            strncpy(state.settings.callsign, ui_state.new_callsign, 10);
            ui_state.edit_mode = false;
        }
        else if(msg.keys & KEY_ESC)
            ui_state.edit_mode = false;
        else
            _ui_textInputArrows(ui_state.new_callsign, 9, msg);
    }
    else if(ui_state.edit_message)
    {
        if(msg.keys & KEY_ENTER)
        {
            _ui_textInputConfirm(ui_state.new_message);
            strncpy(state.settings.M17_meta_text, ui_state.new_message, 52);
            ui_state.edit_message = false;
            ui_state.edit_mode = false;
        }
        else if(msg.keys & KEY_ESC)
            ui_state.edit_message = false;
        else
            _ui_textInputArrows(ui_state.new_message, 52, msg);
    }
    else
    {
        if(msg.keys & KEY_LEFT)
        {
            switch(ui_state.menu_selected)
            {
                case M_CAN:    _ui_changeCAN(-1);                                   break;
                case M_CAN_RX: state.settings.m17_can_rx = !state.settings.m17_can_rx; break;
                default: break;
            }
        }
        else if(msg.keys & KEY_RIGHT)
        {
            switch(ui_state.menu_selected)
            {
                case M_CAN:    _ui_changeCAN(+1);                                   break;
                case M_CAN_RX: state.settings.m17_can_rx = !state.settings.m17_can_rx; break;
                default: break;
            }
        }
        else if(msg.keys & KEY_ENTER)
        {
            switch(ui_state.menu_selected)
            {
                case M_CALLSIGN:
                    ui_state.edit_mode = true;
                    _ui_textInputReset(ui_state.new_callsign);
                    break;
                case M_METATEXT:
                    ui_state.edit_message = true;
                    _ui_textInputReset(ui_state.new_message);
                    break;
                default: break;
            }
        }
        else if(msg.keys & KEY_UP || msg.keys & KNOB_LEFT)
            _ui_menuUp(m17_num);
        else if(msg.keys & KEY_DOWN || msg.keys & KNOB_RIGHT)
            _ui_menuDown(m17_num);
        else if(msg.keys & KEY_ESC)
        {
            *gSyncRtx = true;
            nvm_writeSettings(&state.settings);
            _ui_menuBack(&menuSettingsScreen);
        }
    }
    return true;
}

bool SettingsModule17Screen::handleInput(event_t ev)
{
    if(ev.type != EVENT_KBD) return true;
    kbd_msg_t msg;
    msg.value = ev.payload;

    if(msg.keys & KEY_LEFT)
    {
        switch(ui_state.menu_selected)
        {
            case D_TXWIPER:    _ui_changeWiper(&mod17CalData.tx_wiper, -1); break;
            case D_RXWIPER:    _ui_changeWiper(&mod17CalData.rx_wiper, -1); break;
            case D_TXINVERT:   mod17CalData.bb_tx_invert -= 1;              break;
            case D_RXINVERT:   mod17CalData.bb_rx_invert -= 1;              break;
            case D_MICGAIN:    _ui_changeMicGain(-1);                       break;
            case D_PTTINLEVEL: mod17CalData.ptt_in_level -= 1;              break;
            case D_PTTOUTLEVEL:mod17CalData.ptt_out_level -= 1;             break;
            default: break;
        }
    }
    else if(msg.keys & KEY_RIGHT)
    {
        switch(ui_state.menu_selected)
        {
            case D_TXWIPER:    _ui_changeWiper(&mod17CalData.tx_wiper, +1); break;
            case D_RXWIPER:    _ui_changeWiper(&mod17CalData.rx_wiper, +1); break;
            case D_TXINVERT:   mod17CalData.bb_tx_invert += 1;              break;
            case D_RXINVERT:   mod17CalData.bb_rx_invert += 1;              break;
            case D_MICGAIN:    _ui_changeMicGain(+1);                       break;
            case D_PTTINLEVEL: mod17CalData.ptt_in_level += 1;              break;
            case D_PTTOUTLEVEL:mod17CalData.ptt_out_level += 1;             break;
            default: break;
        }
    }
    else if(msg.keys & KEY_UP || msg.keys & KNOB_LEFT)
        _ui_menuUp(module17_num);
    else if(msg.keys & KEY_DOWN || msg.keys & KNOB_RIGHT)
        _ui_menuDown(module17_num);
    else if(msg.keys & KEY_ESC)
    {
        nvm_writeSettings(&state.settings);
        _ui_menuBack(&menuSettingsScreen);
    }
    return true;
}

bool SettingsReset2DefaultsScreen::handleInput(event_t ev)
{
    if(ev.type != EVENT_KBD) return true;
    kbd_msg_t msg;
    msg.value = ev.payload;

    if(!ui_state.edit_mode)
    {
        if(msg.keys & KEY_ENTER)
            ui_state.edit_mode = true;
        else if(msg.keys & KEY_ESC)
            _ui_menuBack(&menuSettingsScreen);
    }
    else
    {
        if(msg.keys & KEY_ENTER)
        {
            ui_state.edit_mode = false;
            mod17CalData.tx_wiper     = 0x080;
            mod17CalData.rx_wiper     = 0x080;
            mod17CalData.bb_tx_invert = 0;
            mod17CalData.bb_rx_invert = 0;
            mod17CalData.mic_gain     = 0;
            state_resetSettingsAndVfo();
            nvm_writeSettings(&state.settings);
            _ui_menuBack(&menuSettingsScreen);
        }
        else if(msg.keys & KEY_ESC)
        {
            ui_state.edit_mode = false;
            _ui_menuBack(&menuSettingsScreen);
        }
    }
    return true;
}

// ---- end Screen subclasses ----

void ui_init()
{
    ui_state = ui_state_t{};
    ui_nav_init(&vfoScreen);

    // Refresh translatable menu strings from the active language table.
    // Hardware-specific strings ("Shutdown", "Module 17", etc.) stay hardcoded.
    menu_items[M_SETTINGS] = currentLanguage->settings;
#ifdef CONFIG_GPS
    menu_items[M_GPS]      = currentLanguage->gps;
#endif
    menu_items[M_INFO]     = currentLanguage->info;
    menu_items[M_ABOUT]    = currentLanguage->about;

    settings_items[S_DISPLAY]        = currentLanguage->display;
#ifdef CONFIG_RTC
    settings_items[S_TIMEDATE]       = currentLanguage->timeAndDate;
#endif
#ifdef CONFIG_GPS
    settings_items[S_GPS]            = currentLanguage->gpsSettings;
#endif
    settings_items[S_M17]            = currentLanguage->m17;
    settings_items[S_RESET2DEFAULTS] = currentLanguage->defaultSettings;

    display_items[D_BRIGHTNESS] = currentLanguage->brightness;

#ifdef CONFIG_GPS
    settings_gps_items[G_ENABLED]  = currentLanguage->gpsEnabled;
    settings_gps_items[G_SET_TIME] = currentLanguage->gpsSetTime;
    settings_gps_items[G_TIMEZONE] = currentLanguage->UTCTimeZone;
#endif

    m17_items[M_CALLSIGN] = currentLanguage->callsign;
    m17_items[M_METATEXT] = currentLanguage->metaText;
    m17_items[M_CAN]      = currentLanguage->CAN;
    m17_items[M_CAN_RX]   = currentLanguage->canRxCheck;

    info_items[1] = currentLanguage->usedHeap;

    authors[0] = currentLanguage->Niccolo;
    authors[1] = currentLanguage->Silvano;
    authors[2] = currentLanguage->Federico;
}

void ui_drawSplashScreen()
{
    gfx_clearScreen();

    point_t origin = {0, (CONFIG_SCREEN_HEIGHT / 2) - 6};
    gfx_print(origin, FONT_SIZE_12PT, TEXT_ALIGN_CENTER, yellow_fab413, "O P N\nR T X");
}

#ifdef CONFIG_RTC
void _ui_timedate_add_digit(datetime_t *timedate, uint8_t pos, uint8_t number)
{
    switch(pos)
    {
        // Set date
        case 1:
            timedate->date += number * 10;
            break;
        case 2:
            timedate->date += number;
            break;
        // Set month
        case 3:
            timedate->month += number * 10;
            break;
        case 4:
            timedate->month += number;
            break;
        // Set year
        case 5:
            timedate->year += number * 10;
            break;
        case 6:
            timedate->year += number;
            break;
        // Set hour
        case 7:
            timedate->hour += number * 10;
            break;
        case 8:
            timedate->hour += number;
            break;
        // Set minute
        case 9:
            timedate->minute += number * 10;
            break;
        case 10:
            timedate->minute += number;
            break;
    }
}
#endif

static void _ui_changeBrightness(int variation)
{
    // Avoid rollover if current value is zero.
    if((state.settings.brightness == 0) && (variation < 0))
        return;

    // Cap max brightness to 100
    if((state.settings.brightness == 100) && (variation > 0))
        return;

    state.settings.brightness += variation;
    display_setBacklightLevel(state.settings.brightness);
}

static void _ui_changeCAN(int variation)
{
    int8_t can = state.settings.m17_can + variation;

    // M17 CAN ranges from 0 to 15
    if(can > 15)
        can = 0;

    if(can < 0)
        can = 15;

    state.settings.m17_can = can;
}

static void _ui_changeWiper(uint16_t *wiper, int variation)
{
    uint16_t value = *wiper;
    value         += variation;

    // Max value for softpot is 0x100, min value is set to 0x001
    if(value > 0x100)
        value = 0x100;

    if(value < 0x001)
        value = 0x001;

    *wiper = value;
}

static void _ui_changeMicGain(int variation)
{
    int8_t gain = mod17CalData.mic_gain + variation;

    if(gain > 2)
        gain = 0;

    if(gain < 0)
        gain = 2;

    mod17CalData.mic_gain = gain;
}

static void _ui_menuUp(uint8_t menu_entries)
{
    uint8_t maxEntries = menu_entries - 1;
    const hwInfo_t* hwinfo = platform_getHwInfo();

    // Hide the "shutdown" main menu entry for versions lower than 0.1e
    if((hwinfo->hw_version < 1) && (ui_nav_current() == (Screen*)&menuTopScreen))
        maxEntries -= 1;

    // Hide the softpot menu entries if hardware does not have them
    uint8_t softpot = hwinfo->flags & MOD17_FLAGS_SOFTPOT;
    if((softpot == 0) && (ui_nav_current() == (Screen*)&settingsModule17Screen))
        maxEntries -= 2;

    if(ui_state.menu_selected > 0)
        ui_state.menu_selected -= 1;
    else
        ui_state.menu_selected = maxEntries;
}

static void _ui_menuDown(uint8_t menu_entries)
{
   uint8_t maxEntries = menu_entries - 1;
   const hwInfo_t* hwinfo = platform_getHwInfo();

    // Hide the "shutdown" main menu entry for versions lower than 0.1e
    if((hwinfo->hw_version < 1) && (ui_nav_current() == (Screen*)&menuTopScreen))
        maxEntries -= 1;

    // Hide the softpot menu entries if hardware does not have them
    uint8_t softpot = hwinfo->flags & MOD17_FLAGS_SOFTPOT;
    if((softpot == 0) && (ui_nav_current() == (Screen*)&settingsModule17Screen))
        maxEntries -= 2;

    if(ui_state.menu_selected < maxEntries)
        ui_state.menu_selected += 1;
    else
        ui_state.menu_selected = 0;
}

static void _ui_menuBack(Screen *prev)
{
    if(ui_state.edit_mode)
        ui_state.edit_mode = false;
    else
        transition(prev);
}

static void _ui_textInputReset(char *buf)
{
    ui_state.input_number = 0;
    ui_state.input_position = 0;
    ui_state.input_set = 0;
    ui_state.last_keypress = 0;
    memset(buf, 0, 9);
}

static void _ui_textInputArrows(char *buf, uint8_t max_len, kbd_msg_t msg)
{
    if(ui_state.input_position >= max_len)
        return;

    uint8_t num_symbols = 0;
    num_symbols = strlen(symbols_callsign);

    if (msg.keys & KEY_RIGHT)
    {
        if (ui_state.input_position < (max_len - 1))
        {
            ui_state.input_position = ui_state.input_position + 1;
            ui_state.input_set = 0;
        }
    }
    else if (msg.keys & KEY_LEFT)
    {
        if (ui_state.input_position > 0)
        {
            buf[ui_state.input_position] = '\0';
            ui_state.input_position = ui_state.input_position - 1;
        }

        // get index of current selected character in symbol table
        ui_state.input_set = strcspn(symbols_callsign, &buf[ui_state.input_position]);
    }
    else if (msg.keys & KEY_UP)
        ui_state.input_set = (ui_state.input_set + 1) % num_symbols;
    else if (msg.keys & KEY_DOWN)
        ui_state.input_set = ui_state.input_set==0 ? num_symbols-1 : ui_state.input_set-1;

    buf[ui_state.input_position] = symbols_callsign[ui_state.input_set];
}

static void _ui_textInputConfirm(char *buf)
{
    buf[ui_state.input_position + 1] = '\0';
}

void ui_saveState()
{
    last_state = state;
}

void ui_updateFSM(bool *sync_rtx)
{
    event_t event;
    if(!ui_popEvent(&event)) return;

    gSyncRtx = sync_rtx;
    ui_nav_current()->handleInput(event);
}

bool ui_updateGUI()
{
    ui_nav_current()->render();

    return true;
}

