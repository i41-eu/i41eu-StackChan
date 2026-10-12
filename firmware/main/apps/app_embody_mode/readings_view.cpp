/*
 * SPDX-FileCopyrightText: 2026 Michal Jurosz (mj41)
 *
 * SPDX-License-Identifier: MIT
 */
#include "readings_view.h"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace embody {

static constexpr uint32_t _bg      = 0x10142A;
static constexpr uint32_t _muted   = 0x8A93B8;
static constexpr uint32_t _text    = 0xF2F4FF;
static constexpr uint32_t _track   = 0x2A3050;
static constexpr uint32_t _hot     = 0xF5B03C;  // the last input, while new
static constexpr uint32_t _zone_on = 0x2EAD6B;
static constexpr uint32_t _bar     = 0x3B82F6;
static constexpr uint32_t _ring[2] = {0x7D8CFF, 0xFF3D9A};

static constexpr int _top = 24, _step = 17, _x_label = 6, _x_value = 66;
enum Row { Leds, Touch, Accel, Gyro, Mag, Light, Prox, Head, Servo, Power, Chip, Input };
static const char* const _labels[ReadingsView::kRows] = {"LEDS",  "TOUCH", "ACCEL", "GYRO",  "MAG",  "LIGHT",
                                                         "PROX",  "HEAD",  "SERVO", "POWER", "CHIP", "INPUT"};

static lv_obj_t* box(lv_obj_t* parent, int x, int y, int w, int h, uint32_t color, int radius)
{
    lv_obj_t* o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(o, radius, 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
    return o;
}

static lv_obj_t* text(lv_obj_t* parent, int x, int y, int w, uint32_t color, const char* s)
{
    lv_obj_t* l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
    lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_width(l, w);
    lv_obj_set_pos(l, x, y);
    lv_label_set_text(l, s);
    return l;
}

void ReadingsView::create(lv_obj_t* parent)
{
    _root = lv_obj_create(parent);
    lv_obj_remove_style_all(_root);
    lv_obj_set_size(_root, 320, 240);
    lv_obj_set_pos(_root, 0, 0);
    lv_obj_set_style_bg_color(_root, lv_color_hex(_bg), 0);
    lv_obj_set_style_bg_opa(_root, LV_OPA_COVER, 0);
    lv_obj_remove_flag(_root, LV_OBJ_FLAG_SCROLLABLE);

    text(_root, _x_label, 4, 100, _muted, "Readings");
    for (int i = 0; i < kRows; i++) {
        const int y = _top + i * _step;
        text(_root, _x_label, y, 60, _muted, _labels[i]);
        int x = _x_value, w = 320 - _x_value - 6;
        if (i == Leds) {  // two strips of six dots, then volume and backlight
            for (int k = 0; k < 12; k++) {
                const int lx = _x_value + k * 13 + (k >= 6 ? 8 : 0);
                _leds[k]     = box(_root, lx, y + 3, 10, 10, _track, LV_RADIUS_CIRCLE);
            }
            x = _x_value + 12 * 13 + 14;
            w = 320 - x - 4;
        } else if (i == Prox) {
            lv_obj_t* track = box(_root, _x_value + 52, y + 5, 320 - _x_value - 52 - 10, 7, _track, 3);
            _prox_bar       = box(track, 0, 0, 0, 7, _bar, 3);
            w               = 50;
        } else if (i == Head) {
            for (int z = 0; z < 3; z++) {
                _zones[z] = box(_root, _x_value + z * 22, y + 3, 18, 11, _track, 3);
            }
            x = _x_value + 70;
            w = 320 - x - 6;
        }
        _values[i] = text(_root, x, y, w, _text, "");
        std::snprintf(_texts[i], kTextSize, "-");
        lv_label_set_text_static(_values[i], _texts[i]);
    }
    for (int f = 0; f < 2; f++) {  // where a finger is, over the readings
        _rings[f] = lv_obj_create(_root);
        lv_obj_remove_style_all(_rings[f]);
        lv_obj_set_size(_rings[f], 44, 44);
        lv_obj_set_style_radius(_rings[f], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(_rings[f], 3, 0);
        lv_obj_set_style_border_color(_rings[f], lv_color_hex(_ring[f]), 0);
        lv_obj_remove_flag(_rings[f], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(_rings[f], LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_add_flag(_root, LV_OBJ_FLAG_HIDDEN);
    _shown = false;
}

void ReadingsView::destroy()
{
    if (_root) {
        lv_obj_delete(_root);
    }
    *this = ReadingsView();
}

void ReadingsView::show(bool on)
{
    if (!_root) {
        return;
    }
    _shown = on;
    if (on) {
        lv_obj_remove_flag(_root, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(_root, LV_OBJ_FLAG_HIDDEN);
    }
}

void ReadingsView::set_value(int row, const char* text)
{
    if (std::strncmp(_texts[row], text, kTextSize - 1) != 0) {
        std::snprintf(_texts[row], kTextSize, "%s", text);
        lv_label_set_text_static(_values[row], _texts[row]);
    }
}

// Line builds a row's text in a fixed buffer (no heap).
struct Line {
    char buf[96];
    size_t n = 0;
    Line() { buf[0] = 0; }
    Line& add(const char* f, ...) __attribute__((format(printf, 2, 3)))
    {
        va_list ap;
        va_start(ap, f);
        if (n < sizeof buf) {
            int k = vsnprintf(buf + n, sizeof buf - n, f, ap);
            n = k > 0 ? std::min(sizeof buf - 1, n + (size_t)k) : n;
        }
        va_end(ap);
        return *this;
    }
    // num adds a reading, or "?" for none.
    Line& num(float v, const char* f)
    {
        return std::isnan(v) ? add("?") : add(f, v);
    }
    Line& duration(float s)
    {
        if (std::isnan(s)) {
            return add("?");
        }
        const int t = (int)s;
        return t >= 3600 ? add("%d:%02d:%02d", t / 3600, t / 60 % 60, t % 60) : add("%d:%02d", t / 60, t % 60);
    }
};

void ReadingsView::update(const Readings& r)
{
    if (!_root || !_shown) {
        return;
    }
    // Outputs: each LED in its colour as set (a dark LED shows the track colour).
    for (int k = 0; k < 12; k++) {
        if (_led_drawn && _led_colors[k] == r.leds[k]) {
            continue;
        }
        _led_colors[k] = r.leds[k];
        lv_obj_set_style_bg_color(_leds[k], lv_color_hex(r.leds[k] ? r.leds[k] : _track), 0);
    }
    _led_drawn = true;
    set_value(Leds, Line().add("v%d b%d%s", r.volume_pct, r.brightness_pct, r.sound ? " snd" : "").buf);

    // Touch: both fingers' points, and a ring where each is.
    Line touch;
    for (int f = 0; f < 2; f++) {
        if (r.touch_down[f]) {
            touch.add("%s#%d %d,%d", touch.n ? "  " : "", f, r.touch_x[f], r.touch_y[f]);
            lv_obj_set_pos(_rings[f], r.touch_x[f] - 22, r.touch_y[f] - 22);
            lv_obj_remove_flag(_rings[f], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(_rings[f], LV_OBJ_FLAG_HIDDEN);
        }
    }
    set_value(Touch, touch.n ? touch.buf : "no finger");

    if (r.imu_known) {
        set_value(Accel, Line().add("x %+.2f  y %+.2f  z %+.2f g", r.accel_g[0], r.accel_g[1], r.accel_g[2]).buf);
        set_value(Gyro, Line().add("x %+.1f  y %+.1f  z %+.1f dps", r.gyro_dps[0], r.gyro_dps[1], r.gyro_dps[2]).buf);
    }
    if (r.mag_known) {
        set_value(Mag, Line().add("x %.1f  y %.1f  z %.1f uT", r.mag_ut[0], r.mag_ut[1], r.mag_ut[2]).buf);
    }
    if (r.lux >= 0) {
        set_value(Light, Line().add(r.lux < 10 ? "%.1f lux" : "%.0f lux", r.lux).add("  ch0 %d  ch1 %d", r.light_ch0, r.light_ch1).buf);
    } else if (r.light_ch0 < 0) {
        set_value(Light, "no sensor");
    }
    const int prox = std::clamp(r.proximity, 0, 2047);
    set_value(Prox, r.proximity < 0 ? "off" : Line().add("%d", r.proximity).buf);
    lv_obj_set_width(_prox_bar, r.proximity < 0 ? 0 : std::max(1, prox * (320 - _x_value - 62) / 2047));

    for (int z = 0; z < 3; z++) {
        const int level = std::min<int>(r.head_zones[z], 3);
        if (level != _zone_levels[z]) {
            _zone_levels[z] = level;
            lv_obj_set_style_bg_color(_zones[z], lv_color_hex(level ? _zone_on : _track), 0);
            lv_obj_set_style_bg_opa(_zones[z], level ? (lv_opa_t)(110 + level * 48) : LV_OPA_COVER, 0);
        }
    }
    set_value(Head, Line().add("yaw %+.0f  pitch %.0f deg", r.yaw_deg, r.pitch_deg).buf);

    set_value(Servo, Line().add("yaw ").num(r.yaw_load_pct, "%.0f%%").add(" ").num(r.yaw_temp_c, "%.0fC")
                         .add("  pitch ").num(r.pitch_load_pct, "%.0f%%").add(" ").num(r.pitch_temp_c, "%.0fC")
                         .add("  ").num(r.servo_v, "%.1fV").buf);
    set_value(Power, Line().num(r.battery_pct, "%.0f%%").add(r.charging > 0 ? " chg  " : "  ").num(r.body_v, "%.2fV")
                         .add(" ").num(r.body_ma, "%+.0fmA").add("  usb ").num(r.vbus_v, "%.1fV").buf);
    set_value(Chip, Line().num(r.chip_c, "%.0fC").add("  ").num(r.rssi_dbm, "%.0fdBm").add("  int ")
                        .num(r.internal_kb, "%.0f").add("/").num(r.internal_block_kb, "%.0fk").add("  up ")
                        .duration(r.uptime_s).buf);

    // The last input, in amber for 3 s.
    const bool hot = !r.last_input.empty() && r.last_input_ms < 3000;
    if (r.last_input.empty()) {
        set_value(Input, "touch, pet, tap, hold a tag near...");
    } else {
        Line in;
        in.add("%s", r.last_input.c_str());
        if (!hot) {
            in.add("  %us", (unsigned)(r.last_input_ms / 1000));
        }
        set_value(Input, in.buf);
    }
    if (hot != _input_hot) {
        _input_hot = hot;
        lv_obj_set_style_text_color(_values[Input], lv_color_hex(hot ? _hot : _text), 0);
    }
}

}  // namespace embody
