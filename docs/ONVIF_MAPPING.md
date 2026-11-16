# Passerelle : correspondance ONVIF ↔ MQTT et plan de non-régression

Issue #60 (épopée #51). Pour chaque fonction de la table [TOPICS.md](TOPICS.md) : le service ONVIF qui peut la porter
(Imaging, PTZ, Events, Device, DeviceIO) ou « MQTT seul ». Règle de l'épopée : les deux canaux passent par
`/tmp/md_cmd` (une ligne `<fonction> <valeur>`), lu par `md_inject.so` dans `anyka_ipc`, qui appelle la fonction du
firmware et publie l'état retained `<base>/<fonction>`. Ainsi l'état MQTT reste vrai quel que soit le canal utilisé.

Ce document ne contient aucune nouvelle affirmation sur le firmware : il croise la table de TOPICS.md (générée depuis
le désassemblage, `docs/firmware/`) avec ce que `onvif_simple_server` sait faire réellement (source lue :
roleoroleo/onvif_simple_server, commit `7b3941a`, plus les correctifs `tools/onvif/*.patch`). Les valeurs et plages
des DP restent « à confirmer » tant que les lots #54 à #58 ne les ont pas établies.

## 1. Ce que onvif_simple_server peut transmettre

| Service ONVIF | Opération | Mécanisme côté serveur (source) | Branchement sur cette caméra |
|---|---|---|---|
| Imaging | `SetImagingSettings` | `imaging_set_imaging_settings()` (`imaging_service.c`) : exécute `imaging_set_command` avec les variables d'environnement `ONVIF_IMAGING_BRIGHTNESS`, `_CONTRAST`, `_COLOR_SATURATION`, `_SHARPNESS`, `_IRCUT_FILTER`, `_WHITE_BALANCE`, `_EXPOSURE`, `_WIDE_DYNAMIC_RANGE`, `_AUTO_FOCUS_MODE` | `scripts/set_imaging` : seul `IrCutFilter` est traité (ON → `night day`, OFF → `night night`, AUTO → `night auto`, écrit dans `/tmp/md_cmd`) |
| Imaging | `GetImagingSettings` | renvoie les valeurs **statiques** de la configuration (`imaging_ircut_on`, `imaging_brightness`…), pas l'état réel | `imaging_ircut_on=1` : renvoie toujours ON |
| PTZ | `ContinuousMove`, `Stop`, `AbsoluteMove`, `RelativeMove`, `GotoPreset`, `SetPreset`, `RemovePreset`, `GotoHomePosition`, `SetHomePosition`, `GetStatus` | commandes `move_*`, `jump_to_abs/rel`, `*_preset`, `goto_home_position`, `get_position`, `is_moving` de la configuration | `scripts/ptz_move`, `ptz_presets.sh`, `get_position`, `is_moving` (pilote moteur direct, pas les DP 116/119) |
| PTZ | PresetTour (rondes) | non implémenté par onvif_simple_server | — |
| Device | `SystemReboot` | `device_system_reboot()` → `reboot_thread()` (`utils.c`) : `sync` puis redémarrage, interne au serveur | actif ; ne passe pas par le DP 162 |
| Device | `SetSystemFactoryDefault` | `device_set_system_factory_default()` : exécute `factory_default_command` (refusé si vide) | non configuré, volontairement |
| DeviceIO | `SetRelayOutputState` | commande `close=` / `open=` du relais (sans paramètre : uniquement deux états) | relais 0 = mode privé, appelle `privacy.sh on/off` **directement** (voir §3) |
| Events | PullPoint / BaseSubscription | lit une mémoire partagée remplie par `onvif_notify_server` (`events_service.c` : « No shared memory found, is onvif_notify_server running? ») | `events=0` : `onvif_notify_server` n'est pas déployé (docs/ONVIF.md) |
| Media | OSD, rotation (`SetOSD`, `SetVideoSourceConfiguration`) | non implémentés (`SetVideoSourceConfiguration` ne fait que renvoyer une faute si `adv_fault_if_set=1`) | — |
| Analytics / Recording | règles de détection, enregistrement carte SD | non implémentés | — |

Conséquence : seuls les **booléens** peuvent passer par un relais DeviceIO, seules les fonctions d'image listées ci-dessus
par Imaging, et Events ne peut servir qu'à des **sorties** (la caméra signale), jamais à des réglages.

## 2. Correspondance par fonction

Colonnes : **ONVIF** = service qui peut porter la fonction ; **Ligne `/tmp/md_cmd`** = ce que le canal ONVIF écrirait
(format `<fonction> <valeur>` de #53) ; **Aujourd'hui** = état sur `dev`. « MQTT seul » = aucun service ONVIF standard
pris en charge par onvif_simple_server ne correspond. Aucun DP n'est exposé tant que son lot n'a pas rempli
`docs/firmware/DP_VALUES.md` (#53).

### Fonctions déjà disponibles (alias hors table)

| Fonction | MQTT | ONVIF | Ligne `/tmp/md_cmd` | Aujourd'hui |
|---|---|---|---|---|
| Mode nuit (`ht_night_mode_set_irled_mode` 0 auto / 1 jour / 2 nuit) | `<base>/night/set` auto/day/night | Imaging `IrCutFilter` ON/OFF/AUTO | `night day` / `night night` / `night auto` | les deux canaux passent par `/tmp/md_cmd` |
| Mode privé (`privacy.sh`) | `<base>/privacy/set` ON/OFF | DeviceIO relais 0 close/open | `privacy on` / `privacy off` | via `privacy_relay.sh` : état MQTT à jour (§3) |
| Détection de mouvement | `MOTION_TOPIC`, défaut `camera/motion` (sortie) | Events `tns1:VideoSource/MotionAlarm` | — (sortie) | MQTT seul (`events=0`) |
| Redémarrage | — | Device `SystemReboot` | — | ONVIF seul, redémarrage interne au serveur |
| Orientation, préréglages | — | PTZ | — | ONVIF seul, via `ptz_move` |

### DP de la table (61 fonctions)

| DP | Topic | ONVIF | Ligne `/tmp/md_cmd` | Lot |
|---|---|---|---|---|
| 101 | `light_onoff` | DeviceIO relais (bool) si le matériel existe | `light_onoff ON` | #58 |
| 103 | `flip_onoff` | MQTT seul (rotation Media non gérée) | — | #54 |
| 104 | `watermark_onoff` | MQTT seul (OSD Media non géré) | — | #54 |
| 105 | `sleep_mode` | MQTT seul (enum ; distinct du mode privé, voir NIGHT_AND_PRIVACY.md) | — | #58 |
| 106 | `alarm_sensitivity` | MQTT seul (Analytics non géré) | — | #55 |
| 108 | `night_mode` | Imaging `IrCutFilter` si #58 confirme que les valeurs recouvrent auto/jour/nuit ; sinon l'alias `night` reste le seul branché | `night_mode <v>` | #58 |
| 109 | `sd_storage` | MQTT seul (lecture) | — | #56 |
| 110 | `sd_status` | MQTT seul (lecture) | — | #56 |
| 111 | `format_sd_card` | MQTT seul (action destructive : pas de relais) | — | #56 |
| 112 | `unmount_sd_card` | MQTT seul | — | #56 |
| 116 | `ptz_stop_move` | PTZ `Stop` couvert par `ptz_move -m stop` ; ne pas doubler | — | #57 |
| 117 | `sd_format_status` | MQTT seul (lecture) | — | #56 |
| 119 | `ptz_start_move` | PTZ `ContinuousMove` couvert par `ptz_move` ; ne pas doubler | — | #57 |
| 120 | `alarm_siren_onoff` | DeviceIO relais (bool) | `alarm_siren_onoff ON` | #55 |
| 124 | `night_vision_mode` | MQTT seul tant que #58 n'a pas établi la différence avec le DP 108 | — | #58 |
| 132 | `ptz_check` | MQTT seul (calibrage ; `ptz_calibrate.sh` existe déjà) | — | #57 |
| 134 | `alarm_function_onoff` | DeviceIO relais (bool) possible ; sinon MQTT seul | `alarm_function_onoff ON` | #55 |
| 138 | `blub_onoff` | DeviceIO relais (bool) si le matériel existe | `blub_onoff ON` | #58 |
| 139 | `sound_detect_onoff` | MQTT seul (réglage) ; la détection elle-même → Events `DetectedSound` si `onvif_notify_server` est déployé | — | #55 |
| 140 | `sound_sensitivity` | MQTT seul | — | #55 |
| 145 | `battery_percent` | MQTT seul (lecture) | — | #58 |
| 146 | `power_mode` | MQTT seul (lecture) | — | #58 |
| 150 | `sd_record_onoff` | MQTT seul (Recording non géré) | — | #56 |
| 151 | `sd_record_mode` | MQTT seul | — | #56 |
| 152 | `pir_detect_switch` | MQTT seul | — | #58 |
| 158 | `floodlight_lightness` | MQTT seul (entier : un relais n'a que deux états) | — | #58 |
| 159 | `siren_onoff` | DeviceIO relais (bool) | `siren_onoff ON` | #55 |
| 160 | `ipc_lullaby_volume` | non exposé (voir `lullaby/set`) | — | #58 |
| 161 | `track_enable` | MQTT seul (lecture) | — | #57 |
| 162 | `system_reboot` | Device `SystemReboot` existe déjà sans ce DP ; à garder tel quel | — | #54 |
| 167 | `cry_detect_onoff` | MQTT seul (réglage) ; détection → Events `BabyCrying` si `onvif_notify_server` est déployé | — | #55 |
| 168 | `alarm_zone_onoff` | MQTT seul | — | #55 |
| 169 | `alarm_zone_draw` | MQTT seul | — | #55 |
| 170 | `human_filter` | MQTT seul (réglage) ; détection → Events `PeopleDetect` si `onvif_notify_server` est déployé | — | #55 |
| 172 | `flight_warn_onoff` | MQTT seul | — | #58 |
| 173 | `flight_warn_time` | MQTT seul | — | #58 |
| 174 | `patrol_switch` | MQTT seul (PresetTour non géré) | — | #57 |
| 175 | `patrol_mode` | MQTT seul | — | #57 |
| 176 | `patrol_tmode` | MQTT seul | — | #57 |
| 177 | `patrol_time` | MQTT seul | — | #57 |
| 178 | `preset` | PTZ `GotoPreset` couvert par `ptz_presets.sh` (préréglages du toolkit, pas ceux du firmware) ; MQTT seul pour le DP | — | #57 |
| 179 | `patrol_state` | MQTT seul (lecture) | — | #57 |
| 188 | `basic_anti_flicker` | MQTT seul (aucun champ anti-scintillement dans ImagingSettings) | — | #54 |
| 190 | `link_move` | MQTT seul | — | #57 |
| 198 | `object_outline_onoff` | MQTT seul | — | #54 |
| 199 | `link_pos` | MQTT seul | — | #57 |
| 232 | `ipc_lullaby_mode` | non exposé (voir `lullaby/set`) | — | #58 |
| 234 | `ipc_lullaby_control` | non exposé (voir `lullaby/set`) | — | #58 |
| 235 | `ipc_lullaby_list` | non exposé (voir `lullaby/set`) | — | #58 |
| 236 | `ipc_lullaby_command` | non exposé (voir `lullaby/set`) | — | #58 |
| 238 | `temp_report_c` | MQTT seul (lecture) | — | #58 |
| 239 | `temp_report_f` | MQTT seul (lecture) | — | #58 |
| 240 | `temp_unit_select` | MQTT seul | — | #58 |
| 241 | `temp_max_onoff` | MQTT seul | — | #58 |
| 242 | `temp_min_onoff` | MQTT seul | — | #58 |
| 243 | `temp_max_cvalue` | MQTT seul | — | #58 |
| 244 | `temp_min_cvalue` | MQTT seul | — | #58 |
| 245 | `temp_max_fvalue` | MQTT seul | — | #58 |
| 246 | `temp_min_fvalue` | MQTT seul | — | #58 |
| 247 | `audible_alarm_onoff` | DeviceIO relais (bool) | `audible_alarm_onoff ON` | #55 |
| 248 | `door_control` | MQTT seul | — | #58 |

Les six DP sans fonction nommée (136, 141, 142, 143, 147, 233) n'ont pas de correspondance tant que leur gestionnaire
n'est pas compris.

Bilan : 1 fonction par Imaging (DP 108, sous condition), 6 booléens candidats à un relais DeviceIO (101, 120, 134, 138,
159, 247), 3 sorties candidates à Events (139, 167, 170, sous réserve de `onvif_notify_server`), 4 DP déjà couverts
autrement par ONVIF (116, 119, 162, 178), le reste MQTT seul.

## 3. Règles de branchement ONVIF

1. **Toujours écrire dans `/tmp/md_cmd`**, jamais appeler le firmware ou un script directement : c'est
   `md_inject.so` qui publie l'état retained. Exemple pour un relais : `close=echo "siren_onoff ON" > /tmp/md_cmd`.
2. **Relais 0** : `close`/`open` appellent `scripts/privacy_relay.sh on|off`, qui écrit `privacy on|off` dans `/tmp/md_cmd` (état MQTT et mémoire des réglages à jour) ; sans MQTT le script appelle `privacy.sh` directement.
3. Un relais = un booléen. `relay_outputs` doit lister les relais dans l'ordre ; ajouter un relais n'est utile que
   si le DP est exposé dans `DP_VALUES.md`.
4. `GetImagingSettings` ne lit pas l'état : un client ONVIF peut afficher ON alors que la caméra est en auto. C'est une
   limite du serveur, l'état réel est `<base>/night`.
5. Actions destructives (`format_sd_card`, `SetSystemFactoryDefault`) : jamais exposées en ONVIF.
6. Events : n'activer `events=` qu'après déploiement de `onvif_notify_server` et branchement de ses fichiers d'entrée
   (`/tmp/onvif_notify_server/motion_alarm`, `human_detection`, `sound_detection`, `baby_crying`).

## 4. Plan de tests pour Mathilde (caméra réelle)

Aucun de ces tests n'a été joué par la passerelle elle-même : ils sont à faire sur la caméra. Adapter `192.0.2.50`
(caméra), `192.0.2.10` (broker) et `<base>`. Ouvrir un terminal d'écoute pendant tous les tests :

```sh
mosquitto_sub -h 192.0.2.10 -u <utilisateur> -P <mot_de_passe> -v -t '<base>/#'
```

Les opérations ONVIF se font avec un client ONVIF (ONVIF Device Manager, intégration ONVIF de Home Assistant…).

### Non-régression (à rejouer à chaque PR passerelle)

- [ ] R1 Démarrage : après redémarrage, le flux RTSP `rtsp://192.0.2.50:554/main_ch` s'ouvre, le port ONVIF 8080
      répond (`GetDeviceInformation`) et `md_inject` se connecte au broker (journal de `anyka_ipc`).
- [ ] R2 Nuit par MQTT : publier `auto`, `day`, `night` sur `<base>/night/set` → effet visible (LED IR / filtre) et
      `<base>/night` retained égal à la valeur envoyée. Valeur invalide (`foo`) → aucun changement.
- [ ] R3 Nuit par ONVIF : `SetImagingSettings` IrCutFilter ON, OFF, AUTO → même effet que R2 **et** `<base>/night`
      passe à `day`, `night`, `auto`.
- [ ] R4 Privé par MQTT : `ON` puis `OFF` sur `<base>/privacy/set` → image coupée/rétablie, `<base>/privacy` suit.
- [ ] R5 Privé par ONVIF : relais 0 close puis open → image coupée/rétablie. Noter si `<base>/privacy` change
      (attendu aujourd'hui : non, écart §3.2).
- [ ] R6 PTZ ONVIF : déplacement continu dans les 4 directions puis Stop, AbsoluteMove, enregistrement et rappel d'un
      préréglage, GotoHomePosition, GetStatus (position cohérente).
- [ ] R7 Mouvement : passer devant la caméra → `MOTION_TOPIC` (défaut `camera/motion`) ON puis OFF.
- [ ] R8 Redémarrage ONVIF : `SystemReboot` → la caméra redémarre et R1 repasse.
- [ ] R9 Persistance : après R8, `<base>/night` et `<base>/privacy` retained correspondent à l'état réel.
- [ ] R10 Sans MQTT (`MOTION=0` dans `hack.conf`) : R3, R5 et R6 fonctionnent encore ; aucune erreur dans les
      journaux de `onvif_simple_server`.
- [ ] R11 Home Assistant (si #59 est fusionné) : les entités apparaissent, changer la nuit et le privé depuis HA met
      à jour la caméra et l'état.

### Pour chaque DP exposé par un lot (#54 à #58)

À recopier dans la PR du lot, une ligne par DP rendu « Exposé = oui » :

- [ ] D1 `mosquitto_pub -t '<base>/<topic>/set' -m <valeur>` pour chaque valeur documentée dans `DP_VALUES.md` →
      effet visible ou audible, et `<base>/<topic>` retained égal à la valeur.
- [ ] D2 Valeur hors plage ou inconnue → refusée, état inchangé, aucun plantage d'`anyka_ipc`.
- [ ] D3 Même commande écrite à la main : `echo "<topic> <valeur>" > /tmp/md_cmd` (telnet) → même résultat que D1.
- [ ] D4 Si le DP a un équivalent ONVIF (§2) : commande par ONVIF → même effet et même état MQTT que D1.
- [ ] D5 Capteur (lecture seule) : la valeur publiée correspond à l'application constructeur ou à la réalité.
- [ ] D6 Après redémarrage : l'état publié correspond à l'état réel.
- [ ] D7 Rejouer R2 à R7 : rien n'a régressé.
