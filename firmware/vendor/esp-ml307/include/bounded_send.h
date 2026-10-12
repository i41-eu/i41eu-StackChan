// SPDX-FileCopyrightText: 2026 Michal Jurosz (mj41)
// SPDX-License-Identifier: Apache-2.0
//
// Embody Mode's change to esp-ml307 (vendored from 78/esp-ml307 3.6.5): a send that cannot
// block for ever. Upstream's EspSsl::Send retried "would block" without end; on a dead link
// (no Wi-Fi, a lost route) the caller, the robot's app loop, then never came back.
#pragma once
#include <cstddef>
#include <cstdint>

namespace esp_ml307 {

// BoundedSend writes size bytes with write(data, n) -> sent or < 0. A would_block answer is
// retried after sleep_ms(retry_ms) until limit_ms have gone by (now_ms()); then it gives up
// with timed_out. Another error ends it at once. It returns the bytes sent, or < 0.
constexpr int kSendTimedOut = -0x7FFF;

template <typename Write, typename NowMs, typename SleepMs>
int BoundedSend(const char* data, size_t size, Write write, NowMs now_ms, SleepMs sleep_ms, int would_block,
                uint32_t limit_ms, uint32_t retry_ms = 10)
{
    size_t total = 0;
    const uint32_t start = now_ms();
    while (total < size) {
        const int ret = write(data + total, size - total);
        if (ret == would_block) {
            if ((uint32_t)(now_ms() - start) >= limit_ms) {
                return kSendTimedOut;
            }
            sleep_ms(retry_ms);
            continue;
        }
        if (ret <= 0) {
            return ret == 0 ? -1 : ret;
        }
        total += (size_t)ret;
    }
    return (int)total;
}

}  // namespace esp_ml307
