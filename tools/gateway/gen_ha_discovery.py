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


def label(name):
    return name.replace("_", " ").capitalize()


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
        ("text", "speaker_play", {"name": "Haut-parleur : jouer un son (nom ou stop)", "unique_id": "%N_speaker_play",
                                  "command_topic": "%B/speaker_play/set", "max": 24}),
    ]
    for l in open(hdr, encoding="utf-8"):
        m = ENTRY.search(l)
        if not m or m[9] != "1":
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
        if has_get and (comp not in ("button", "text") or kind == "JSON"):
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
