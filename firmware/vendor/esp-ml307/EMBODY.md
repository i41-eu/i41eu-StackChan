# esp-ml307, vendored for Embody Mode

78/esp-ml307 3.6.5 (github.com/78/esp-ml307, Apache-2.0), used through `override_path` in
`main/idf_component.yml`, with one change: **a send cannot block for ever.**

- `include/bounded_send.h` (new): a write retried on "would block" for at most a time limit,
  with a pause between tries; tested on the host (`tests/bounded_send_test.cpp`).
- `src/esp/esp_ssl.cc`: after the handshake the TLS socket gets a 5 s send timeout
  (`SO_SNDTIMEO`; receiving unchanged), and `EspSsl::Send` gives up after 5 s of "would block"
  (upstream retried without end).
- `src/esp/esp_tcp.cc`: the plain TCP socket gets a 5 s send timeout (`SO_SNDTIMEO`).
- `idf_component.yml`: upstream's `repository_info` removed (a component used through
  `override_path` may not point to a repository path; the component manager refuses it).
- The changed files carry a "Changed by Embody Mode" line at the top.

Why (2026-10-12): on a dead link (lost Wi-Fi or route) a send never returned, so Embody Mode's
app loop stopped (the screen froze, commands and reports stopped) and it never reconnected; a
server that stops reading reproduced it. With the change the send fails, Embody Mode drops the
connection and reconnects (`embody_client.cpp`, `drop_socket`).
