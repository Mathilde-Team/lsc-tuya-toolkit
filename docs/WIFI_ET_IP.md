# Wi-Fi et adresse IP : deux réglages qui coexistent

Le Wi-Fi est configuré à **deux endroits**, lus par **deux composants différents** qui tournent en parallèle au démarrage.
C'est la raison pour laquelle l'adresse IP de la caméra peut changer d'un démarrage à l'autre.

> Les valeurs réelles (SSID, mot de passe, adresses) ne figurent **jamais** dans ce dépôt. Les exemples ci-dessous
> utilisent des marqueurs : `<SSID>`, `<MOT_DE_PASSE>`, `<IP_FIXE>`, `<IP_DHCP>`.

## 1. Les deux fichiers

| | `sd_card/custom/configs/wifi.conf` | `sd_card/_ak39_factory.ini` |
|---|---|---|
| Section / clés | `MODE`, `SSID`, `PASS`, `SECURITY`, `DHCP` | `[wifi_station]` `ssid`, `passphrase`, `ip`, `netmask`, `gateway` |
| Lu par | `custom/scripts/wifi_apply.sh` (toolkit) | le **firmware d'origine** : `anyka_ipc`, module `ht_factory_test` |
| Chemin lu | `/tmp/sd/custom/configs/wifi.conf` | `/tmp/_ak39_factory.ini` (copie faite par `service.sh`) |
| Connexion Wi-Fi | `station_connect.sh <sécurité> <ssid> <mdp>` | `wifi_station.sh connect <ssid> <mdp>` |
| Adresse IP | **DHCP** (`udhcpc`), ou aucune si `DHCP=0` | **fixe** si `ip` est renseignée (`ifconfig wlan0 <ip> netmask <masque>` puis `route add default gw <passerelle>`) |
| Autres effets | — | sa simple **présence** active le mode usine : `telnetd`, RTSP `/main_ch`, lecture de `shadow` |

Le fichier `_ak39_factory.ini` est indispensable (c'est lui qui active le RTSP et le telnet du firmware) : on ne peut
pas le supprimer, seulement le renseigner ou le laisser avec des valeurs vides.

## 2. Ce qui a été constaté

Sur cette caméra, `_ak39_factory.ini` contenait une IP fixe et `wifi.conf` demandait le DHCP. Les deux s'exécutent
à chaque démarrage, et l'adresse obtenue variait d'un démarrage à l'autre.

Quatre démarrages complets ont été observés, journal `wifi.*.log` et adresse réellement obtenue :

| Démarrage | Fin du journal Wi-Fi | Adresse de la caméra |
|---|---|---|
| 1 | bail DHCP obtenu | `<IP_DHCP>` |
| 2 | bail DHCP obtenu | `<IP_DHCP>` |
| 3 | `Killed`, **aucun bail** | `<IP_FIXE>` (celle du fichier usine) |
| 4 | `Killed`, **aucun bail** | `<IP_FIXE>` |

Le firmware tue explicitement les clients DHCP : `wifi_station.sh stop` contient `killall udhcpc`. La dernière
configuration appliquée l'emporte.

Cause **probable** (non prouvée) : course entre `wifi_apply.sh` (DHCP) et le thread d'usine du firmware (IP fixe).
Remarque : les noms de fichiers de journaux utilisent l'horloge de la caméra, fausse au début du démarrage
(année 2000) : ne vous fiez pas à leur date pour les ordonner.

## 3. Recommandations

Garder **une seule source de vérité**.

**Option A, conseillée : DHCP + réservation sur le routeur**
1. `wifi.conf` : `MODE=sta`, `SSID`, `PASS`, `SECURITY=3`, `DHCP=1`.
2. `_ak39_factory.ini` : mêmes `ssid` / `passphrase`, et `ip=`, `netmask=`, `gateway=` **vides** (c'est le contenu du dépôt amont).
3. Sur le routeur : réserver une IP à l'adresse MAC de la caméra.
   *(Le comportement du firmware avec `ip` vide est celui du dépôt amont ; il n'a pas été retesté ici.)*

**Option B : IP fixe côté firmware, sans DHCP**
1. `_ak39_factory.ini` : `ip`, `netmask`, `gateway` renseignés.
2. `wifi.conf` : `DHCP=0` (le script journalise « DHCP disabled by config » et ne lance pas `udhcpc`).
3. Plus de course : seule l'IP fixe est appliquée.

Dans les deux cas, `ssid` et `passphrase` doivent être **identiques** dans les deux fichiers, puisque les deux
composants se connectent au Wi-Fi.

## 4. État de ce fork

`wifi.conf` et `_ak39_factory.ini` sont **identiques à ceux du dépôt amont** (sans SSID ni mot de passe) : ils sont à
renseigner localement, à la main, sur la carte SD. Aucun changement n'a été appliqué à la caméra pour ce point :
c'est une recommandation, pas une correction faite.

Prudence : ne copiez jamais votre `wifi.conf` ou votre `_ak39_factory.ini` rempli dans un commit. Les journaux
`sd_card/logs/` (déjà ignorés par `.gitignore`) contiennent aussi le SSID et le mot de passe en clair.
