/*
 * SPDX-FileCopyrightText: 2026 Michal Jurosz (mj41)
 *
 * SPDX-License-Identifier: MIT
 */
// LVGL's memory (CONFIG_LV_USE_CUSTOM_MALLOC) in PSRAM, internal RAM only when PSRAM is full.
//
// With the C library's malloc, LVGL's objects, styles and label texts (mostly under 512 bytes,
// CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL) took the scarce internal RAM: 22 KB for Embody Mode's
// screens alone. Internal RAM is what Wi-Fi, Bluetooth, the audio codec's DMA and TLS's AES
// need; without it the robot lost its connection after its first sound (2026-10-12).
// The display's draw buffers are not LVGL's: esp_lvgl_port allocates them for DMA itself.
#include <lvgl.h>
#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM
#include <esp_heap_caps.h>
#include <stdlib.h>

void lv_mem_init(void)
{
}

void lv_mem_deinit(void)
{
}

lv_mem_pool_t lv_mem_add_pool(void* mem, size_t bytes)
{
    LV_UNUSED(mem);
    LV_UNUSED(bytes);
    return NULL;
}

void lv_mem_remove_pool(lv_mem_pool_t pool)
{
    LV_UNUSED(pool);
}

void* lv_malloc_core(size_t size)
{
    void* p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    return p ? p : malloc(size);
}

void* lv_realloc_core(void* p, size_t new_size)
{
    void* q = heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    return q ? q : realloc(p, new_size);
}

void lv_free_core(void* p)
{
    heap_caps_free(p);
}

void lv_mem_monitor_core(lv_mem_monitor_t* mon_p)
{
    LV_UNUSED(mon_p);
}

lv_result_t lv_mem_test_core(void)
{
    return LV_RESULT_OK;
}

#endif
