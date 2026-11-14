# Fixture de test (NE DÉCRIT PAS LE FIRMWARE)

Valeurs inventées pour exercer `gateway.c` sur PC. Les adresses `0x0` ne sont pas des preuves : ne jamais recopier dans `docs/firmware/DP_VALUES.md`.

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 103 | flip_onoff | bool | ON / OFF | int | int | test | 0x0 fixture | oui |
| 104 | watermark_onoff | bool | ON / OFF | str | str | test | 0x0 fixture | oui |
| 105 | sleep_mode | enum | `0` `1` | int | int | test | 0x0 fixture | oui |
| 151 | sd_record_mode | enum | `event` `always` | str | str | test | 0x0 fixture | oui |
| 106 | alarm_sensitivity | int | `0` `2` | int | int | test | 0x0 fixture | oui |
| 111 | format_sd_card | action | — | aucun | aucun | test | 0x0 fixture | oui |
| 110 | sd_status | sensor | — | aucun | int | test | 0x0 fixture | oui |
| 188 | basic_anti_flicker | int | `0` `2` | int | int | test | 0x0 fixture | non |
| 169 | alarm_zone_draw | json | JSON | str | str | test | 0x0 fixture | oui |
| 179 | patrol_state | sensor | — | aucun | out | test | 0x0 fixture | oui |
| 119 | ptz_start_move | action | `0` `2` `4` `6` | str | aucun | test | 0x0 fixture | oui |
