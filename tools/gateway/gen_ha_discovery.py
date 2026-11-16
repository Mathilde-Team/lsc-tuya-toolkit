#!/usr/bin/env python3
"""Découverte MQTT Home Assistant (issue #59), générée depuis tools/gateway/dp_table.h.

  gen_ha_discovery.py [dp_table.h] [ha_discovery.h]
      écrit l'en-tête C publié au démarrage par md_inject.so (tools/gateway/ha.c).
  gen_ha_discovery.py --print --base camera188 [--motion camera188/motion] [--prefix homeassistant] [dp_table.h]
      affiche « topic<TAB>charge » après substitution (même résultat que la caméra), par ex. pour
      publier à la main : mosquitto_pub -r -t <topic> -m '<charge>'.

Entités : toujours motion (binary_sensor), night (select), privacy (switch), PTZ (position, préréglages, absolu, recalage, calibrage) et haut-parleur ; puis un composant par DP
exposé (bool -> switch, enum -> select, int -> number, action sans argument -> button,
action avec argument -> text, json -> text, sensor -> sensor). Les DP non exposés ne sont jamais annoncés.
Marqueurs remplacés à l'exécution : %B = base, %N = identifiant de nœud, %M = topic de mouvement.
"""
import json, re, sys

DEVICE = {"identifiers": ["%N"], "name": "Caméra LSC %N", "manufacturer": "LSC Smart Connect",
          "model": "Anyka AK3918 (anyka_ipc 3.10.78)", "sw_version": "lsc-tuya-toolkit"}

ENTRY = re.compile(r'\{(\d+), "(\w+)", DP_(\w+), (0|"\w+"), (0|"\w+"), (ABI_\w+), (ABI_\w+), (0|"[^"]*"), ([01]),')


FR = {
    "light_onoff": "Lumière", "flip_onoff": "Image retournée (vertical)", "watermark_onoff": "Filigrane (date/logo)",
    "sleep_mode": "Mode veille", "alarm_sensitivity": "Sensibilité d'alarme", "sd_storage": "Stockage carte SD",
    "sd_status": "État carte SD", "sd_format_status": "Progression formatage SD", "ptz_stop_move": "PTZ arrêt (DP)",
    "night_vision_mode": "Mode vision nocturne", "basic_anti_flicker": "Anti-scintillement",
    "object_outline_onoff": "Contour des objets détectés", "system_reboot": "Redémarrer la caméra",
    "alarm_zone_onoff": "Zone d'alarme active", "alarm_zone_draw": "Zone d'alarme (JSON)",
    "siren_onoff": "Sirène", "audible_alarm_onoff": "Alarme sonore", "flight_warn_onoff": "Avertissement lumineux",
    "flight_warn_time": "Durée avertissement lumineux (s)", "alarm_function_onoff": "Alarme activée",
    "human_filter": "Filtre détection humaine", "alarm_siren_onoff": "Sirène sur alarme",
    "sound_detect_onoff": "Détection de bruit", "sound_sensitivity": "Sensibilité bruit",
    "cry_detect_onoff": "Détection de pleurs", "patrol_tmode": "Patrouille : mode horaire",
    "patrol_time": "Patrouille : durée (s)", "preset": "Préréglage (JSON)", "patrol_state": "État de patrouille",
    "link_move": "Liaison mouvement", "link_pos": "Liaison position", "ptz_start_move": "PTZ démarrer (DP)",
    "ptz_check": "PTZ auto-test", "track_enable": "Suivi automatique", "patrol_mode": "Patrouille : type",
    "patrol_switch": "Patrouille", "blub_onoff": "Ampoule", "floodlight_lightness": "Luminosité projecteur",
    "pir_detect_switch": "Détecteur PIR", "door_control": "Contrôle de porte", "ipc_lullaby_command": "Berceuse : commande",
    "battery_percent": "Niveau de batterie", "power_mode": "Mode d'alimentation", "temp_report_c": "Température (°C)",
    "temp_report_f": "Température (°F)", "temp_unit_select": "Unité de température",
    "temp_max_onoff": "Alerte température max", "temp_min_onoff": "Alerte température min",
    "temp_max_cvalue": "Température max (°C)", "temp_min_cvalue": "Température min (°C)",
    "temp_max_fvalue": "Température max (°F)", "temp_min_fvalue": "Température min (°F)",
    "ipc_lullaby_mode": "Berceuse : mode", "ipc_lullaby_volume": "Berceuse : volume",
    "ipc_lullaby_control": "Berceuse : lecture/pause", "ipc_lullaby_list": "Berceuse : morceau",
}


# Fonctions sans matériel (aucun capteur de température, aucune porte) : pas d'entité HA, topics MQTT conservés.
HIDDEN = {"temp_report_c", "temp_report_f", "temp_unit_select", "temp_max_onoff", "temp_min_onoff",
          "temp_max_cvalue", "temp_min_cvalue", "temp_max_fvalue", "temp_min_fvalue", "door_control", "system_reboot"}


def label(name):
    return FR.get(name) or name.replace("_", " ").capitalize()


def entities(hdr):
    out = [
        ("binary_sensor", "motion", {"name": "Mouvement", "unique_id": "%N_motion", "state_topic": "%M",
                                     "payload_on": "ON", "payload_off": "OFF", "device_class": "motion"}),
        ("select", "night", {"name": "Jour/nuit", "unique_id": "%N_night", "state_topic": "%B/night",
                             "command_topic": "%B/night/set", "options": ["auto", "day", "night"],
                             "icon": "mdi:weather-night"}),
        ("switch", "privacy", {"name": "Mode privé", "unique_id": "%N_privacy", "state_topic": "%B/privacy",
                               "command_topic": "%B/privacy/set", "payload_on": "ON", "payload_off": "OFF",
                               "state_on": "ON", "state_off": "OFF", "icon": "mdi:eye-off"}),
        ("sensor", "ptz_position", {"name": "Position PTZ", "unique_id": "%N_ptz_position", "state_topic": "%B/ptz_position",
                                    "icon": "mdi:cctv"}),
        ("sensor", "ptz_presets", {"name": "Préréglages PTZ", "unique_id": "%N_ptz_presets", "state_topic": "%B/ptz_presets"}),
        ("text", "ptz_absolute", {"name": "PTZ position absolue (x,y de -1 à 1)", "unique_id": "%N_ptz_absolute",
                                  "command_topic": "%B/ptz_absolute/set", "max": 24}),
        ("number", "ptz_preset_goto", {"name": "PTZ aller au préréglage", "unique_id": "%N_ptz_preset_goto",
                                       "command_topic": "%B/ptz_preset_goto/set", "min": 1, "max": 99, "step": 1, "mode": "box"}),
        ("text", "ptz_preset_save", {"name": "PTZ enregistrer le préréglage (nom)", "unique_id": "%N_ptz_preset_save",
                                     "command_topic": "%B/ptz_preset_save/set", "max": 24}),
        ("number", "ptz_preset_delete", {"name": "PTZ supprimer le préréglage", "unique_id": "%N_ptz_preset_delete",
                                         "command_topic": "%B/ptz_preset_delete/set", "min": 1, "max": 99, "step": 1, "mode": "box"}),
        ("button", "ptz_move_up", {"name": "PTZ haut", "unique_id": "%N_ptz_move_up", "command_topic": "%B/ptz_move/set", "payload_press": "up"}),
        ("button", "ptz_move_down", {"name": "PTZ bas", "unique_id": "%N_ptz_move_down", "command_topic": "%B/ptz_move/set", "payload_press": "down"}),
        ("button", "ptz_move_left", {"name": "PTZ gauche", "unique_id": "%N_ptz_move_left", "command_topic": "%B/ptz_move/set", "payload_press": "left"}),
        ("button", "ptz_move_right", {"name": "PTZ droite", "unique_id": "%N_ptz_move_right", "command_topic": "%B/ptz_move/set", "payload_press": "right"}),
        ("button", "ptz_move_stop", {"name": "PTZ arrêt", "unique_id": "%N_ptz_move_stop", "command_topic": "%B/ptz_move/set", "payload_press": "stop"}),
        ("button", "ptz_home", {"name": "PTZ recalage (home)", "unique_id": "%N_ptz_home",
                                "command_topic": "%B/ptz_home/set", "payload_press": "ON"}),
        ("button", "ptz_calibrate", {"name": "PTZ calibrage (long)", "unique_id": "%N_ptz_calibrate",
                                     "command_topic": "%B/ptz_calibrate/set", "payload_press": "ON"}),
        ("switch", "speaker_siren", {"name": "Sirène du haut-parleur (30 s max)", "unique_id": "%N_speaker_siren",
                                     "state_topic": "%B/speaker_siren", "command_topic": "%B/speaker_siren/set",
                                     "payload_on": "ON", "payload_off": "OFF", "state_on": "ON", "state_off": "OFF",
                                     "icon": "mdi:bullhorn"}),
        ("binary_sensor", "sd_health", {"name": "Problème carte SD (lecture seule)", "unique_id": "%N_sd_health",
                                       "state_topic": "%B/sd_health", "payload_on": "ON", "payload_off": "OFF",
                                       "device_class": "problem", "entity_category": "diagnostic"}),
        ("switch", "mirror", {"name": "Image en miroir (horizontal)", "unique_id": "%N_mirror", "state_topic": "%B/mirror",
                              "command_topic": "%B/mirror/set", "payload_on": "ON", "payload_off": "OFF",
                              "state_on": "ON", "state_off": "OFF", "icon": "mdi:flip-horizontal"}),
        ("switch", "flip_v", {"name": "Image retournée (vertical seul)", "unique_id": "%N_flip_v", "state_topic": "%B/flip_v",
                              "command_topic": "%B/flip_v/set", "payload_on": "ON", "payload_off": "OFF",
                              "state_on": "ON", "state_off": "OFF", "icon": "mdi:flip-vertical"}),
        ("button", "reboot", {"name": "Redémarrer la caméra", "unique_id": "%N_reboot", "command_topic": "%B/reboot/set",
                              "payload_press": "ON", "device_class": "restart"}),
        ("select", "lullaby", {"name": "Berceuse (morceau, boucle 15 min max)", "unique_id": "%N_lullaby", "state_topic": "%B/lullaby",
                               "command_topic": "%B/lullaby/set", "options": ["stop", "1", "2", "3", "4", "5"], "icon": "mdi:music"}),
        ("text", "speaker_play", {"name": "Haut-parleur : jouer un son (nom ou stop)", "unique_id": "%N_speaker_play",
                                  "command_topic": "%B/speaker_play/set", "max": 24}),
    ]
    for l in open(hdr, encoding="utf-8"):
        m = ENTRY.search(l)
        if not m or m[9] != "1" or m[2] in HIDDEN:
            continue
        dp, name, kind, set_abi, values = int(m[1]), m[2], m[3], m[6], m[8].strip('"') if m[8] != "0" else None
        base = {"name": label(name), "unique_id": "%%N_%s" % name}
        st, cmd = "%%B/%s" % name, "%%B/%s/set" % name
        has_get = m[5] != "0" and m[7] != "ABI_NONE"
        if kind == "BOOL":
            comp, e = "switch", dict(command_topic=cmd, payload_on="ON", payload_off="OFF", state_on="ON", state_off="OFF")
        elif kind == "ENUM":
            comp, e = "select", dict(command_topic=cmd, options=values.split("|"))
        elif kind == "INT":
            lo, hi = values.split("..")
            comp, e = "number", dict(command_topic=cmd, min=int(lo), max=int(hi), step=1, mode="box")
        elif kind == "ACTION":
            if set_abi == "ABI_NONE":
                comp, e = "button", dict(command_topic=cmd, payload_press="PRESS")
            else:
                comp, e = "text", dict(command_topic=cmd)
        elif kind == "JSON":
            comp, e = "text", dict(command_topic=cmd, max=160)
        else:
            comp, e = "sensor", {}
        if (has_get or kind in ("BOOL", "ENUM", "INT")) and (comp not in ("button", "text") or kind == "JSON"):
            e["state_topic"] = st
        if comp == "sensor" and "state_topic" not in e:
            continue
        base.update(e)
        out.append((comp, name, base))
    for comp, obj, e in out:
        e["device"] = DEVICE
        txt = json.dumps(e, ensure_ascii=False, separators=(",", ":"))
        assert len(txt.encode()) <= 700, "%s : charge trop longue (%d)" % (obj, len(txt))
        assert "%" not in txt.replace("%B", "").replace("%N", "").replace("%M", ""), obj
        yield comp, obj, txt


def node_id(base):
    return re.sub(r"[^A-Za-z0-9_-]", "_", base)


def cstr(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def main(argv):
    if "--print" in argv:
        a = argv[:]
        opt = {}
        for k in ("--base", "--motion", "--prefix"):
            if k in a:
                i = a.index(k)
                opt[k] = a[i + 1]
                del a[i:i + 2]
        a.remove("--print")
        hdr = a[0] if a else "tools/gateway/dp_table.h"
        b = opt["--base"]
        mo = opt.get("--motion", b + "/motion")
        pre = opt.get("--prefix", "homeassistant")
        n = node_id(b)
        for comp, obj, txt in entities(hdr):
            print("%s/%s/%s/%s/config\t%s" % (pre, comp, n, obj,
                  txt.replace("%B", b).replace("%N", n).replace("%M", mo)))
        return
    hdr = argv[0] if argv else "tools/gateway/dp_table.h"
    out = argv[1] if len(argv) > 1 else "tools/gateway/ha_discovery.h"
    rows = list(entities(hdr))
    with open(out, "w", encoding="utf-8") as f:
        f.write("/* Généré par tools/gateway/gen_ha_discovery.py depuis dp_table.h : ne pas éditer. */\n"
                "#ifndef HA_DISCOVERY_H\n#define HA_DISCOVERY_H\n\n"
                "/* topic : <prefix>/<component>/<node>/<object>/config ; %B base, %N nœud, %M mouvement */\n"
                "struct ha_entry {\n  const char *component;\n  const char *object;\n  const char *payload;\n};\n\n"
                "static const struct ha_entry ha_table[] = {\n")
        for comp, obj, txt in rows:
            f.write("  {%s, %s,\n   %s},\n" % (cstr(comp), cstr(obj), cstr(txt)))
        f.write("};\n\n#define HA_TABLE_LEN %d\n\n#endif\n" % len(rows))
    print(len(rows), "entités Home Assistant")


if __name__ == "__main__":
    main(sys.argv[1:])
