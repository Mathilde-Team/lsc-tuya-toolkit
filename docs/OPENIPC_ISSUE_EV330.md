# Brouillon d'issue OpenIPC : AK3918EV330 (Tuya / Hutong `IPC-XC-C323`)

À déposer sur <https://github.com/OpenIPC/firmware/issues> avec le modèle **« New board or SoC: hardware profile and vendor dump »** (`5-new-hardware.yml`). Les champs ci-dessous suivent ce modèle ; le texte à coller est en anglais.

Aucun identifiant, clé, MAC, IP ni réglage Wi-Fi n'est inclus. Les dumps ne sont pas joints : ils contiennent l'identité Tuya de la caméra.

## À compléter par le propriétaire avant publication (je ne peux pas les produire)

| Champ du modèle | Obligatoire | État |
|---|---|---|
| Photos de la carte (recto, verso, gros plan de chaque puce : SoC, capteur, flash, Wi-Fi) | **oui** | à faire : boîtier à ouvrir |
| Modèle commercial et marque (« Retail model ») | **oui** | fait : LSC Smart Connect, Rotatable Camera 1080p, réf. 3007325 (nom interne `IPC-XC-C323`) |
| Lien d'achat | non, mais décisif | à renseigner |
| Marquage et JEDEC de la puce flash | non | à lire sur le composant (le DTB liste 65 JEDEC possibles) |
| Log de boot série complet depuis la mise sous tension | non | à faire avec un adaptateur UART 3,3 V (le `dmesg` en marche est saturé par les traces Wi-Fi) |
| Bannière et commandes U-Boot en direct, protection par mot de passe | non | à faire sur la console série (voir ci-dessous ce qui est lisible dans le dump) |
| Cases « Before you start » (liste openipc.org, issues ouvertes et fermées) | oui | à cocher après vérification |
| Cases « What you can do with this board » | non | à cocher selon le cas |

Choix proposé pour « The vendor firmware dump » : **Available on request, I would rather not publish it**.

---

# Texte à coller (anglais)

**Title:** AK3918EV330 (Tuya/Hutong `IPC-XC-C323`): hardware profile from 7 cameras, 6 firmware versions

**SoC vendor:** Anyka. **SoC model:** AK3918EV330 (`AK39EV330`). Related: #1970, #1447, #1462; `qemu-bench` already has `ak3918ev330` machines.

## Hardware profile

### Device
- **Retail:** LSC Smart Connect Rotatable Camera 1080p, ref. 3007325 (internal name `IPC-XC-C323`)
- **Board id:** `ak3918ev330 dev board` (DTB model string); product string in `anyka_ipc`: `HT_IPC286KS2M_TUYA_AK3918EV330`
- **Vendor firmware:** Tuya SDK; `/usr/share/VERSION` values seen on the 7 units: 2.10.33.993, 2.10.34.981, 3.10.72.1254, 3.10.78.1269, 3.167.94.14, 3.286.81.13. Reference unit: 3.10.78.1269.

### Main SoC
- **Model / chip id:** AK3918EV330, `/proc/cpuinfo` ARM926EJ-S rev 5 (ARMv5TEJL), single core, ~351 BogoMIPS, soft-float
- **Stock kernel:** `Linux version 4.4.192V2.1 (hutong@ubuntu) (gcc 4.9.4, Buildroot 2018.02.7_V1.0.03) #1 Wed Apr 27 10:24:15 CST 2022`, uncompressed uImage, load `0x80008000`
- **Userland:** Buildroot 2018.02.7, uClibc 1.0.28, BusyBox 1.30.1, init `/sbin/init`, ROOTFS read-only squashfs xz
- **IRQs / iomem (reference unit):** `ak_camera` (1), `video-encoder` (2), `mmc0` at `0x20100000` (4), `spi0` at `0x20120000` (8), `uart0` at `0x20130000` (10), `l2` (12), `ak-hshcd:usb1` (18), `otgdma` (19), `key_detect` GPIO 5. iomem: saradc `0x08000000`, vi0 `0x20000000`, mmc0 `0x20100000`, spi0 `0x20120000`, uart0 `0x20130000`, i2c0 `0x20150000`, gpio `0x20170000`, RAM `0x80000000-0x83ffffff`.

### Sensor
- **Model:** SmartSens **SC2336** (MIPI 2 lanes, I2C0 address `0x30`, reset GPIO 49) on 3 units; **MIS2006** (MIPI 2 lanes) on 4 units. Modules: `sensor_sc2336.ko`, `sensor_mis2006.ko`; ISP tuning: `isp_sc2336_mipi_2lane_h3b.conf` (114 KB), `isp_mis2006_mipi_2lane_h3b.conf` (114 KB). `sensor_type_def` is 17 on the SC2336 units and 4 on the oldest ones (stock profile of one unit: 20).
- **Resolution, fps:** stock profile 1920x1080 at 15 fps, 768 kbit/s target (1024 max), GOP 30, QP 28 to 43, `main_enc_out_type=0` (H.264); sub stream 640x360, 15 fps, 256 kbit/s. Both 1280x720 H.264 and H.265 (`main_enc_out_type=2`) at 15 fps run on all 7 units.
- A 2304x1296 H.265 channel (requested by an upstream config) is refused: `Channel creation failed, processing power of the available cores insufficient`. Cause not proven.

### RAM
- 64 MiB DDR declared (`mem=64M memsize=64M`); `MemTotal` 35720 kB. 24 MiB `dma_reserved` at `0x81400000` (ION pool for VI/ISP/encoder). About 3 MB free with the stock app running, no swap.

### Wi-Fi
- Realtek **RTL8188FTV**, USB `0bda:f179` (reported as `802.11n`), behind the SoC's USB host `ak_hcd`. Stock modules: `rtl8188ftv.ko`, `ak_hcd.ko`, `cfg80211.ko`, `mac80211.ko`. `ssv6x5x.ko` and `ssv6x5x-wifi.cfg` are shipped but unused here.
- Ethernet MAC `ak_eth.ko` exists in the image; no PHY is wired as far as I can tell.

### Flash
- 8 MiB SPI NOR on `spi0.0` (`spi:ak-spiflash`), `sf_hz=20000000` in U-Boot env. **Chip marking and JEDEC not read yet**; the DTB lists 65 candidate JEDEC ids (Winbond `0xEF…` 10, Micron/XMC `0x20…` 18, Atmel `0x1F…` 8, SST `0xBF…` 8, Macronix `0xC2…` 6, others).

### Flash layout (`mtdparts` from the stock cmdline)
```
spi0.0:200K@0x0(UBOOT),4K@0x32000(ENV),4K@0x33000(ENVBK),48K@0x34000(DTB),1536K@0x40000(KERNEL),1024K@0x1C0000(ROOTFS),256K@0x2C0000(CONFIG),5120K@0x300000(APP)
```
UBOOT = `ANYKAH3B` header; KERNEL = uImage; ROOTFS = squashfs xz (mounted `/`, 369 inodes); CONFIG = jffs2 on `/etc/config`; APP = squashfs xz on `/usr`. Same layout on all 7 units. Cmdline: `console=ttySAK0,115200n8 root=/dev/mtdblock5 rootfstype=squashfs init=/sbin/init mem=64M memsize=64M mtdparts=…`.

### Bootloader (from the U-Boot dump and the ENV partition, not from a live console)
- **Banner:** `U-Boot 2013.10.0-V3.1.28_bchV1.0.00 (Apr 12 2023 - 14:09:26)`
- **Platform / env:** `anycloud_h3b_nor_v330`, `env_version=1.0.01`, `bootdelay=1`, `bootcmd=run boot_normal`, `loadaddr=0x80008000`, `fdtcontroladdr=0x81300000`, `sf_hz=20000000`, `read_kernel=sf probe 0:0 ${sf_hz} 0; sf read …`, `update=tftp …; sf probe; sf update …`
- **Commands present in the binary:** `sf`, `tftp`, `mmc`, `fatload`, `fdt`, `bootm`.
- **Password protected / actual console prompt:** *not checked yet (needs serial).*

### Video / Audio
- Encoder is a hardware block (`video-encoder` IRQ 2) driven only through the closed Anyka media SDK. H.264 and H.265 both used. Audio: internal codec `ak_pcm`, AAC-LC 8 kHz mono; speaker amplifier enable on GPIO 82.

## GPIO and peripherals (from the DTB / `_ht_hw_settings.ini`)

| Function | Value |
|---|---|
| `ir_led` / `fill_light_led` / `state_led` | GPIO 79 / 34 / 68 (active level 1). Some units only have `state_led` and `ir_led` in `/sys/class/leds` |
| IR-cut | GPIO 42 (A) and 41 (B/EN); day levels 0 / 1, night 1 / 0; `ircut_type` 1 or 2 depending on the unit |
| Light sensor | SARADC0, thresholds 400/900 (DTB); `ps_hw_higher_level=2350`, `ps_hw_lower_level=2000`, software day/night counters |
| Reset button | `key_detect` GPIO 5, code `0x74` |
| Pan/tilt motors | 28BYJ-48 (`HZF-24BYJ48`), 4096 steps/turn. motor0 pins 80, 81, 66, 9 (travel 2500); motor1 pins 67, 6, 7, 8 (travel 600). Profile: `pan_total_steps=4096`, `tilt_total_steps=2230`, `pan_max_degree=360`, `tilt_max_degree=196`, `tilt_clkwise_inv=1` |
| Fixed camera | no `ak_motor.ko` loaded on the oldest unit (2.10.33.993) |
| Watchdog | `ak39ev330-wdt`, fed by the `daemon` binary (`killall -12 daemon` stops it) |
| UART | `ttySAK0` 115200 8N1 with a getty on the console (shell on serial); `/dev/ttySAK1` referenced by `anyka_ipc` |
| Wi-Fi power | none switched by GPIO that I found; Wi-Fi power-save disabled by script on my units |

## Differences between the 7 units

| | A | B | C | D | E, F | G |
|---|---|---|---|---|---|---|
| Firmware | 2.10.33.993 | 2.10.34.981 | 3.167.94.14 | 3.10.72.1254 | 3.10.78.1269 | 3.286.81.13 |
| Sensor | MIS2006 | MIS2006 | MIS2006 | MIS2006 | SC2336 | SC2336 |
| Motors | no | yes | yes | yes | yes | yes |
| LEDs | all three | `state_led`, `ir_led` | `state_led`, `ir_led` | all three | all three | all three |
| Resolution/codec read from | `/etc/config/_ht_hw_settings.ini` (2.10.x) | same | `/usr/local/_ht_hw_settings.ini` | same | same | same |

- The keys that really drive the encoder are `main_enc_out_type` (0 = H.264, 2 = H.265) and the main/sub width/height/fps of `_ht_hw_settings.ini`; `factory_cfg.ini` is not used for this.
- Everything above was applied with `mount --bind` of a tmpfs copy and `LD_PRELOAD` from the SD card; **nothing is written to flash**.
- The OTA path checks only an MD5, no signature (update scripts).

## Closed / proprietary pieces

- Kernel modules (no sources found for these exact ones): `ak_isp`, `ak_ion`, `ak_motor`, `ak_pcm`, `ak_i2c`, `ak_saradc`, `ak_mci`, `ak_rtc`, `ak_leds`, `ak_gpio_keys`, `ak_pwm_char`, `ak_uio`, `ak_efuse`, `ak_crypto`, `ak_eth`, `ak_udc`, `ak_hcd`, `ak_adc_key`, ~16 `sensor_*.ko` (MIS2006, SC2336, SC3336, SC4336, GC2063, GC2083, F22/F23/F28/F35/F37/F37P, Q03P, MIS2008, MIS4001, SC1346, SC223A, SC2331, SC3332).
- Userland: `libavssdkbeta.so`, `liblvhsr.so`, `liblink_visual_lite_ai.so`; `anyka_ipc` (5 MB) links the Anyka media SDK statically (`ak_vi`, `ak_vpss`, `ak_venc`, `ak_aenc`, `ak_adec`, `ak_ai`, `ak_ao`, `ak_osd`, `ak_md`) plus the Tuya HAL.

## Reference hashes (unit running 3.10.78.1269)

```
mtd0_UBOOT   sha256 bc6a9602fe626fb4...
mtd3_DTB     sha256 f19e8abf3addc04b...
mtd4_KERNEL  sha256 830255b86562b40...
mtd5_ROOTFS  sha256 0580a28c48a28920...
mtd7_APP     sha256 9045ac4f6813cabc...
anyka_ipc    md5    3d8cad8080cae1f8bd6bca7331e2c651
```

Full hashes and a redacted dump (CONFIG and device identity removed) available on request.

## Stock boot log

*Not captured yet.* The running `dmesg` has lost the boot messages (ring buffer filled by Realtek driver traces), so the boot log needs a serial capture from power-on. I will add it when I have a UART adapter on one of the units.

## What I can do with these boards

- Read-only commands on 7 running cameras (any register, sysfs or file that helps the profile).
- Test builds from RAM or the SD card on one unit; flash only once a serial recovery path is verified.
- Serial console: *not yet*. Programmer: *no*.

Questions: is an EV330 board with this PTZ/GPIO layout welcome as a `devices/` profile in OpenIPC/builder, and are the closed `ak_*` / `sensor_*` modules acceptable as vendor blobs the way other Anyka boards ship them?

Everything was measured on my own cameras, with the cloud disabled.
