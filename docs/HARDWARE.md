# Audit matériel : caméra Anyka AK3918EV330 (Tuya / Hutong-Interthings)

Collecte en lecture seule le 2026-11-09 sur la caméra décrite dans [`CAMERA.md`](CAMERA.md)
(IPC-XC-C323, firmware 3.10.78.1269). Rien n'a été écrit sur la caméra pendant cet audit.

## 1. Plateforme

| Élément | Valeur | Source |
|---|---|---|
| SoC | Anyka AK3918EV330 (`AK39EV330`), carte « ak3918ev330 dev board » | /proc/cpuinfo, DTB |
| CPU | ARM926EJ-S rev 5, ARMv5TEJL, mono-coeur, ~351 BogoMIPS, soft-float | /proc/cpuinfo |
| RAM | 64 Mo déclarés (`mem=64M`), ~35 Mo visibles (voir 3) | cmdline, meminfo |
| Flash | SPI NOR 8 Mo (`spi0.0`, 20 MHz au boot). Puce exacte non identifiée : le DTB liste plusieurs JEDEC (Atmel `at25fs010` + autres), à lire sur le boîtier | DTB, env U-Boot |
| Stockage ext. | carte microSD sur `mmc0` (`mmcblk0p1`, FAT) | /proc/partitions |
| Capteur | **SmartSens SC2336**, MIPI 2 lanes, I2C0 adresse `0x30`, reset GPIO 49 | `sc2336 0-0030`, DTB |
| Résolution capteur | 1920x1080 (le toolkit amont demandait 2304x1296 en sortie, au-delà du capteur) | logs |
| Wi-Fi | **Realtek RTL8188FTV/FU, USB** via `ak_hcd` (driver `rtl8188ftv.ko`, wlan0). Driver `ssv6x5x.ko` présent mais inutilisé | lsmod, wifi_driver_new.sh |
| Ethernet | contrôleur `ak_eth` dans le SoC, driver non chargé (pas de PHY câblé apparemment) | main.sh |
| Audio | codec intégré (`ak_pcm`), ampli HP activé par GPIO 82 | hw_settings |
| PTZ | 2 moteurs pas à pas 28BYJ-48 (`HZF-24BYJ48`), 4096 pas/tour. motor0 pins 80,81,66,9 (course 2500) ; motor1 pins 67,6,7,8 (course 600) | DTB |
| IR-cut | GPIO 42 (A) et 41 (B) | DTB |
| LED | `ir_led` GPIO 79, `fill_light_led` GPIO 34, `state_led` GPIO 68 | DTB |
| Capteur de lumière | SARADC0, seuils 400/900 | DTB |
| Bouton | `key_detect`, GPIO 5 (reset), code 0x74 | DTB |
| Watchdog | `ak39ev330-wdt` (daemon `daemon` le nourrit ; `killall -12 daemon` le désactive) | DTB, recover_cfg.sh |
| UART console | `ttySAK0`, 115200 8N1, getty actif (**shell sur la console série**) | cmdline, inittab |
| Second UART | `/dev/ttySAK1` (référencé par anyka_ipc) | strings |

## 2. Carte mémoire flash (8 Mo, 0x800000)

| MTD | Nom | Offset | Taille | Contenu |
|---|---|---|---|---|
| mtd0 | UBOOT | 0x000000 | 200K | U-Boot Anyka (en-tête `ANYKAH3B`) |
| mtd1 | ENV | 0x032000 | 4K | environnement U-Boot |
| mtd2 | ENVBK | 0x033000 | 4K | sauvegarde de l'env (identique à ENV) |
| mtd3 | DTB | 0x034000 | 48K | device-tree (40 055 octets utiles) |
| mtd4 | KERNEL | 0x040000 | 1536K | uImage Linux 4.4.192, **non compressé**, load `0x80008000` |
| mtd5 | ROOTFS | 0x1C0000 | 1024K | squashfs xz, créé le 2023-08-05 |
| mtd6 | CONFIG | 0x2C0000 | 256K | jffs2, monté sur `/etc/config` |
| mtd7 | APP | 0x300000 | 5120K | squashfs xz, monté sur `/usr`, créé le 2023-09-23 |

Les offsets viennent de `mtdparts` dans la ligne de commande noyau. Les tables internes du binaire U-Boot (1600K/1960K/...) sont des valeurs par défaut écrasées par l'ENV.

### Boot
`U-Boot -> sf read kernel -> bootm 0x80008000 - 0x81300000` avec
`console=ttySAK0,115200n8 root=/dev/mtdblock5 rootfstype=squashfs init=/sbin/init mtdparts=... mem=64M memsize=64M`.
Plate-forme env : `anycloud_h3b_nor_v330`, `env_version=1.0.01`, mise à jour TFTP intégrée (`update=tftp ...; sf update`) avec une adresse de serveur et de carte par défaut propres au constructeur.

## 3. Mémoire
`dma_reserved` : **24 Mo réservés** à `0x81400000` (heap ION pour VI/ISP/encodeur). Il reste ~40 Mo au noyau (35 Mo visibles).
L'encodeur a refusé le canal 2304x1296 H.265 demandé par la config amont (`Channel creation failed, processing power of the
available cores insufficient`). La RAM limitée et l'encodeur matériel de ce SoC sont des explications plausibles, **non prouvées** :
voir [`CAMERA.md`](CAMERA.md) pour la vraie différence (config d'un autre modèle).
RAM libre en fonctionnement : ~3 Mo, aucun swap. Tout ajout (dropbear, scripts lourds) doit rester petit.

## 4. Logiciel de base

| Élément | Valeur |
|---|---|
| Noyau | Linux 4.4.192 « V2.1 », compilé 2022-04-27 par `hutong@ubuntu`, **non compressé** |
| Toolchain | Buildroot 2018.02.7 (V1.0.05), **GCC 4.9.4**, **uClibc 1.0.28**, ARM EABI5 soft-float, ARMv5TE |
| Init | BusyBox 1.30.1 (2021-03-10) init, `inittab` + `rcS` -> `rc.local` -> `/usr/sbin/main.sh` |
| Rootfs | 369 inodes, squashfs xz bloc 128K, en lecture seule |
| `/usr` (APP) | binaires Tuya : `anyka_ipc`, `daemon`, `cli`, `wpa_supplicant`, `rtwpriv`, `mtd_update`, modules `.ko`, scripts `.sh`, ISP conf de ~20 capteurs |
| Modules `.ko` | ak_isp, ak_ion, ak_uio, ak_motor, ak_pcm, ak_i2c, ak_saradc, ak_rtc, ak_leds, ak_gpio_keys, mmc_*, ak_mci, usbcore, ak_hcd, cfg80211, mac80211, rtl8188ftv, ssv6x5x, ~16 pilotes capteur |
| Séquence | main.sh : syslogd, rtc, mmc, saradc, i2c, pcm, **sensor_driver.sh** (SC2336), gpio_keys, ion, **ak_leds + `state_led.sh on`**, uio, motor, puis `service.sh start` |
| Hook toolkit | `service.sh` lance `/tmp/hostapd` (remplacé par la SD) qui lance `/mnt/hack.sh` |

## 5. Pour construire un firmware

1. **Matériel requis** : le noyau 4.4.192 est fourni sans source ici. Les sources Anyka (`AK3918EV330`, `anyka_sdk`) sont chez le fabricant. Sans elles, la voie réaliste est de **modifier l'existant** (rootfs/APP) et de garder noyau, DTB et U-Boot d'origine.
2. **Outils** : `mksquashfs` (xz, `-b 131072`, `-comp xz`), `mkfs.jffs2` (erase block 4096), `dd`, `flashrom` si programmateur SPI.
3. **Contraintes de taille** : ROOTFS <= 1 048 576 octets (actuel 1 012 684, **il reste 35 Ko**). APP <= 5 242 880 (actuel 5 174 406, il reste 68 Ko). Pour ajouter des fichiers, il faut retirer des pilotes de capteurs inutilisés (~15 `.ko` et conf ISP d'autres capteurs : gain de plusieurs centaines de Ko).
4. **Cibles de compilation** : `arm-buildroot-linux-uclibcgnueabi-gcc` 4.9.4, `-march=armv5te -mfloat-abi=soft`. Binaire en uClibc, pas glibc.
5. **Flasher** :
   - via U-Boot TFTP (UART requis) : `tftp ${loadaddr} img; sf probe; sf update ...`,
   - via `mtd_update` / `tf_update.sh` (mise à jour par SD, existe dans `/usr/sbin`),
   - ou clip SOIC sur la puce SPI avec `flashrom`.
6. **Accès de secours** : la console UART (3,3 V, 115200 8N1) donne un getty. **Sauvegardez toujours avant de flasher** (voir ci-dessous).
7. **Vérification d'intégrité** : l'OTA ne contrôle que le MD5, aucune signature : un firmware modifié sera accepté.

## 6. Sauvegarde de la flash (non publiée)
Un dump complet de la flash a été fait en lecture seule, hors de ce dépôt : `mtd0..mtd7.bin`, `live_fdt.dtb`,
`full_flash_8MB.bin` (image reconstruite, zones vides à 0xFF) et `SHA256SUMS`, avec les extraits `rootfs/` et `app/`
(squashfs décompressés) et `device-tree.dts` (DTS décodé).

**Ces fichiers ne sont pas dans le dépôt** : `mtd6_CONFIG.bin` et `full_flash_8MB.bin` contiennent l'identité et les clés
Tuya de la caméra (`_ht_uuid.ini`, `tuya_enckey.db`, `_ht_encrypt.bin`), la MAC et d'éventuels identifiants Wi-Fi. Le firmware
du fabricant (`anyka_ipc`, `rootfs`, `/usr`) est aussi sous droits : le dépôt ne publie que des scripts qui patchent
**votre** copie (`tools/`).

Pour refaire un dump chez vous (lecture seule), depuis le shell de la caméra :
```
# sur le PC          : nc -l -p 9000 > mtd7_APP.bin
# sur la caméra      : nc <IP_DU_PC> 9000 < /dev/mtd7ro
```
Remplacez `mtd7` par `mtd0` à `mtd6` pour les autres partitions ; `unsquashfs` ouvre `mtd5` (ROOTFS) et `mtd7` (APP).

## 7. Limites et points non vérifiés
- Modèle exact de la puce SPI NOR : à lire sur le composant.
- `full_flash_8MB.bin` est reconstruit depuis les MTD (le vrai dump SPI peut différer sur les zones hors partitions, ex. 0x32000 à 0x34000).
- Pas de `/proc/config.gz` : la configuration du noyau n'est pas disponible. Les sources du fabricant seraient nécessaires pour recompiler le noyau.
- Le brochage UART physique (TX/RX/GND) n'est pas déterminé par ce relevé logiciel.
