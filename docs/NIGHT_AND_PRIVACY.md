# Vision nocturne (#9) et mode « privacy » d'origine (#13) : constats

Constats issus du journal de la caméra et du désassemblage de `anyka_ipc` (firmware 3.10.78). Ce qui n'est pas vérifié est dit.

## Vision nocturne

- Réglages (`_ht_hw_settings.ini`) : `night_mode_def=1` (LED IR), `ircut_type=1`, GPIO 42 (A) et 41 (B), niveaux jour A=0/B=1, nuit A=1/B=0, `ps_mode=1` (décision logicielle par luminosité), seuils `ps_auto_day_to_night_lum=2000` et `ps_auto_night_to_day_lum=1000`, pas de LED blanche.
- Journal : `current lumi:N` toutes les ~3 s, `set ircut_state:1/0` à chaque bascule, `day--->night` / `night--->day` côté détection.
- **Bascule en boucle observée au crépuscule** : 59 changements d'état en une douzaine de minutes (17:25–17:38 ce jour-là), cycle d'environ 30 s. En mode nuit la luminosité lue est ~30–70 ; juste après le retour en mode jour elle est de 12266, ce qui relance la nuit. Piste : le capteur lit l'infrarouge au moment du changement de filtre IR-cut. La boucle s'est arrêtée seule ensuite (luminosité stable à 592).
- Les GPIO 41/42 sont lisibles dans `/sys/class/gpio` mais valent 1/1 en permanence : le filtre IR-cut est probablement piloté par impulsions. **Test « objectif couvert » non concluant** (mauvais filtre de log) : à refaire.
- À faire : confirmer la boucle avec un objectif couvert, mesurer la portée utile, voir si `ps_auto_*` permet une hystérésis (écart plus grand entre les seuils), exposer jour/nuit via ONVIF Imaging.

## Mode « privacy » d'origine

- Le firmware a un **mode veille** (`bool_sleep_mode`, `IPC_APP_set_sleep_mode`) : active `ht_ptz_set_cmd(9)`, met en pause le stockage et le service média **Tuya** (cloud) et arrête l'enregistrement ; la sortie de veille fait `ht_ptz_set_cmd(10)`.
- `ht_ptz_set_cmd` ne traite que les commandes 9 (veille) et 10 (réveil). La position moteur choisie par la commande 9 n'est pas déterminée.
- Sans cloud, seule la partie PTZ est utile : le preset de `docs/PRIVACY.md` joue ce rôle. La pause du service média ne coupe pas le RTSP local (non testé, à vérifier).
- Il existe aussi des **masques d'image** (`ak_vpss_mask_set_area`, `AK_ISP_set_main_chan_mask_area`) : zones noircies dans la vidéo, piste pour une vraie confidentialité. Non testé.
