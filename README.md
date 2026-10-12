# i41eu-StackChan: StackChan with Embody Mode

M5Stack's StackChan firmware with **Embody Mode**: the first launcher app, which makes the robot a
light client of a home server (camera, microphone, speaker, every sensor as raw data, and
commands), switchable between servers and apps (a dashboard, a pet, a focus timer, a robot for
your own programs, a cockpit that drives a TPBot car over BLE). It is part of the
[i41](https://github.com/i41-eu/i41eu-arch) home platform, local first and privacy first: the
[manager](https://github.com/i41-eu/i41eu-manager) sets robots up and gives them their apps,
[Raw data](https://github.com/i41-eu/q41eu-raw) is the first app, and
[the robot in 3D](https://github.com/i41-eu/i41eu-s3d) shows what each app does with it.

- **Branches:** `embody` (ours, the default) and `main` (upstream's, as it is). `embody` is based
  on upstream's 1b57655 ("Merge pull request #112 from m5stack/firmware-dev"). To sync:
  `git remote add upstream https://github.com/m5stack/StackChan.git`, `git fetch upstream`, then
  `git merge upstream/main` into `embody`.
- **Embody Mode** lives in
  [firmware/main/apps/app_embody_mode](firmware/main/apps/app_embody_mode/README.md): its
  [README](firmware/main/apps/app_embody_mode/README.md),
  [SETUP](firmware/main/apps/app_embody_mode/SETUP.md) (set a robot up: one click in Chrome at
  [m.i41.eu](https://m.i41.eu), or your own server or build) and
  [HARDWARE](firmware/main/apps/app_embody_mode/HARDWARE.md).
- **Releases:** a tag `embody-v*` builds and publishes the firmware
  (`.github/workflows/embody-firmware.yml`); the managers install the latest release. Current:
  embody-v0.6.0.
- **Upstream:** Embody Mode is meant to be offered to upstream as an official mode (an app of
  the launcher), so its changes are kept apart in a few commits on top of upstream.
- **License:** MIT, as upstream (M5Stack's copyright kept, [firmware/LICENSE](firmware/LICENSE)).
  Stack-chan was created by Shinya Ishikawa and the Stack-chan community
  ([github.com/stack-chan/stack-chan](https://github.com/stack-chan/stack-chan)); this project is
  independent of it and of M5Stack.

**A proof of concept, vibe coded:** written with AI agents and tested on a real robot at home,
but neither the code nor its security has been reviewed by humans. Use it on your own network.

---

# Upstream: StackChan Open-Source

<img src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1205/K151_stack_chan_main_pictures_01.webp" width="60%">

Here are StackChan related open-source resources, including source code of the StackChan firmware, remote controller firmware, mobile app (iOS and Android), and server. 

Update of this repo could be a little late than the released firmware and mobile app. 

----

<img src="https://cdn.shopify.com/s/files/1/0056/7689/2250/files/5a589623895f65487717894d9240f6b8.png" width="60%">

**StackChan is a super kawaii AI desktop robot co-created by M5Stack and the user community.** It uses the M5Stack **flagship IoT development kit [CoreS3](https://docs.m5stack.com/en/core/CoreS3)** as its main controller, powered by an ESP32-S3 SoC featuring a 240 MHz dual-core processor, with 16MB Flash and 8MB PSRAM onboard, and supporting Wi-Fi and BLE. The main unit also integrates a 2.0-inch capacitive touch display with a high-strength glass cover, a 0.3 MP camera, a proximity & ambient light sensor, a 9-axis IMU (accelerometer + gyroscope + magnetometer), a microSD card slot, a 1W speaker, dual microphones, and power/reset buttons. 

The **robot body**, connected to the main unit, includes a USB-C interface for power and data, a 550 mAh battery, two feedback servos (360-degree continuous rotation on the horizontal axis and 90-degree movement on the vertical axis), two rows totaling 12 RGB LEDs, infrared transmitter and receiver, a three-zone touch panel, and a full-featured NFC module. 

The **factory firmware** is feature-rich, including an AI Agent, lively and expressive animations, ESP-NOW wireless remote control, and online app downloads. It can connect to a mobile app for video viewing, remote avatar control, and more, and also supports online updates (OTA). The product also supports programming via Arduino, UiFlow2, and other methods, and can connect to various expansion units in the M5Stack ecosystem, making it easy to implement a wide range of custom functions. 

> ⚠️ Do not forcibly rotate any movable parts connected to the motors by hand when you are unsure whether the motors are powered and under control, as this may cause hardware damage. 

- Purchase link: [M5Stack Official Store](https://shop.m5stack.com/products/stackchan-kawaii-co-created-open-source-ai-desktop-robot) | [淘宝 Taobao](https://item.taobao.com/item.htm?id=1042238294510)

- Product document page: [English](https://docs.m5stack.com/en/StackChan) | [日本語](https://docs.m5stack.com/ja/StackChan) | [中文](https://docs.m5stack.com/zh_CN/StackChan)

- Board support package: https://github.com/m5stack/StackChan-BSP

Thank you to the contributors of the StackChan community, especially: 

| ![](https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1205/avatar_stack_chan.jpg) | ![](https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1205/avatar_takao.jpg) |
| -------------------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| [@stack_chan](https://x.com/stack_chan)                                          | [@mongonta555](https://x.com/mongonta555)                                   |
| Shinya Ishikawa                                                                  | Takao Akaki                                                                 |
