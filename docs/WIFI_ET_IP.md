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

## 3. Test de l'option « DHCP partout » : ça ne marche pas (1 essai)

Le contenu du dépôt amont (`ip=`, `netmask=`, `gateway=` **vides**) a été essayé sur cette caméra. Résultat d'un démarrage,
d'après `anyka.*.log` : le firmware lit bien `camera_ip` vide, puis tente de se reconnecter en boucle
(`wifi has not connect, need reconnect`, `network_disconnect_times` de 1 à 5, puis de nouveau 1…).
Aucun relevé « connecté » sur les ~100 s du journal, alors que le démarrage précédent (IP fixe) était connecté **21 s** après son début.
Côté toolkit, `udhcpc` reçoit une offre (`sending select`) puis le journal se termine par `Killed`, sans bail.

Limites : un seul démarrage ; le journal s'arrête à ce moment (arrêt de la caméra ou retrait de la SD, non déterminé), donc
une connexion plus tardive n'est pas exclue. La cause (la reconnexion du firmware tue le DHCP du toolkit) est **plausible, non prouvée**.

## 4. Configuration retenue : IP fixe, à régler à deux endroits

Garder **une seule source d'adresse**, mais elle se règle dans **deux fichiers** :

| Fichier | Réglage | Rôle |
|---|---|---|
| `_ak39_factory.ini` | `ip=<IP_FIXE>`, `netmask=<MASQUE>`, `gateway=<PASSERELLE>` | **fixe l'adresse** : le firmware applique `ifconfig wlan0 <ip> netmask <masque>` puis `route add default gw <passerelle>` |
| `wifi.conf` | `DHCP=0` | **empêche le toolkit de lancer `udhcpc`** : sans cela, un second mécanisme cherche une adresse et les deux se disputent l'interface |

`wifi.conf` n'a pas de champ pour l'adresse : le script `wifi_apply.sh` s'arrête simplement à « DHCP disabled by config ».
L'adresse n'est donc **écrite qu'à un seul endroit** (`_ak39_factory.ini`) ; l'autre fichier doit seulement lui laisser la place.

Dans les deux fichiers, `ssid` / `passphrase` (`SSID` / `PASS` dans `wifi.conf`) doivent être **identiques**, puisque les deux
composants se connectent au Wi-Fi.

Précautions :
- Choisir une adresse **hors de la plage DHCP du routeur** (ou la réserver à l'adresse MAC de la caméra) pour éviter un conflit.
- `netmask` et `gateway` doivent correspondre à votre réseau : une passerelle fausse coupe l'accès à Internet **et** au réseau local au-delà du sous-réseau.
- Sans `udhcpc`, la caméra n'obtient plus de serveur DNS par DHCP ; elle utilise le `resolv.conf` du firmware. Cela n'affecte pas le RTSP.
- Le retour à un DHCP complet (`ip` vide dans le fichier usine) n'a **pas fonctionné** lors du test (§3) : en cas de doute, rester sur l'IP fixe.

## 5. État de ce fork et de la carte SD de l'auteur

Dans le dépôt, `wifi.conf` et `_ak39_factory.ini` sont **identiques à ceux de l'amont** (sans SSID, mot de passe ni adresse) :
ils sont à renseigner localement sur la carte SD, à la main.

Sur la carte de l'auteur : `_ak39_factory.ini` est revenu à l'IP fixe d'origine (octet pour octet) et `wifi.conf` est passé de
`DHCP=1` à `DHCP=0`. **Cette configuration n'a pas encore été testée sur la caméra** au moment de la rédaction ; ce document sera
complété par le résultat.

Prudence : ne copiez jamais votre `wifi.conf` ou votre `_ak39_factory.ini` rempli dans un commit. Les journaux
`sd_card/logs/` (déjà ignorés par `.gitignore`) contiennent aussi le SSID et le mot de passe en clair.
