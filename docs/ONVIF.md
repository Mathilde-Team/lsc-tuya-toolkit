# ONVIF sur la caméra (pile légère)

Le firmware de cette caméra (3.10.78.1269) n'embarque **ni `lighttpd`, ni `wsd_simple_server`, ni `onvif_notify_server`** : la pile ONVIF d'origine du toolkit ne démarre donc pas. Cette pile la remplace.

## Composants

| Élément | Rôle | Origine |
|---|---|---|
| `sd_card/custom/onvif/www/onvif/onvif_simple_server` | Serveur ONVIF (Device, Media, PTZ…) | [roleoroleo/onvif_simple_server](https://github.com/roleoroleo/onvif_simple_server), commit `7b3941a`, **GPL-3.0** ; seul changement : chemin de config et de journal (voir `tools/onvif/build.sh`) |
| `sd_card/custom/bin/mini_cgi_httpd` | Mini serveur HTTP/CGI (port 8080) | `tools/onvif/mini_cgi_httpd.c`, écrit pour ce dépôt |
| `sd_card/custom/configs/onvif_simple_server.conf` | Configuration | fournie sans identifiants |
| `sd_card/custom/scripts/onvif.sh` | Lancement au démarrage | modifié (voir ci-dessous) |

**Pourquoi un mini serveur ?** `lighttpd` charge `mod_cgi` en plugin dynamique (`.so`), impossible avec un binaire statique sur ce firmware. La RAM est limitée (~35 Mo au total, ~9 Mo libres) : `mini_cgi_httpd` pèse 0,6 Mo. Il n'accepte que `POST /onvif/<service>` pour sept noms de service connus (`device`, `media`, `media2`, `ptz`, `events`, `deviceio`, `imaging`), un processus par requête, `Connection: close`.

## Mise en place

1. Compiler (PC avec `arm-linux-gnueabi-gcc`, `cmake`) : `tools/onvif/build.sh`. Les binaires sont aussi fournis déjà compilés.
2. **Définir des identifiants** dans `sd_card/custom/configs/onvif_simple_server.conf` : décommenter `user=` et ajouter `password=`. Ne publiez jamais ce fichier une fois rempli. Sans identifiants, n'importe qui sur le réseau local pourrait piloter la caméra.
3. Copier `custom/` sur la SD. `onvif.sh` lance `mini_cgi_httpd 8080 <dossier onvif>` au démarrage.
4. Dans un client ONVIF, utiliser l'adresse `http://<ip-camera>:8080/onvif/device_service`.

## Profil annoncé

H.265, 1920×1080, AAC, flux `rtsp://<ip>:554/main_ch`. Les événements sont désactivés (`events=0`).

## PTZ via ONVIF

Les commandes appellent `ptz_move`, qui appelle `ptzctl` (voir `docs/CHANGES.md` §12).

- Un `ContinuousMove` = **un déplacement de taille fixe** (~100 px de pan, ~130 px de tilt à pleine vitesse), pas un mouvement continu. Les clients qui envoient « move » puis « stop » voient un saut par clic.
- Le moteur n'a pas de capteur de position : la position est suivie par calcul et dérive. `PTZ_HOME_ON_BOOT=1` recentre au démarrage.
- **Positions absolues** : l'espace absolu est normalisé de **-1 à 1** (x = pan, y = tilt ; 0,0 = centre). `GetStatus` rend la position dans la même échelle, `GotoPreset` déplace réellement le moteur. Serveur patché : `tools/onvif/ptz_absolu_normalise.patch`.
- Jeu mécanique d'environ 1 ordre à chaque changement de sens.

## Home Assistant : bugs rencontrés et corrigés

- **`Invalid header token: HTTP/1.1 500`** : `onvif_simple_server` écrit sa propre ligne `HTTP/1.1 500 ...` pour les erreurs SOAP ; `mini_cgi_httpd` la recopiait comme en-tête. Elle est maintenant lue comme statut.
- **`command failed with status -1`** : `mini_cgi_httpd` laissait `SIGCHLD` ignoré, hérité par le CGI, ce qui faisait échouer `system()`. Corrigé par `signal(SIGCHLD, SIG_DFL)`.
- **Déplacement trop faible** : HA envoie une vitesse de 0.1 ; `ptz_move` force 0.8 pour up/down/left/right (sans cela le saut est imperceptible).
- Le snapshot `http://<ip>:80/cgi-bin/snapshot.sh` échoue (port 80 fermé) ; HA bascule sur FFmpeg.
- L'heure de la caméra est fausse (pas de NTP/fuseau) ; l'authentification fonctionne malgré tout.

## Vérifications faites

- Sans identifiants, ou avec un mauvais mot de passe : `Sender not Authorized`.
- Bons identifiants : `GetCapabilities`, `GetProfiles` et `ContinuousMove` fonctionnent.
- Après redémarrage, le service revient seul ; flux 1080p H.265 à 20 fps inchangé, ~9 Mo de RAM libres.

## Retour arrière

Sur la SD : restaurer `custom/configs/onvif_simple_server.conf.orig` et `custom/scripts/onvif.sh.orig` (sauvegardes créées avant le déploiement), puis supprimer `custom/onvif/` et `custom/bin/mini_cgi_httpd`.
