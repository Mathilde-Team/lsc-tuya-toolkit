# Passerelle locale MQTT / ONVIF (epic #51)

Remplace le cloud Tuya par des topics MQTT locaux, un par DP de `docs/firmware/DP_TABLE.md`.

| Fichier | Rôle | Issue |
|---|---|---|
| `gen_gateway.py` | génère `dp_table.h` et `docs/TOPICS.md` depuis `DP_TABLE.md` + `DP_VALUES.md` | #52, #53 |
| `dp_table.h` | table générée (ne pas éditer) | #52, #53 |
| `gateway.c` / `gateway.h` | runtime générique, lié dans `tools/motion/md_inject.so` | #53 |
| `test/` | tests PC (`run.sh`, `MQTT=1 run.sh` avec mosquitto) | #53 |

## Fonctionnement (runtime #53)

- `md_inject.so` s'abonne à **un seul filtre** `<base>/+/set` (QoS 0) et lit `/tmp/md_cmd` chaque seconde (ligne `<fonction> <valeur>`, utilisée par ONVIF).
- `night` et `privacy` sont des **alias historiques** au comportement inchangé (mêmes appels, mêmes journaux, mêmes états retained).
- Toute autre `<fonction>` est cherchée dans `dp_table`. Elle n'est appelée que si le DP est **confirmé et exposé** dans `docs/firmware/DP_VALUES.md` (colonne Exposé = `oui`, adresse dans Preuve). Sinon : journal `gateway: unknown or unexposed <fonction>`, rien n'est appelé.
- Conversion de la charge utile selon le type confirmé (`bool`, `enum`, `int`, `action`, `sensor`) puis appel de `IPC_APP_set_*` avec un entier ou une chaîne (colonne Écriture). Après l'ordre, `IPC_APP_get_*` est relu et publié retained sur `<base>/<fonction>`. Valeur hors liste / hors bornes : `gateway: refused`.
- Au premier abonnement : état retained de tous les DP exposés lisibles. Capteurs (`sensor`) : republiés toutes les 5 minutes.
- Les `IPC_APP_*` sont des **symboles faibles** : seules les fonctions des DP exposés sont référencées ; un symbole absent vaut NULL et n'est jamais appelé.

**État actuel : aucun DP exposé** (les lots #54 à #58 n'ont pas encore rempli `DP_VALUES.md`). La bibliothèque se comporte donc exactement comme avant pour `night` et `privacy`.

## Exposer un DP

1. Lot de décompilation : ajouter la ligne dans `docs/firmware/DP_VALUES.md` (format en tête du fichier), `Exposé = oui`.
2. `python3 tools/gateway/gen_gateway.py` (aussi lancé par `tools/motion/build.sh`).
3. `tools/motion/build.sh` : refuse tout symbole libc non déjà importé par `md_inject.so` et toute routine de division (libgcc absente).
4. Test sur caméra par Mathilde (journal `[md_inject] … gateway: <fonction> <valeur>`).

## Tests

- `tools/gateway/test/run.sh` : table générée depuis `test/DP_VALUES.fixture.md` (valeurs **inventées**, pas une description du firmware), fonctions `IPC_APP_*` simulées : bool/enum/int/action/sensor, bornes, refus, DP non exposé, alias `night`/`privacy`, `/tmp/md_cmd`.
- `MQTT=1 tools/gateway/test/run.sh` : `md_inject.c` complet sur PC contre un mosquitto local (abonnement `+/set`, alias, `/tmp/md_cmd`, trame de plus de 127 octets sans désynchronisation).
