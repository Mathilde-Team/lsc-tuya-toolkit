# Valeurs et signatures des DP (lots de décompilation)

Rédigé **à la main** par les lots de décompilation (#54 à #58), un tableau par lot. Contrairement aux autres fichiers de ce dossier, il n'est pas généré : c'est la **source des confirmations** lue par `tools/gateway/gen_gateway.py`.

Règle (epic #51) : chaque ligne s'appuie sur le désassemblage (adresse ou fonction citée dans « Preuve »). Une ligne sans adresse ne peut pas être exposée : le générateur refuse.

## Format d'une ligne (lu par le générateur)

`| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |`

| Colonne | Contenu attendu |
|---|---|
| DP | numéro (ex. `103`) |
| Fonction | nom du topic, comme dans `docs/TOPICS.md` (informatif) |
| Type | `bool`, `enum`, `int`, `action` ou `sensor` |
| Valeurs | `bool` : texte libre (charges `ON`/`OFF`) ; `enum` : **chaque charge utile admise entre accents graves** (ex. `` `0` `1` `2` ``) ; `int` : **deux nombres entre accents graves**, min puis max (ex. `` `0` `100` ``) ; autres : texte libre |
| Écriture | passage de la valeur à `IPC_APP_set_*` (registre r0) : `int`, `str` (pointeur de chaîne) ou `aucun` |
| Lecture | retour de `IPC_APP_get_*` (r0) : `int`, `str` (pointeur de chaîne) ou `aucun` |
| Effets de bord / persistance | clé de configuration écrite (`ht_tuya_config_set_*`), redémarrage, etc. |
| Preuve | adresse(s) / fonction(s) lues (ex. `0x74a78 IPC_APP_set_flip_onoff : ldr ... bl ht_tuya_config_set_int`) |
| Exposé | `oui` = la passerelle pilote ce DP ; `non` = documenté seulement (matériel absent, effet dangereux, non testé) |

Après modification : `python3 tools/gateway/gen_gateway.py` régénère `tools/gateway/dp_table.h` et `docs/TOPICS.md`, puis `tools/motion/build.sh`.

Conversion faite par la passerelle (`tools/gateway/gateway.c`) :

- `bool` : `ON`/`on` → 1, `OFF`/`off` → 0 ; passé en entier (`int`) ou en chaîne `"1"`/`"0"` (`str`). État publié : `ON`/`OFF`.
- `enum` : la charge utile doit figurer dans la liste ; passée telle quelle (`str`) ou convertie en entier (`int`, valeurs numériques uniquement).
- `int` : entier décimal dans [min, max] ; passé en entier ou en chaîne.
- `action` : `aucun` = appel sans argument ; sinon comme `int`/`str`. Pas d'état publié.
- `sensor` : lu au démarrage et toutes les 5 minutes, publié retained.

## Lot 1 : image et système (#54)

À compléter (DP 103, 104, 188, 198, 162).

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|

## Lot 2 : détection et alarmes (#55)

À compléter (DP 106, 134, 120, 139, 140, 167, 168, 169, 170, 159, 247).

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|

## Lot 3 : carte SD (#56)

À compléter (DP 109, 110, 111, 112, 117, 150, 151).

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|

## Lot 4 : PTZ natif (#57)

À compléter (DP 116, 119, 132, 161, 174 à 179, 190, 199).

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|

## Lot 5 : berceuse, température, lumière, PIR, porte (#58)

À compléter (DP 232 à 236, 160, 238 à 246, 101, 138, 158, 152, 248).

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
