# Sirène et annonces depuis Home Assistant

Principe : seul Home Assistant parle au port UDP de la caméra (`speaker.allow` = IP de HA). HA décide qui peut déclencher
(comptes, tableaux de bord, automatisations) ; le port de la caméra n'a pas d'authentification.

## 1. Fichiers à copier dans le dossier de configuration de HA

```
/config/talk/send_audio.py     (tools/talk/send_audio.py)
/config/talk/sirene.ulaw       (tools/talk/sirene.ulaw : déjà en G.711 µ-law 8 kHz, ffmpeg n'est pas nécessaire)
```

Pour d'autres sons, convertissez-les sur un PC : `ffmpeg -i son.mp3 -ac 1 -ar 8000 -f mulaw son.ulaw`.

## 2. `configuration.yaml`

```yaml
shell_command:
  sirene_cam_a: "python3 /config/talk/send_audio.py 192.0.2.50 /config/talk/sirene.ulaw --loop --max 20 --stop-file /config/talk/stop_a"
  sirene_cam_b: "python3 /config/talk/send_audio.py 192.0.2.51 /config/talk/sirene.ulaw --loop --max 20 --stop-file /config/talk/stop_b"
  sirene_stop_a: "touch /config/talk/stop_a"
  sirene_stop_b: "touch /config/talk/stop_b"
  annonce_cam_a: "python3 /config/talk/send_audio.py 192.0.2.50 /config/talk/{{ fichier }}.ulaw"
```

Remplacez 192.0.2.50 et 192.0.2.51 par les IP de vos caméras. `--max 20` coupe toute sirène au bout de 20 s, même si HA plante.

## 3. Boutons du tableau de bord

```yaml
type: horizontal-stack
cards:
  - type: button
    name: Sirène
    icon: mdi:alarm-light
    tap_action:
      action: call-service
      service: shell_command.sirene_cam_a
      confirmation:
        text: Déclencher la sirène ?
  - type: button
    name: Stop
    icon: mdi:stop
    tap_action:
      action: call-service
      service: shell_command.sirene_stop_a
```

Un clic lance la sirène (arrêt automatique), un second bouton l'interrompt. Un bouton « maintenu » n'existe pas dans
Lovelace : c'est l'arrêt par fichier et la durée maximale qui jouent ce rôle.

## 4. Limites

- Les commandes `shell_command` s'exécutent dans le conteneur de HA : il faut `python3` (présent dans l'image officielle).
- Voix en direct (push-to-talk) : non fournie ici (voir l'issue #12).
- Volume : `speaker.gain` dans `install.yaml` (`TALK_GAIN` dans `hack.conf`), appliqué par la caméra.
