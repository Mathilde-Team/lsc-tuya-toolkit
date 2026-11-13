# Installation sur la carte SD (installeur `install.py`)

Caméra visée : modèle **3007325** (référence imprimée sous la caméra et sur le code-barres), interne `IPC-XC-C323`,
firmware **3.10.78.1269**, SoC Anyka AK3918EV330. Courante en France et aux Pays-Bas, souvent vendue moins de 20 € d'occasion.
Autre modèle ou autre firmware : lisez d'abord [`CAMERA.md`](CAMERA.md), les fichiers ne sont pas interchangeables.

L'installeur copie `sd_card/` sur la carte puis écrit les réglages (Wi-Fi, IP fixe, heure, syslog, ONVIF, services)
à partir de vos réponses ou d'un fichier YAML. Aucune dépendance : Python 3 seul (PyYAML est utilisé s'il est présent).

## Prérequis

- Une carte microSD en **FAT32** (32 Go maximum conseillé), lisible sur votre PC.
- Un Wi-Fi **2,4 GHz** (la puce Wi-Fi de la caméra ne gère pas le 5 GHz).
- Une adresse IP libre **hors de la plage DHCP** de votre routeur.
- Linux (ou macOS avec un point de montage). Pour donner un `/dev/sdX1` non monté, il faut les droits de `mount` (`sudo`) ;
  sinon montez la carte vous-même et donnez son point de montage.

Flux vidéo : réglés dans `_ht_hw_settings.ini` (`*_enc_out_type`, `*_width`, `*_height`), et le profil ONVIF est aligné
sur le flux principal. Mesures sur ce modèle : H.265 en 1920×1080 = 20 images/s ; H.264 = ~11 images/s ; MJPEG non testé ;
au-dessus de 1920×1080 (2304×1296) la caméra bouclait au redémarrage. Le sous-flux RTSP `/sub_ch` ne fonctionne pas sur ce firmware.

## 1. Préparer le fichier de réglages

```
cp install.example.yaml install.yaml
chmod 600 install.yaml
```

Éditez `install.yaml`. Tous les champs sont commentés. Résumé :

| Section | Réglages |
|---|---|
| `target` | partition FAT (`/dev/sdb1`) ou point de montage ; vide = liste des cartes amovibles détectées |
| `network` | `ssid`, `password`, `security` (3 = WPA/WPA2), `dhcp`, `ip`, `netmask`, `gateway` |
| `time` | `tz` (chaîne POSIX), `ntp_server` (vide = pas de NTP) |
| `syslog` | `server` au format `hote:port`, UDP (vide = désactivé) |
| `camera` | `hostname`, `ptz_home_on_boot`, `mute_factory_prompt`, `logs_keep` (rotation des journaux de démarrage, 0 = tout garder), `privacy_preset` (preset « position privée », 0 = désactivé — voir `docs/PRIVACY.md`) |
| `onvif` | `enabled`, `user`, `password` (`auto` = généré, vide = sans authentification), `ptz` |
| `security` | `root_password` (`auto` = généré ; écrit dans `shadow` sur la SD) |
| `services` | `telnet` (`open` / `login` / `off`), `ftp`, `offline` (coupe le cloud Tuya) |
| `video` | `main_codec` / `sub_codec` (0 = H.264, 1 = MJPEG, 2 = H.265), `main_width` / `main_height`, `sub_width` / `sub_height` |
| `firmware` | `dump_dir` : dossier du dump de **votre** caméra (étape 4) |

Sans fichier, `python3 install.py` pose les mêmes questions une par une et propose d'enregistrer `install.yaml`.
`install.yaml` est ignoré par git : il contient vos mots de passe, ne le publiez jamais.

## 2. Lancer l'installeur

```
python3 install.py --config install.yaml --dry-run     # vérifie et montre, n'écrit rien
sudo python3 install.py --config install.yaml          # écrit sur la carte (confirmation : taper OUI)
python3 install.py --config install.yaml --yes         # sans question (pour un script)
```

L'installeur :
- n'accepte comme périphérique qu'une partition **FAT amovible** (le disque système est refusé) ;
- vérifie la configuration (SSID, mot de passe WPA de 8 à 63 caractères, IP, masque et passerelle cohérents) et refuse
  les valeurs d'exemple ;
- copie les fichiers (sans `logs/` ni `state/`), puis écrit `_ak39_factory.ini`, `wifi.conf`, `hack.conf` et
  `onvif_simple_server.conf`. Les mots de passe sont protégés pour les fichiers lus par le shell de la caméra ;
- si `onvif.password` vaut `auto`, affiche le mot de passe généré **une seule fois** ;
- attend la fin des écritures et démonte la carte s'il l'a montée.

## 3. Premier démarrage

1. Caméra débranchée, insérez la carte, puis branchez-la.
2. Attendez environ 1 minute. Elle doit répondre sur l'IP choisie :
   ```
   ping <IP>
   ffplay rtsp://<IP>:554/main_ch          # vidéo H.265 1080p et son AAC
   telnet <IP> 24                          # seulement si services.telnet est open ou login
   ```
3. ONVIF : `http://<IP>:8080/onvif/device_service` (utilisateur et mot de passe choisis).
   Home Assistant : intégration **ONVIF**, port 8080.
4. Si la caméra reste muette : retirez la carte, lisez `logs/` (attention : `wifi.*.log` contient le SSID et le mot de
   passe en clair). Voir aussi [`WIFI_ET_IP.md`](WIFI_ET_IP.md).

## 4. Vidéo 20 images par seconde

Le firmware d'origine plafonne à 15 images/s. Le gain demande de patcher `anyka_ipc` et le fichier ISP **de votre propre
caméra** (ces fichiers du fabricant ne sont pas redistribués ici, voir [`CHANGES.md`](CHANGES.md) §6 et §7).

**Firmware 3.10.78 (celui testé) : rien à faire.** Au démarrage, `custom/scripts/patch_fw.sh` contrôle le MD5 des fichiers
de la caméra, fabrique `anyka_ipc_fps20b` et `isp_sc2336_fps20.conf` sur la carte et les monte aussitôt (désactivable avec
`AUTO_PATCH_FW=0` dans `hack.conf`). Si un MD5 diffère, rien n'est patché et la caméra reste d'origine ; voir `custom/state`
et les journaux `logs/run.*.log` (lignes `[patch_fw]`).

Autre firmware ou contrôle depuis le PC (le patch n'est alors appliqué que si les octets attendus sont présents) :

1. Au tout premier démarrage (pas de `custom/state/dump.done` sur la carte), le toolkit copie le système de la caméra
   dans `dump/` **avant** d'appliquer ses montages : les fichiers sont donc ceux d'origine.
2. Éteignez la caméra, remettez la carte dans le PC :
   ```
   sudo python3 install.py --patch-firmware --target /dev/sdX1
   ```
   Autre possibilité : copier le dump sur le PC, renseigner `firmware.dump_dir` et relancer l'installeur complet.
3. Redémarrez la caméra (`reboot`, ne tuez pas `anyka_ipc`).

## 4 bis. Haut-parleur par le LAN (facultatif)

Section `speaker:` du YAML (`enabled`, `allow`, `gain`, `port`). La caméra ouvre alors un port UDP qui reçoit du G.711 µ-law
8 kHz ; **seules les IP de `allow` (liste séparée par des virgules) sont acceptées** (le port n'a pas d'authentification : indiquez votre Home Assistant et fermez le
port ailleurs). Envoi d'un son : `tools/talk/send_audio.py <ip> sirene.wav --max 10`. Détails : `tools/talk/README.md`.

## 4 ter. Détection de mouvement vers MQTT (facultatif, opérationnelle)

Section `motion:` du YAML (`enabled`, `topic`, `hold`, `sensitivity`, `broker_host`, `broker_port`, `broker_user`, `broker_password`).
La caméra publie `ON` au premier mouvement et `OFF` après `hold` secondes sans mouvement (MQTT 3.1.1, QoS 1, retain, sans TLS).
Le mot de passe du broker est écrit uniquement sur la SD (`custom/configs/mqtt.cred`, mode 600). `CLOCKFIX=1` (par défaut) empêche le saut de +8 h de l'horloge.
Côté Home Assistant : `binary_sensor` MQTT avec `payload_on: "ON"` et `payload_off: "OFF"` sur le topic choisi.
La détection est validée sur une caméra réelle ; le filtre humain (`MOTION_HUMAN`) n'est pas encore fiable. Détails : `tools/motion/README.md`.

## 5. Après les réglages : durcir

- **Mot de passe root** : `security.root_password` est écrit (hash MD5-crypt) dans le fichier `shadow` de la SD, que le
  firmware recopie à chaque démarrage. Il protège le telnet du firmware (port 23). Un `passwd` fait sur la caméra
  serait perdu au redémarrage : changez-le toujours ici. Sans cela, le mot de passe est celui, public, du dépôt.
- **Telnet** : `services.telnet: open` donne un shell **root sans mot de passe** (port 24) à tout le LAN, pratique pour
  régler. Passez ensuite à `login` (root + mot de passe) puis à `off`. Gardez `login` tant qu'un accès de secours est utile.
- Gardez une authentification ONVIF. Le flux RTSP `/main_ch`, lui, n'a **pas** de mot de passe : isolez la caméra
  (VLAN, ou règle du routeur qui bloque son accès à Internet).
- `services.offline: true` coupe les connexions vers le cloud Tuya ; bloquez-les aussi sur le routeur.

## Retour arrière

Retirez la carte : la caméra redémarre sur son firmware d'origine, rien n'est écrit dans sa mémoire flash.
