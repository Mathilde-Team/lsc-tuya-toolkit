# Référence de désassemblage du firmware

Base commune pour tous les travaux sur la caméra (issues #28 à #50 et suivantes). **Règle : toute affirmation sur le firmware s'appuie sur ces documents ou sur un nouveau désassemblage, pas sur une supposition.**

| Document | Contenu |
|---|---|
| [ARCHITECTURE.md](ARCHITECTURE.md) | groupes de fonctions, dépendances, séquence de démarrage de `main`, threads |
| [FLOWS.md](FLOWS.md) | diagrammes d'appels : vidéo, audio, PTZ, jour/nuit, détection, enregistrement, réseau, service Tuya, privacy |
| [DP_TABLE.md](DP_TABLE.md) | les 67 commandes Tuya (numéro de DP → gestionnaire → `IPC_APP_*`) |
| [MODULES.md](MODULES.md) | un module (fichier source) par ligne |
| [../FIRMWARE_COMMANDS.md](../FIRMWARE_COMMANDS.md) | les 311 fonctions `IPC_APP_*`/`ht_*` appelables en `LD_PRELOAD` |

Tout est **généré** ; ne pas éditer à la main (sauf ce README).

## Chiffres (`anyka_ipc` 3.10.78.1269, Ghidra 12.2)

- 8 760 fonctions, 28 358 liens d'appel (26 322 appels, 1 114 adresses de fonction prises, 922 sauts).
- 2 476 fonctions rattachées à un fichier source, dont 997 **déduites** par contiguïté dans `.text` (objets contigus dans un binaire statique).
- Le binaire embarque le SDK Tuya, le SDK Anyka (`ak_*`), TLS, RTSP et l'application HT (`ht_*`).

## Résultats clés

- **Table des DP** : le service Tuya appelle des fonctions via une table de paires (DP, fonction) en `.data`. Chaque DP correspond à un couple `IPC_APP_set_*` / `get_*`. Sans cloud, la table n'est jamais parcourue ; `md_inject.so` appelle directement les fonctions.
- **Démarrage** : `main` initialise config, vidéo/audio, jour/nuit, détection, réseau, RTSP, enregistrement, **puis** le service Tuya.
- **`daemon`** : superviseur (`/dev/watchdog`, FIFO `/tmp/daemon_fifo`, relance `/usr/sbin/anyka_ipc.sh`), piste pour les redémarrages (#33).
- **`handleRecvData`** appelle `ht_night_mode_set_irled_mode` : canal de commande local possible (#48).

## Autres composants

| Composant | Rôle (déduit) | Analyse |
|---|---|---|
| `app/bin/daemon` | watchdog et relance | chaînes lues |
| `app/bin/cli`, `mtd_update` | commande, mise à jour de partitions | non analysés |
| `app/lib/liblvhsr.so` | silhouettes humaines (11 exports `lv_hsr_*`) | exports listés |
| `app/lib/libavssdkbeta.so` | authentification/détection (155 exports `avs*`) | exports listés |
| `app/modules/*.ko` | pilotes : capteurs, ISP, moteur PTZ, LED, GPIO, Wi-Fi | noms listés |
| `rootfs/etc/init.d/rcS`, `rc.local` | démarrage système | utilisés par le hack |

## Reproduire

1. Ghidra 12.2 et JDK 25 (archives en dossier utilisateur, sans root).
2. Graphe d'appels : `analyzeHeadless <projet> anyka -import anyka_ipc -processor ARM:LE:32:v5t -scriptPath tools/firmware -postScript ExportCalls.java <dossier>/calls.tsv`
3. Fonctions et chaînes : `analyzeHeadless <projet> anyka -process anyka_ipc -noanalysis -scriptPath tools/firmware -postScript ExportAll.java <dossier>/out`
4. Documents : `python3 tools/firmware/gen_reference.py --in <dossier> --bin anyka_ipc --out docs/firmware`

## Limites

- FLOWS.md contient des **extraits** (profondeur et largeur limitées), pas le graphe complet.
- Les appels par pointeur ne sont suivis que via la table des DP.
- Les `FUN_xxxxxxxx` n'ont pas de symbole : rôle déduit de leurs appels, à confirmer par décompilation.
- Les modules « déduits » peuvent être faux en bordure d'un objet.
- Rien ici n'est vérifié par exécution sur caméra.
