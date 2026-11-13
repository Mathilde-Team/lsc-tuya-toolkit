# md_inject : détection locale (mouvement / humain) → MQTT

Bibliothèque `LD_PRELOAD` pour `anyka_ipc`. Le firmware initialise la détection mais ne l'active que
depuis le service Tuya : sans cloud, `md_inject.so` appelle `ht_montion_detect_set_active_cb()` puis
`ht_montion_detect_open()` et publie `ON` / `OFF` en MQTT 3.1.1 (QoS 1, retain).

## Variables

| Variable | Rôle | Défaut |
|---|---|---|
| `MOTION_MQTT_FILE` | fichier 4 lignes : IP, port, utilisateur, mot de passe (mode 600) | journal seul |
| `MOTION_TOPIC` | topic MQTT | `camera/motion` |
| `MOTION_HOLD` | secondes sans mouvement avant `OFF` | 10 |
| `MOTION_SENS` | sensibilité firmware 0..2 | celle du firmware |
| `MOTION_HUMAN` | `1` active le filtre humain, `0` le coupe | inchangé (désactivé : `bool_human_filter_enable = 0`) |

## Types de détection du firmware

- **Mouvement** (`ak_md`) : zones d'image qui changent. Sert de déclencheur, sensible aux changements de lumière.
- **Filtre humain** (`liblvhsr.so`, LinkView `hsr_v2.0.b20200520D`) : cherche des silhouettes humaines
  (`Detected %d human shapes`). Activé par `ht_montion_detect_set_human_filter()`.
  Support matériel : `human_filter_support = 1` dans `_ht_hw_settings.ini`.
- Pas de PIR (symboles présents, pas de capteur). Détection de pleurs/son : clés `bool_cry_detect_enable`, `bool_sound_detect_enable`, non testées.

## Seuils observés (journal de la caméra)

- Image analysée : 640×360 ; `max_rects 1` ; mode asynchrone désactivé.
- Seuil d'inférence initial (`inference_threshold`) : 228 en mode jour ; `ht_hsr_detect_set_day_night` ajoute/retire 20 selon jour/nuit.
- La sensibilité 0/1/2 choisit une clé de seuil différente (`ht_hsr_detect_init`) ; les changements jour/nuit sont journalisés (`night--->day`).

## Compilation / test

`./build.sh` (uClibc : `-nostdlib`, symboles vérifiés). Journal : lignes `[md_inject] … human filter set`.

## Résultats du test `MOTION_HUMAN=1` (une caméra, de jour, sensibilité du firmware)

Les événements se lisent dans le journal de la caméra (`[md_inject] t=… ON/OFF`), plus fiable qu'un abonné MQTT de passage.

| Action | Heure | Événement |
|---|---|---|
| 3 passages à 20 s d'intervalle | 17:45:00–17:45:40 | `ON` 17:45:11, `OFF` 17:46:01 (une seule alerte, maintien 30 s) |
| 1 passage (humain) | 17:50:00 | `ON` 17:50:28 |
| objet qui bouge (rideau, main) | 17:52:00 | `ON` 17:52:21 |

Constat : le `ON` arrive 10 à 28 s après le geste et l'objet déclenche aussi. Aucune différence humain/objet
démontrée ; le filtre n'ajoute pas de topic « human » (même `ON`/`OFF`). Sensibilités 0/1/2 et seuil jour/nuit non encore comparés.

## À tester pour le filtre humain (code déjà écrit : `MOTION_HUMAN`, `MOTION_SENS`)

Le code est en place (`md_inject.c`, `hack.conf`, `anyka_ipc.sh`, installeur `motion.human`). Reste à mesurer, un réglage par cycle, en lisant les lignes `[md_inject] t=… ON/OFF` du journal de la caméra (l'abonné MQTT ponctuel manque des événements) :

1. **Objet seul vs humain** : rideau, feuille, main seule, animal, ombre/lumière, contre 3 passages d'une personne entière, avec `MOTION_HUMAN=0` puis `1`.
2. **Sensibilité** `MOTION_SENS` 0, 1, 2 avec le filtre actif : taux de `ON` sans passage (30 min) et passages manqués.
3. **Jour / nuit** : même protocole de nuit (infrarouge). Le seuil d'inférence vaut 228 le jour ; `ht_hsr_detect_set_day_night` ajoute/retire 20 selon le mode (journal `lv hsr SET day night mode`).
4. **Délai** : mesurer le temps entre le geste et le `ON` (10 à 28 s observés) avec et sans filtre ; chercher la cause (analyse asynchrone, intervalle `ht_montion_detect_set_interval`).
5. **Topic « humain » séparé** : si le filtre ne bloque pas le mouvement simple, publier le résultat de `lv_hsr_process` (`Detected %d human shapes`, `ht_hsr_detect_send`) sur un second topic.
6. Autres clés non testées : `bool_cry_detect_enable`, `bool_sound_detect_enable`, zone `bool_md_area_enable` / `str_md_area_rect`.
