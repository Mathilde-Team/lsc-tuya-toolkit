# Position privée (privacy) par preset PTZ

La boîte annonce une « privacy function ». Ce toolkit l'obtient simplement : un
**preset PTZ** (tête tournée vers le mur ou vers le bas) vers lequel un bouton
envoie la caméra. Les presets sont vérifiés sur deux caméras (SetPreset,
GetPresets, GotoPreset, RemovePreset — CHANGES.md §17).

## Limites, à lire avant de s'y fier

- **Une position PTZ ne coupe ni le flux RTSP ni le micro** : quiconque peut
  lire le flux (voir le filtrage LAN, `docs/SECURITY.md` recommandation 1) et
  écouter, et remettre la caméra en position s'il a accès à ONVIF. Ce n'est
  qu'un doute visuel, pas une confidentialité réelle.
- Le preset est mémorisé côté caméra (`custom/state/ptz_presets.db`, offsets
  calculés) : la dérive du moteur s'y applique comme au reste.

## 1. Créer le preset « privé »

Placer la caméra en position privée (ONVIF `ContinuousMove` depuis Home
Assistant, ou `ptz_move`), puis :

```sh
# sur la caméra (telnet port 24)
/tmp/sd/custom/scripts/ptz_presets.sh -a add_preset -n 1 -m prive
```

ou depuis Home Assistant (Outils de développement → Actions →
`onvif.set_preset` avec le numéro choisi). Noter le numéro (1 ci-dessus).

## 2. Câbler l'option

- installeur : `camera.privacy_preset: 1` dans `install.yaml` (ou question de
  l'assistant) — écrit `PRIVACY_PRESET=1` dans `hack.conf` ;
- à la main : `PRIVACY_PRESET=1` dans `sd_card/custom/configs/hack.conf`.

Sur la caméra même : `/tmp/sd/custom/scripts/privacy.sh` envoie la tête sur le
preset (0 = désactivé, ne fait rien).

## 3. Bouton « Privé / Normal » dans Home Assistant

```yaml
script:
  camera_prive:
    action: onvif.goto_preset
    data:
      preset: 1          # numéro du preset privé
      continuous_duration: 0
  camera_normal:
    action: onvif.goto_preset
    data:
      preset: 2          # preset « normal » (créé de la même façon)
```

Boutons du tableau de bord : deux entités `button` appelant ces scripts, ou
l'action `onvif.goto_preset` directement sur une carte bouton.

Pour couper *aussi* le flux dans HA pendant le mode privé (arrêt de
l'enregistrement NVR, masquage de la carte caméra), ajouter une automatisation
sur l'appel du script `camera_prive` — cela n'empêche pas un autre client du
LAN de lire le RTSP (voir limites ci-dessus).
