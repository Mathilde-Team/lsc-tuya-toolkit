#!/usr/bin/env python3
"""Génère les tableaux de bord Home Assistant d'une caméra (YAML à coller dans l'éditeur brut).

  python3 gen_dashboards.py <nom> <base_mqtt> [dossier_sortie]
  ex. : python3 gen_dashboards.py cam1 camera1 ../../docs/dashboards

Produit :
  ha_dashboard_onvif_<nom>.yaml : vidéo + déplacements/préréglages par l'intégration ONVIF (onvif.ptz)
  ha_dashboard_mqtt_<nom>.yaml  : tout par MQTT (mqtt.publish vers <base>/<fonction>/set) : PTZ, préréglages,
                                  mode privé, jour/nuit, miroir/retournement, berceuse, redémarrage

Entité caméra ONVIF supposée : camera.<nom>_profile_0 (HA 2024.8 ou plus récent pour « perform-action »).
"""
import os
import sys

PRESETS = 6


def ind(text, n):
    return "\n".join((" " * n + l) if l else l for l in text.split("\n"))


def btn(name, icon, action, confirm=None):
    out = "- type: button\n  name: \"%s\"\n  icon: %s\n  tap_action:\n" % (name, icon)
    if confirm:
        out += "    confirmation:\n      text: \"%s\"\n" % confirm
    out += ind(action, 4) + "\n"
    return out


def blank():
    return "- type: button\n  name: \" \"\n  show_icon: false\n  tap_action:\n    action: none\n"


def onvif(cam, **data):
    lines = ["action: perform-action", "perform_action: onvif.ptz", "data:", "  entity_id: camera.%s_profile_0" % cam]
    lines += ["  %s: %s" % kv for kv in data.items()]
    return "\n".join(lines)


def mqtt(base, fn, payload):
    return "action: perform-action\nperform_action: mqtt.publish\ndata:\n  topic: %s/%s/set\n  payload: \"%s\"" % (base, fn, payload)


def grid(cols, buttons):
    return "- type: grid\n  columns: %d\n  square: false\n  cards:\n%s\n" % (cols, ind("".join(buttons).rstrip("\n"), 4))


def view(title, path, cards, header):
    body = "".join(cards).rstrip("\n")
    return "%stitle: %s\nviews:\n  - title: %s\n    path: %s\n    icon: mdi:cctv\n    type: panel\n    cards:\n      - type: vertical-stack\n        cards:\n%s\n" % (
        header, title, title, path, ind(body, 10))


def picture(cam, live=True):
    return "- type: picture-entity\n  entity: camera.%s_profile_0\n  camera_view: %s\n  show_name: false\n  show_state: false\n" % (
        cam, "live" if live else "auto")


def dash_onvif(cam):
    c = [picture(cam)]
    pad = [blank(), btn("Haut", "mdi:arrow-up-bold", onvif(cam, move_mode="ContinuousMove", tilt="UP", speed=0.8, continuous_duration=0.5)), blank(),
           btn("Gauche", "mdi:arrow-left-bold", onvif(cam, move_mode="ContinuousMove", pan="LEFT", speed=0.8, continuous_duration=0.5)),
           btn("Stop", "mdi:stop-circle-outline", onvif(cam, move_mode="Stop")),
           btn("Droite", "mdi:arrow-right-bold", onvif(cam, move_mode="ContinuousMove", pan="RIGHT", speed=0.8, continuous_duration=0.5)),
           blank(), btn("Bas", "mdi:arrow-down-bold", onvif(cam, move_mode="ContinuousMove", tilt="DOWN", speed=0.8, continuous_duration=0.5)), blank()]
    c.append(grid(3, pad))
    c.append(grid(3, [btn("Préréglage %d" % i, "mdi:numeric-%d-box-outline" % i, onvif(cam, move_mode="GotoPreset", preset="PresetToken_%d" % i))
                      for i in range(1, PRESETS + 1)]))
    c.append(picture(cam, live=False))
    h = ("# Tableau de bord ONVIF de la caméra %s (généré par tools/dashboards/gen_dashboards.py)\n"
         "# Installation : Paramètres > Tableaux de bord > Ajouter > nouveau tableau, puis crayon > ⋮ > Éditeur de configuration brute > coller.\n"
         "# Préréglages : à créer avant (MQTT ptz_preset_save ou SetPreset ONVIF) ; jeton = PresetToken_<n°>.\n"
         "# Mode privé, jour/nuit, berceuse et redémarrage ne passent pas par onvif.ptz : voir le tableau de bord MQTT.\n" % cam)
    return view("Caméra %s (ONVIF)" % cam, "%s-onvif" % cam, c, h)


def dash_mqtt(cam, base):
    c = [picture(cam)]
    dirs = [("Haut gauche", "mdi:arrow-top-left", "-0.6,0.6"), ("Haut", "mdi:arrow-up", "0,0.6"), ("Haut droite", "mdi:arrow-top-right", "0.6,0.6"),
            ("Gauche", "mdi:arrow-left", "-0.6,0"), ("Centre", "mdi:crosshairs", "0,0"), ("Droite", "mdi:arrow-right", "0.6,0"),
            ("Bas gauche", "mdi:arrow-bottom-left", "-0.6,-0.6"), ("Bas", "mdi:arrow-down", "0,-0.6"), ("Bas droite", "mdi:arrow-bottom-right", "0.6,-0.6")]
    c.append(grid(3, [btn(n, i, mqtt(base, "ptz_absolute", p)) for n, i, p in dirs]))
    c.append(grid(3, [btn("Déplacer haut", "mdi:arrow-up-bold", mqtt(base, "ptz_move", "up")),
                      btn("Arrêt", "mdi:stop-circle-outline", mqtt(base, "ptz_move", "stop")),
                      btn("Déplacer bas", "mdi:arrow-down-bold", mqtt(base, "ptz_move", "down")),
                      btn("Déplacer gauche", "mdi:arrow-left-bold", mqtt(base, "ptz_move", "left")),
                      btn("Recalage (home)", "mdi:home-import-outline", mqtt(base, "ptz_home", "ON"), "Recaler la caméra sur les butées ?"),
                      btn("Déplacer droite", "mdi:arrow-right-bold", mqtt(base, "ptz_move", "right"))]))
    c.append(grid(3, [btn("Aller au préréglage %d" % i, "mdi:numeric-%d-box-outline" % i, mqtt(base, "ptz_preset_goto", str(i)))
                      for i in range(1, PRESETS + 1)]))
    c.append(grid(3, [btn("Enregistrer ici : pos%d" % i, "mdi:content-save-outline", mqtt(base, "ptz_preset_save", "pos%d" % i))
                      for i in range(1, PRESETS + 1)]))
    c.append(grid(2, [btn("Mode privé ON", "mdi:eye-off", mqtt(base, "privacy", "ON")),
                      btn("Mode privé OFF", "mdi:eye", mqtt(base, "privacy", "OFF"))]))
    c.append(grid(3, [btn("Jour/nuit auto", "mdi:brightness-auto", mqtt(base, "night", "auto")),
                      btn("Jour", "mdi:weather-sunny", mqtt(base, "night", "day")),
                      btn("Nuit", "mdi:weather-night", mqtt(base, "night", "night"))]))
    c.append(grid(4, [btn("Miroir ON", "mdi:flip-horizontal", mqtt(base, "mirror", "ON")),
                      btn("Miroir OFF", "mdi:flip-horizontal", mqtt(base, "mirror", "OFF")),
                      btn("Retourné ON", "mdi:flip-vertical", mqtt(base, "flip_v", "ON")),
                      btn("Retourné OFF", "mdi:flip-vertical", mqtt(base, "flip_v", "OFF"))]))
    c.append(grid(3, [btn("Berceuse %d" % i, "mdi:music", mqtt(base, "lullaby", str(i))) for i in range(1, 6)]
                  + [btn("Berceuse stop", "mdi:music-off", mqtt(base, "lullaby", "stop"))]))
    c.append(grid(1, [btn("Redémarrer la caméra", "mdi:restart", mqtt(base, "reboot", "ON"), "Redémarrer la caméra ?")]))
    h = ("# Tableau de bord MQTT de la caméra %s, base %s (généré par tools/dashboards/gen_dashboards.py)\n"
         "# Chaque bouton publie sur %s/<fonction>/set (voir docs/TOPICS.md) : aucune entité à nommer.\n"
         "# Installation : Paramètres > Tableaux de bord > Ajouter > nouveau tableau, puis crayon > ⋮ > Éditeur de configuration brute > coller.\n"
         "# Les états (privé, jour/nuit, position, SD…) sont dans la page de l'appareil « Caméra LSC » (découverte MQTT).\n" % (cam, base, base))
    return view("Caméra %s (MQTT)" % cam, "%s-mqtt" % cam, c, h)


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    cam, base = sys.argv[1], sys.argv[2]
    out = sys.argv[3] if len(sys.argv) > 3 else "."
    os.makedirs(out, exist_ok=True)
    for kind, text in (("onvif", dash_onvif(cam)), ("mqtt", dash_mqtt(cam, base))):
        p = os.path.join(out, "ha_dashboard_%s_%s.yaml" % (kind, cam))
        with open(p, "w", encoding="utf-8") as f:
            f.write(text)
        print(p)


main()
