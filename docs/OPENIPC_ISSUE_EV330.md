# AK3918EV330 (Tuya / Hutong `IPC-XC-C323`, LSC Smart Connect): hardware data from 7 cameras, 6 firmware versions

Draft issue for <https://github.com/OpenIPC/firmware/issues>. No identifiers, keys, MACs, IPs or Wi-Fi settings are included. Flash dumps are **not** attached (they hold the device identity and Tuya keys); only hashes are given.

## Summary

Seven cameras, all `ak3918ev330 dev board` / `AK39EV330`, root access via an SD-card hook ([lsc-tuya-toolkit](https://github.com/tasarren/lsc-tuya-toolkit) and a fork). All measurements below are read-only (`/proc`, `/sys`, `lsmod`, `/proc/mtd`, DTB, squashfs extraction). Related: #1970 (V330L/V300S PTZ), #1447 (SDK request), #1462 (LSC PTZ).

## Common to all 7

| Item | Value |
|---|---|
| SoC | AK3918EV330, ARM926EJ-S rev 5, single core, soft-float |
| Kernel | Linux 4.4.192 "V2.1" (vendor, uncompressed uImage, load `0x80008000`) |
| RAM | `mem=64M`; 24 MiB `dma_reserved` at `0x81400000`; `MemTotal` 35720 kB |
| Flash | 8 MiB SPI NOR; `mtdparts`: UBOOT 0x32000, ENV 0x1000, ENVBK 0x1000, DTB 0xc000, KERNEL 0x180000, ROOTFS 0x100000 (squashfs xz), CONFIG 0x40000 (jffs2 at `/etc/config`), APP 0x500000 (squashfs xz at `/usr`) |
| U-Boot header | `ANYKAH3B`, env platform `anycloud_h3b_nor_v330` |
| Wi-Fi | Realtek RTL8188FTV over USB (`ak_hcd`, `rtl8188ftv.ko`); `ssv6x5x.ko` shipped but unused |
| Audio | internal codec `ak_pcm`, AAC-LC 8 kHz mono in the stock stream |
| Console | `ttySAK0` 115200 8N1 with a getty |
| Userland | Buildroot 2018.02.7, GCC 4.9.4, uClibc 1.0.28, BusyBox 1.30.1 |
| Boot chain | `main.sh`: rtc, mmc, saradc, i2c, pcm, sensor driver, gpio_keys, ion, leds, uio, motor, then `anyka_ipc` |

## What differs between models

| | cam A | cam B | cam C | cam D | cam E, F | cam G |
|---|---|---|---|---|---|---|
| Firmware (`/usr/share/VERSION`) | 2.10.33.993 | 2.10.34.981 | 3.167.94.14 | 3.10.72.1254 | 3.10.78.1269 | 3.286.81.13 |
| Sensor module / ISP conf | `sensor_mis2006` / `isp_mis2006_mipi_2lane_h3b.conf` | same | same | same | `sensor_sc2336` / `isp_sc2336_mipi_2lane_h3b.conf` | same |
| `sensor_type_def` / `ircut_type` | 4 / 2 | 4 / 2 | 17 / 1 | 17 / 1 | 17 / 1 | 17 / 1 |
| Motors (`ak_motor`) | no (fixed) | yes | yes | yes | yes | yes |
| LEDs in `/sys/class/leds` | `state_led`, `fill_light_led`, `ir_led` | `state_led`, `ir_led` | `state_led`, `ir_led` | all three | all three | all three |
| Where resolution/codec are read | `/etc/config/_ht_hw_settings.ini` (2.10.x) | same | `/usr/local/_ht_hw_settings.ini` | same | same | same |

Notes that may help board definitions:

- Hardware-profile keys that really drive the encoder: `main_enc_out_type` (0 = H.264, 2 = H.265) and the main/sub resolution keys of `_ht_hw_settings.ini`. `factory_cfg.ini` does not.
- The 2.10.x firmware reads `_ht_hw_settings.ini` from the jffs2 CONFIG partition, the 3.x ones from `/usr/local` (APP). Overriding with a bind mount of a tmpfs file works on both.
- Reference camera (3.10.78.1269, SC2336, PTZ): GPIO from the DTB: `ir_led` 79, `fill_light_led` 34, `state_led` 68, IR-cut 42/41, reset button 5, SC2336 I2C0 `0x30` with reset GPIO 49, amplifier enable GPIO 82, motors 28BYJ-48 (`HZF-24BYJ48`, 4096 steps; motor0 pins 80, 81, 66, 9, travel 2500; motor1 pins 67, 6, 7, 8, travel 600). Light sensor on SARADC0 (thresholds 400/900).
- Hardware encoder limit seen on the reference camera: a 2304x1296 H.265 channel was refused (`Channel creation failed, processing power of the available cores insufficient`). 1280x720 H.265 at 15 fps runs on all 7. The cause is not proven.
- Free RAM with the stock `anyka_ipc` running is about 3 MB, with no swap.
- The OTA path checks only an MD5, no signature (observed in the update scripts).

## Hashes (reference camera, firmware 3.10.78.1269)

SHA-256 prefixes of the partition dumps, so others can check they match:

```
mtd0_UBOOT   bc6a9602fe626fb4
mtd3_DTB     f19e8abf3addc04b
mtd4_KERNEL  830255b86562b40
mtd5_ROOTFS  0580a28c48a28920
mtd7_APP     9045ac4f6813cabc
anyka_ipc    md5 3d8cad8080cae1f8bd6bca7331e2c651
```

(Full hashes available on request. The dumps themselves stay private because CONFIG holds device keys and the rest is vendor code.)

## Closed or proprietary pieces seen in the dump

`ak_isp.ko`, `ak_ion.ko`, `ak_motor.ko` and the other `ak_*.ko` modules, about 16 `sensor_*.ko`, `libavssdkbeta.so`, `liblvhsr.so`, `liblink_visual_lite_ai.so`. `anyka_ipc` (5 MB) links the Anyka media SDK statically (`ak_vi`, `ak_vpss`, `ak_venc`, `ak_aenc`, `ak_adec`, `ak_ai`, `ak_ao`, `ak_osd`, `ak_md`) and the Tuya HAL; it has only `liblvhsr.so`, `libstdc++`, `libgcc_s`, `libc` and the loader as dynamic dependencies.

## What I can do / ask

- Run read-only checks on the cameras if a specific register, GPIO or file would help a board definition for the EV330 + MIS2006 / SC2336 combination.
- Test OpenIPC U-Boot or a kernel from RAM on one camera, only after a serial-console recovery path is confirmed (not yet tested here).
- Question: is a PTZ EV330 board with the `ak_motor` GPIO layout above welcome as a config, and which sensor drivers are you willing to ship given they are closed modules?

Everything above was measured on my own cameras, with the cloud disabled and no flash writes.
