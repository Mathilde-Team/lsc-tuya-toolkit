# La caméra de ce fork (et en quoi elle diffère du dépôt d'origine)

Ce fork part d'une caméra **différente** de celle du dépôt amont `tasarren/lsc-tuya-toolkit`.
Les fichiers de configuration du toolkit ont été écrits pour l'autre modèle ; les appliquer tels quels
sur celle-ci provoquait une boucle de redémarrage de `anyka_ipc` (voir `docs/CHANGES.md`).

## Identification de la caméra utilisée ici

| Élément | Valeur | Source |
|---|---|---|
| Nom de modèle interne | `IPC-XC-C323` (`dev_name`) | `factory_cfg.ini` |
| **Firmware applicatif** | **3.10.78.1269** | `/usr/share/VERSION` (partition APP) |
| `soft_version` | 3007 | `factory_cfg.ini` |
| SDK | V1.02.04 | `/etc/Version` |
| Compilation APP / ROOTFS | 2023-09-23 / 2023-08-05 | en-têtes squashfs |
| Noyau | Linux 4.4.192 « V2.1 » (2022-04-27) | `/proc/version` |
| SoC | Anyka AK3918EV330 (ARM926EJ-S) | `/proc/cpuinfo` |
| Capteur | SmartSens **SC2336**, MIPI 2 lanes, 1920×1080 | `sensor_driver.sh`, `dmesg` |
| Wi-Fi | Realtek RTL8188FTV (USB) | `lsmod` |
| Cloud d'origine | Tuya (`HT_IPC286KS2M_TUYA_AK3918EV330`) | chaînes de `anyka_ipc` |

La marque commerciale n'a pas été déterminée : seul le nom interne ci-dessus est établi.
Détails matériels complets : [`HARDWARE.md`](HARDWARE.md).

## Dépôt amont

| | Dépôt amont | Cette caméra |
|---|---|---|
| Firmware visé | `6.2863.123` (README amont) | **3.10.78.1269** |
| Capteur (`sensor_type_def`) | 17 = SC3332 MIPI | **20 = SC2336** (valeur d'usine) |
| Résolution principale | 2304×1296 | **1920×1080** (d'usine) |
| Codec principal / sous-flux | H.265 (`enc_out_type = 2`) | **H.264** (`0`) d'usine |
| Débit principal cible / max | 800 / 1600 kbit/s | 768 / 1024 kbit/s d'usine |
| QP principal min / max | 20 / 44 | 28 / 43 d'usine |
| `tilt_home_degree` | 98 | 110 d'usine |
| Détection humaine | — | `human_filter_support = 1` d'usine |

« d'usine » = `/usr/local/_ht_hw_settings.ini` extrait de la partition APP de cette caméra.
Le toolkit monte son propre `_ht_hw_settings.ini` par-dessus (`OVERRIDE_HW_SETTINGS=1`) : avec les valeurs
amont, l'encodeur de cette caméra refusait le canal 2304×1296 H.265 (`Channel creation failed`), d'où la boucle.

## Conséquences pour ce fork

- `main_width` / `main_height` passent à **1920 / 1080** (résolution réelle du capteur).
- Le flux est actuellement en **H.265**, hérité de la config amont. L'usine de cette caméra était en H.264 :
  repasser à `main_enc_out_type = 0` est possible (non testé ici).
- `sensor_type_def` est resté à **17** dans ce fork alors que l'usine indique **20**. La caméra fonctionne ainsi
  (le pilote capteur est chargé par `sensor_driver.sh`, indépendamment), mais la valeur 20 serait la bonne.
  **Non modifié, non testé.**
- Les patchs binaires de `tools/` ne sont valables que pour **les fichiers de ce firmware** (MD5 contrôlés) ;
  sur un autre firmware ils refusent de s'appliquer.
- Les adresses de code citées dans la doc (`0xb54e0`, `0xca9f4`…) sont propres au binaire `anyka_ipc` de 3.10.78.1269.
