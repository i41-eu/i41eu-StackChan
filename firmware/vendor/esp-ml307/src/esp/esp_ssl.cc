// Changed by Embody Mode (2026-10-12): a bounded send and a send timeout; see EMBODY.md.
#include "esp_ssl.h"
#include "bounded_send.h"
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_crt_bundle.h>
#include <cstring>
#include <unistd.h>
#include <lwip/sockets.h>

static const char *TAG = "EspSsl";

EspSsl::EspSsl() {
    event_group_ = xEventGroupCreate();
}

EspSsl::~EspSsl() {
    Disconnect();

    if (event_group_ != nullptr) {
        vEventGroupDelete(event_group_);
        event_group_ = nullptr;
    }
}

bool EspSsl::Connect(const std::string& host, int port) {
    if (tls_client_ != nullptr) {
        ESP_LOGE(TAG, "tls client has been initialized");
        return false;
    }

    tls_client_ = esp_tls_init();
    if (tls_client_ == nullptr) {
        ESP_LOGE(TAG, "Failed to initialize TLS");
        return false;
    }

    esp_tls_cfg_t cfg = {};
    cfg.crt_bundle_attach = esp_crt_bundle_attach;

    int ret = esp_tls_conn_new_sync(host.c_str(), host.length(), port, &cfg, tls_client_);
    if (ret != 1) {
        esp_tls_error_handle_t last_error;
        if (esp_tls_get_error_handle(tls_client_, &last_error) == ESP_OK) {
            int error_code, error_flags;
            esp_err_t err = esp_tls_get_and_clear_last_error(last_error, &error_code, &error_flags);
            last_error_ = err;
            ESP_LOGE(TAG, "Failed to connect to %s:%d, code=0x%x", host.c_str(), port, err);
        } else {
            last_error_ = -1;
            ESP_LOGE(TAG, "Failed to get error handle");
        }
        esp_tls_conn_destroy(tls_client_);
        tls_client_ = nullptr;
        return false;
    }

    // Embody Mode: a send times out on a dead link ("would block", bounded in Send) instead of
    // holding its caller for ever; receiving stays as it was.
    int fd = -1;
    if (esp_tls_get_conn_sockfd(tls_client_, &fd) == ESP_OK && fd >= 0) {
        struct timeval send_timeout = {.tv_sec = 5, .tv_usec = 0};
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout));
    }

    connected_ = true;

    xEventGroupClearBits(event_group_, ESP_SSL_EVENT_RECEIVE_TASK_EXIT);
    xTaskCreate([](void* arg) {
        EspSsl* ssl = (EspSsl*)arg;
        ssl->ReceiveTask();
        xEventGroupSetBits(ssl->event_group_, ESP_SSL_EVENT_RECEIVE_TASK_EXIT);
        vTaskDelete(NULL);
    }, "ssl_receive", 4096, this, 1, &receive_task_handle_);
    return true;
}

void EspSsl::Disconnect() {
    connected_ = false;
    
    // Close socket if it is open
    if (tls_client_ != nullptr) {
        int sockfd;
        ESP_ERROR_CHECK(esp_tls_get_conn_sockfd(tls_client_, &sockfd));
        if (sockfd >= 0) {
            close(sockfd);
        }
    
        auto bits = xEventGroupWaitBits(event_group_, ESP_SSL_EVENT_RECEIVE_TASK_EXIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));
        if (!(bits & ESP_SSL_EVENT_RECEIVE_TASK_EXIT)) {
            ESP_LOGE(TAG, "Failed to wait for receive task exit");
        }

        esp_tls_conn_destroy(tls_client_);
        tls_client_ = nullptr;
    }
}

/* CONFIG_MBEDTLS_SSL_RENEGOTIATION should be disabled in sdkconfig.
 * Otherwise, invalid memory access may be triggered.
 */
int EspSsl::Send(const std::string& data) {
    if (!connected_) {
        ESP_LOGE(TAG, "Not connected");
        return -1;
    }

    // Embody Mode: "would block" retried for 5 s at most (upstream retried for ever).
    int ret = esp_ml307::BoundedSend(
        data.data(), data.size(),
        [this](const char* p, size_t n) { return (int)esp_tls_conn_write(tls_client_, p, n); },
        []() { return (uint32_t)(esp_timer_get_time() / 1000); },
        [](uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }, ESP_TLS_ERR_SSL_WANT_WRITE, 5000);
    if (ret < 0) {
        ESP_LOGE(TAG, "SSL send failed: ret=%d, errno=%d%s", ret, errno,
                 ret == esp_ml307::kSendTimedOut ? " (timed out)" : "");
    }
    return ret;
}

void EspSsl::ReceiveTask() {
    std::string data;
    while (connected_) {
        data.resize(1500);
        int ret = esp_tls_conn_read(tls_client_, data.data(), data.size());

        if (ret == ESP_TLS_ERR_SSL_WANT_READ) {
            continue;
        }

        if (ret <= 0) {
            if (ret < 0) {
                ESP_LOGE(TAG, "SSL receive failed: %d", ret);
            }
            connected_ = false;
            // 接收失败或连接断开时调用断连回调
            if (disconnect_callback_) {
                disconnect_callback_();
            }
            break;
        }
        
        if (stream_callback_) {
            data.resize(ret);
            stream_callback_(data);
        }
    }
}

int EspSsl::GetLastError() {
    return last_error_;
}
