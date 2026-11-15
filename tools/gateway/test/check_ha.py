#!/usr/bin/env python3
"""Vérifie les messages de découverte HA (sortie de test_ha) : JSON valide, topics cohérents, champs requis."""
import json, sys

REQ = {"switch": ("command_topic",), "select": ("command_topic", "options"), "number": ("command_topic", "min", "max"),
       "button": ("command_topic",), "text": ("command_topic",), "sensor": ("state_topic",),
       "binary_sensor": ("state_topic",)}
path, base = sys.argv[1], sys.argv[2]
node = base.replace("/", "_")
seen, n = set(), 0
for line in open(path, encoding="utf-8"):
    topic, payload = line.rstrip("\n").split("\t")
    pre, comp, nd, obj, cfg = topic.split("/")
    assert pre == "homeassistant" and nd == node and cfg == "config", topic
    d = json.loads(payload)
    for k in REQ[comp]:
        assert k in d, (topic, k)
    for k in ("state_topic", "command_topic"):
        if k in d and obj != "motion":
            assert d[k].startswith(base + "/" + obj.split("_move_")[0]), (topic, d[k])
    assert d["unique_id"] not in seen
    seen.add(d["unique_id"])
    assert len(payload.encode()) <= 900
    n += 1
assert n >= 3
print("HA OK :", n, "entités")
