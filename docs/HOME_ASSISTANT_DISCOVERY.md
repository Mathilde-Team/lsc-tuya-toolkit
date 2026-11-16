# Home Assistant : découverte MQTT automatique (#59)

Avec la détection MQTT active (`MOTION=1`, `configs/mqtt.cred`), `md_inject.so` annonce la caméra à Home Assistant
dès son premier abonnement au broker : un **appareil** par caméra et ses entités apparaissent seuls dans
*Paramètres → Appareils et services → MQTT*. Aucun YAML à écrire côté HA. Complète l'intégration globale (#26).

## Prérequis

- Intégration **MQTT** de Home Assistant configurée sur le même broker, découverte activée (par défaut, préfixe `homeassistant`).
- `hack.conf` : `HA_DISCOVERY=1` (défaut) et `HA_PREFIX="homeassistant"` ; installeur : `motion.ha_discovery`.
- Un `MOTION_TOPIC` **différent par caméra** (ex. `camera188/motion`, `camera187/motion`) : il fixe `<base>` et l'identifiant de l'appareil.

## Entités annoncées

Topic de découverte : `<prefix>/<composant>/<nœud>/<objet>/config`, publié **retained**. `<nœud>` = `<base>` avec tout caractère hors `[A-Za-z0-9_-]` remplacé par `_` (ex. `camera188`, `maison/salon` → `maison_salon`).

| Entité | Composant HA | État | Commande | Toujours |
|---|---|---|---|---|
| Mouvement | `binary_sensor` (`motion`) | `MOTION_TOPIC` (`ON`/`OFF`) | — | oui |
| Jour/nuit | `select` (`auto`, `day`, `night`) | `<base>/night` | `<base>/night/set` | oui |
| Mode privé | `switch` | `<base>/privacy` | `<base>/privacy/set` | oui |
| DP `bool` exposé | `switch` | `<base>/<fonction>` | `<base>/<fonction>/set` | si exposé |
| DP `enum` exposé | `select` (valeurs de `DP_VALUES.md`) | idem | idem | si exposé |
| DP `int` exposé | `number` (min/max de `DP_VALUES.md`) | idem | idem | si exposé |
| DP `action` sans argument | `button` (`PRESS`) | — | `<base>/<fonction>/set` | si exposé |
| DP `action` avec argument | `text` | — | `<base>/<fonction>/set` | si exposé |
| DP `sensor` exposé | `sensor` | `<base>/<fonction>` | — | si exposé |

Seuls les DP **confirmés et exposés** dans `docs/firmware/DP_VALUES.md` sont annoncés (même règle que la passerelle #53). L'appareil contient aujourd'hui 32 entités (mouvement, jour/nuit, mode privé, PTZ, préréglages, berceuse, redémarrage, réglages exposés…).

Les états de `night` et `privacy` ne sont publiés qu'après un premier ordre : jusque-là HA affiche « inconnu ».

## Générer / vérifier

- `tools/motion/build.sh` régénère `tools/gateway/ha_discovery.h` depuis `dp_table.h` (`tools/gateway/gen_ha_discovery.py`) et le lie dans `md_inject.so` (`tools/gateway/ha.c`).
- Voir exactement ce que publiera une caméra, sans elle :
  `python3 tools/gateway/gen_ha_discovery.py --print --base camera188`
  (options `--motion <topic>`, `--prefix <préfixe>`). Chaque ligne `topic<TAB>charge` peut être publiée à la main avec
  `mosquitto_pub -r -t '<topic>' -m '<charge>'` pour essayer côté HA.
- Tests PC : `tools/gateway/test/run.sh` (sortie C identique à `--print`, JSON valide, identifiants uniques) et `MQTT=1 tools/gateway/test/run.sh` (publication réelle sur mosquitto, charges > 127 octets).

## Retirer une caméra de HA

Mettre `HA_DISCOVERY=0`, puis effacer les messages retained (charge vide) :
`python3 tools/gateway/gen_ha_discovery.py --print --base camera188 | cut -f1 | while read t; do mosquitto_pub -r -t "$t" -n; done`

## Limites

- Pas de disponibilité (`availability`) : la caméra n'envoie pas de testament MQTT ; une caméra éteinte garde ses dernières valeurs.
- Annonce une fois par démarrage d'`anyka_ipc` (premier abonnement réussi). Les messages étant retained, un redémarrage de HA les relit.
- Le caractère `"`, `\` ou `%` dans `MOTION_TOPIC` ou `HA_PREFIX` bloque l'annonce (journal `ha discovery skipped`).
- Non testé sur caméra : à valider par Mathilde (journal `[md_inject] … ha discovery published`, appareil visible dans HA).
