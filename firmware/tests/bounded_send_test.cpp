/*
 * SPDX-FileCopyrightText: 2026 Michal Jurosz (mj41)
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * The robot's sends (esp-ml307, vendored: include/bounded_send.h) on the host: a link that only
 * ever answers "would block" (a dead Wi-Fi, a lost route) must not hold the caller for ever.
 * Before the fix, the robot's app loop stayed in such a send until a restart (2026-10-12).
 */
#include <bounded_send.h>
#include <cstdio>
#include <string>

static int failures = 0;
static void expect(bool ok, const char* what)
{
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        failures++;
    }
}

int main()
{
    using esp_ml307::BoundedSend;
    const int kWouldBlock = -0x6880;  // ESP_TLS_ERR_SSL_WANT_WRITE
    const std::string data(1000, 'x');

    // A dead link: would block, for ever. A fake clock moves with each sleep.
    uint32_t clock = 0, writes = 0, sleeps = 0;
    int ret = BoundedSend(
        data.data(), data.size(), [&](const char*, size_t) { writes++; return kWouldBlock; }, [&]() { return clock; },
        [&](uint32_t ms) { clock += ms; sleeps++; }, kWouldBlock, 5000);
    expect(ret == esp_ml307::kSendTimedOut, "a link that always blocks gives up");
    expect(clock >= 5000 && clock < 5100, "after its time limit, not before nor much after");
    expect(sleeps > 0 && sleeps == writes - 1, "it waits between tries (no busy loop)");

    // Partial writes, then done: everything is sent.
    clock = 0;
    size_t got = 0;
    ret = BoundedSend(
        data.data(), data.size(),
        [&](const char*, size_t n) {
            size_t k = n < 300 ? n : 300;
            got += k;
            return (int)k;
        },
        [&]() { return clock; }, [&](uint32_t ms) { clock += ms; }, kWouldBlock, 5000);
    expect(ret == 1000 && got == 1000, "partial writes add up");

    // A short stall, then the link moves again: still sent.
    clock = 0;
    int calls = 0;
    ret = BoundedSend(
        data.data(), data.size(), [&](const char*, size_t n) { return ++calls <= 20 ? kWouldBlock : (int)n; },
        [&]() { return clock; }, [&](uint32_t ms) { clock += ms; }, kWouldBlock, 5000);
    expect(ret == 1000, "a stall shorter than the limit is waited out");

    // Another error ends it at once.
    calls = 0;
    ret = BoundedSend(
        data.data(), data.size(), [&](const char*, size_t) { calls++; return -0x4E; }, [&]() { return clock; },
        [&](uint32_t ms) { clock += ms; }, kWouldBlock, 5000);
    expect(ret == -0x4E && calls == 1, "an error is not retried");

    if (failures) {
        std::fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    std::printf("bounded_send_test: ok\n");
    return 0;
}
