/*
 * SPDX-FileCopyrightText: 2026 Michal Jurosz (mj41)
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <lvgl.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

namespace embody {

// What the readings screen shows: the robot's own raw readings and outputs, filled in by the app.
// NaN (or a *_known false) means "no reading".
struct Readings {
    // Outputs: the 12 LEDs as set (#rrggbb, left 0-5 then right 6-11), volume, backlight.
    std::array<uint32_t, 12> leds{};
    int volume_pct     = -1;
    int brightness_pct = -1;
    bool sound         = false;  // the speaker plays

    // Inputs.
    bool touch_down[2] = {};
    int touch_x[2]     = {}, touch_y[2] = {};
    bool imu_known     = false;
    float accel_g[3]   = {};
    float gyro_dps[3]  = {};
    bool mag_known     = false;
    float mag_ut[3]    = {};
    float lux          = -1;  // < 0: no light sensor or no reading yet
    int light_ch0 = -1, light_ch1 = -1;
    int proximity     = -1;  // 0..2047; < 0: off or no sensor
    std::array<uint8_t, 3> head_zones{};  // 0..3, front to back
    float yaw_deg = 0, pitch_deg = 0;

    // Slower readings: the last telemetry (every 2 s); NaN when not there.
    float yaw_load_pct = NAN, pitch_load_pct = NAN, yaw_temp_c = NAN, pitch_temp_c = NAN, servo_v = NAN;
    float battery_pct = NAN, charging = NAN, body_v = NAN, body_ma = NAN, vbus_v = NAN;
    float chip_c = NAN, rssi_dbm = NAN, heap_kb = NAN, uptime_s = NAN;
    float internal_kb = NAN, internal_block_kb = NAN;  // free internal RAM, and its largest block (TLS needs one)

    std::string last_input;     // the last input event, e.g. "head press 3 2 0"
    uint32_t last_input_ms = 0;  // how long ago
};

// The readings screen: one row per sensor, in the order of the robot's parts (as on
// s.i41.eu/s3d: screen, motion sensor, light and proximity, head touch, servos, power,
// processor), the LEDs as they are lit, the last input, and rings where fingers touch.
// An app chooses it (Raw data: "view" or Accepted {view}); it lies over the face and under
// pictures and sprites. Everything here runs under the LVGL lock.
class ReadingsView {
public:
    void create(lv_obj_t* parent);
    void destroy();
    lv_obj_t* root() const { return _root; }
    void show(bool on);
    bool shown() const { return _shown; }
    void update(const Readings& r);

    static constexpr int kRows = 12;

private:
    lv_obj_t* _root = nullptr;
    bool _shown     = false;
    lv_obj_t* _values[kRows] = {};
    // Each row's text, set with lv_label_set_text_static: an update allocates nothing (small
    // allocations churning internal RAM starved TLS of its buffers, 2026-10-12).
    static constexpr int kTextSize = 80;
    char _texts[kRows][kTextSize] = {};
    lv_obj_t* _leds[12] = {};
    uint32_t _led_colors[12] = {};
    bool _led_drawn = false;
    lv_obj_t* _zones[3] = {};
    int _zone_levels[3] = {-1, -1, -1};
    lv_obj_t* _prox_bar = nullptr;
    lv_obj_t* _rings[2] = {};
    std::string _last_input;
    bool _input_hot = false;

    void set_value(int row, const char* text);
};

}  // namespace embody
