# Audit de sécurité de la caméra (lecture seule)

Audit interne fait par le shell du toolkit, sans rien modifier, sur la caméra décrite dans [`CAMERA.md`](CAMERA.md).
Objectif de départ : couper le cloud Tuya et garder un flux limité au réseau local.
Les adresses du réseau de l'auteur sont volontairement absentes.

## Constats

| Gravité | Constat | État dans ce fork |
|---|---|---|
| Élevée | Shell root **sans mot de passe** sur le port 24 (`telnetd -l /bin/sh`), ouvert à tout le LAN. Le port 23 est un telnet root protégé par le `shadow` de la SD. | Inchangé. À désactiver après les tests : `TELNET=0` dans `hack.conf`. |
| Élevée | Aucun pare-feu (pas d'`iptables`). Ports ouverts sur `0.0.0.0` : 23, 24, 554 (RTSP), 8090 (`anyka_ipc`, rôle non identifié). | Inchangé. Filtrer sur le routeur. |
| Moyenne | Tout tourne en root ; `/etc/config/passwd` est en `rwxrwxrwx`. | Inchangé. |
| Moyenne | `wpa_supplicant.conf`, `tuya_user.db`, `tuya_enckey.db`, `_ht_uuid.ini` lisibles par tous. Le mot de passe Wi-Fi est en clair sur la SD **et dans les journaux** `logs/wifi.*.log`. | Inchangé. Vider `logs/` avant tout partage. |
| Moyenne | `recover_cfg.sh`, `anyka_ipc.sh` et désormais `state_led.sh` / `anyka_ipc_fps20b` sont montés depuis la SD (FAT, lecture/écriture) et exécutés en root. Quiconque a la SD ou le shell du port 24 obtient un accès root persistant. | **Aggravé** par ce fork (2 montages de plus, dont un binaire). |
| Faible | Le cloud est coupé par `/etc/hosts` vers `127.0.0.1`, mais pour 3 régions Tuya seulement (6 hôtes chacune) ; les DNS publics restent autorisés et la caméra garde sa route vers Internet. | Inchangé. Bloquer la sortie Internet sur le routeur. |
| Faible | Noyau 4.4.192 (2022) avec modules hors arbre ; pas de mise à jour du firmware. | Inchangé. |
| Faible | `anyka_ipc` : exécutable non-PIE, pas de RELRO, **pile exécutable** (`GNU_STACK RWE`) ; importe `system`, `popen`, `execvp`. | Inchangé. |
| Faible | L'OTA du firmware ne vérifie qu'un **MD5**, aucune signature : un firmware modifié est accepté. | Inchangé. Le risque est neutralisé tant que la caméra n'atteint pas Internet. |
| Info | Racine en lecture seule (squashfs), ASLR noyau actif, pas de `curl`/`wget`/`iptables` sur la caméra. | — |
| Info | Aucun mot de passe, jeton ou porte dérobée codé en dur trouvé dans les chaînes de `anyka_ipc` (analyse statique). | — |
| Info | **Pas de SSH** : ports 22 et 2222 fermés, aucun binaire `ssh`/`dropbear`. Le telnet envoie tout en clair. | — |

## Recommandations, par ordre d'efficacité

1. **Routeur** : bloquer tout le trafic sortant de la caméra vers Internet ; n'autoriser que son accès depuis votre PC/NVR vers le port 554. Idéalement un VLAN dédié aux objets connectés.
2. **Après les réglages** : `TELNET=0` dans `sd_card/custom/configs/hack.conf`, puis vérifier avec `nmap` que seuls 554 et 8090 restent ouverts.
3. **Wi-Fi** : changer la passphrase si elle a pu apparaître dans des journaux partagés ; envisager un SSID IoT séparé.
4. **Accès physique** : la SD donne un accès root ; la console UART (`ttySAK0`, 115200 8N1) aussi.
5. **RTSP sans authentification** : accessible à tout appareil du LAN tant qu'il n'est pas filtré.

## Limites

- Le RTSP n'a pas été testé avec authentification ; les connexions sortantes n'ont pas été listées (`netstat -tn`).
- Analyse statique du binaire seulement : aucune recherche de vulnérabilités mémoire n'a été faite.
