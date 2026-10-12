/*
 * SPDX-FileCopyrightText: 2026 Michal Jurosz (mj41)
 *
 * SPDX-License-Identifier: MIT
 *
 * Embody Mode's readings screen on the host (tests/sim): the real ReadingsView and the real
 * LVGL render into memory; the test checks pixels. SNAPSHOT_DIR=/tmp/x also writes the screens
 * as PPM files to look at.
 */
#include <apps/app_embody_mode/readings_view.h>
#include <lvgl.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int g_failures = 0;

void flush(lv_display_t* disp, const lv_area_t*, uint8_t*)
{
    lv_display_flush_ready(disp);
}

struct Rgb {
    int r, g, b;
};

struct Screen {
    lv_draw_buf_t* buf = nullptr;
    explicit Screen(const char* name)
    {
        lv_refr_now(nullptr);
        buf = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_ARGB8888);
        if (const char* dir = std::getenv("SNAPSHOT_DIR"); dir && buf) {
            FILE* f = std::fopen((std::string(dir) + "/" + name + ".ppm").c_str(), "wb");
            std::fprintf(f, "P6 %d %d 255\n", (int)buf->header.w, (int)buf->header.h);
            for (int y = 0; y < (int)buf->header.h; y++) {
                for (int x = 0; x < (int)buf->header.w; x++) {
                    const Rgb c = at(x, y);
                    std::fputc(c.r, f), std::fputc(c.g, f), std::fputc(c.b, f);
                }
            }
            std::fclose(f);
        }
    }
    ~Screen() { lv_draw_buf_destroy(buf); }
    Rgb at(int x, int y) const
    {
        const uint8_t* p = buf->data + y * buf->header.stride + x * 4;  // B, G, R, A
        return {p[2], p[1], p[0]};
    }
};

void expect(bool ok, const std::string& what)
{
    if (!ok) {
        std::cerr << "FAIL: " << what << "\n";
        g_failures++;
    }
}

bool near(Rgb c, int r, int g, int b)
{
    return std::abs(c.r - r) < 24 && std::abs(c.g - g) < 24 && std::abs(c.b - b) < 24;
}

}  // namespace

int main()
{
    lv_init();
    lv_tick_set_cb([]() { return (uint32_t)0; });
    lv_display_t* disp = lv_display_create(320, 240);
    static uint8_t draw_buf[320 * 240 * 2];
    lv_display_set_buffers(disp, draw_buf, nullptr, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(disp, flush);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), 0);  // white: the face below

    embody::ReadingsView view;
    view.create(lv_screen_active());
    {
        Screen s("readings_hidden");
        expect(near(s.at(160, 120), 255, 255, 255), "hidden at start: the face shows");
    }

    embody::Readings r;
    r.leds[0]  = 0xFF0000;  // left strip, at the screen end
    r.leds[11] = 0x00FF00;  // right strip, at the screen end
    r.volume_pct = 50, r.brightness_pct = 80;
    r.touch_down[0] = true, r.touch_x[0] = 250, r.touch_y[0] = 150;
    r.imu_known = true, r.accel_g[2] = 1.0f, r.gyro_dps[0] = 0.4f;
    r.mag_known = true, r.mag_ut[0] = 12.3f, r.mag_ut[1] = -4.5f, r.mag_ut[2] = 40.1f;
    r.lux = 123, r.light_ch0 = 456, r.light_ch1 = 78, r.proximity = 1024;
    r.head_zones = {3, 1, 0};
    r.yaw_deg = 12, r.pitch_deg = 40;
    r.yaw_load_pct = 3, r.pitch_load_pct = 5, r.yaw_temp_c = 31, r.pitch_temp_c = 33, r.servo_v = 6.9f;
    r.battery_pct = 87, r.charging = 1, r.body_v = 4.02f, r.body_ma = -120, r.vbus_v = 5.0f;
    r.chip_c = 45, r.rssi_dbm = -52, r.heap_kb = 123, r.uptime_s = 3723;
    r.last_input = "head press 3 1 0", r.last_input_ms = 500;
    view.show(true);
    view.update(r);
    {
        Screen s("readings");
        expect(near(s.at(4, 120), 0x10, 0x14, 0x2A), "shown: the dark background");
        expect(near(s.at(66 + 5, 24 + 8), 255, 0, 0), "LED 0 red, first dot");
        expect(near(s.at(66 + 11 * 13 + 8 + 5, 24 + 8), 0, 255, 0), "LED 11 green, last dot");
        expect(near(s.at(66 + 1 * 13 + 5, 24 + 8), 0x2A, 0x30, 0x50), "LED 1 dark: the track colour");
        const Rgb ring = s.at(250, 129);  // the finger's ring, its top edge
        expect(ring.b > 200 && ring.r > 100, "a ring where the finger is");
        const int head_y = 24 + 7 * 17 + 8;
        expect(s.at(66 + 9, head_y).g > 120, "head zone 0 lit");
        expect(near(s.at(66 + 44 + 9, head_y), 0x2A, 0x30, 0x50), "head zone 2 dark");
        const int prox_y = 24 + 6 * 17 + 8;
        expect(near(s.at(66 + 52 + 20, prox_y), 0x3B, 0x82, 0xF6), "proximity bar filled at its start");
        expect(near(s.at(310 - 15, prox_y), 0x2A, 0x30, 0x50), "proximity bar half empty at its end");
    }

    r.touch_down[0] = false;
    r.last_input_ms = 9000;
    view.update(r);
    {
        Screen s("readings_later");
        const Rgb ring = s.at(250, 129);
        expect(!(ring.b > 200 && ring.r > 100), "no ring once the finger is up");
    }

    view.show(false);
    {
        Screen s("readings_off");
        expect(near(s.at(160, 120), 255, 255, 255), "hidden again: the face shows");
    }
    view.destroy();

    if (g_failures) {
        std::cerr << g_failures << " failure(s)\n";
        return 1;
    }
    std::cout << "readings_view_test: ok\n";
    return 0;
}
