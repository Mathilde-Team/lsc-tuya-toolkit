# Flux (diagrammes d'appels)

Généré par `tools/firmware/gen_reference.py`. Chaque diagramme part de fonctions connues et suit les **appels directs** (profondeur et nombre d'enfants limités ; les fonctions utilitaires sont masquées). Ce sont des **extraits**, pas le graphe complet (28 000 liens, voir `ghidra/calls.tsv` à régénérer). Les appels par pointeur de fonction n'y sont pas suivis, sauf via la table des DP (`DP_TABLE.md`).

## Chemin d'une commande cloud (DP) et chemin local

```mermaid
flowchart LR
  cloud[Cloud Tuya] --> sdk[SDK Tuya : tuya_ipc_dp_handler]
  sdk --> table[Table des DP dans .data]
  table --> h[Gestionnaire FUN_xxx]
  h --> app[IPC_APP_set_*]
  app --> ht[ht_* : matériel et configuration]
  local[md_inject.so : MQTT, fichier /tmp/md_cmd] -->|appel direct| app
  local -->|appel direct| ht
```

## Vidéo : capture, encodage, RTSP

```mermaid
flowchart TD
  ht_video_codec_start_encode["ht_video_codec_start_encode"]
  ak_venc_open["ak_venc_open"]
  ak_venc_set_stream_buff["ak_venc_set_stream_buff"]
  ak_thread_create["ak_thread_create"]
  ht_rtsp_start["ht_rtsp_start"]
  FUN_000d1930["FUN_000d1930"]
  ak_thread_rwlock_wrlock["ak_thread_rwlock_wrlock"]
  ak_print["ak_print"]
  ak_thread_rwlock_unlock["ak_thread_rwlock_unlock"]
  ak_open["ak_open"]
  ak_thread_rwlock_rdlock["ak_thread_rwlock_rdlock"]
  ak_debug_info["ak_debug_info"]
  FUN_000d21f0["FUN_000d21f0"]
  ht_video_codec_start_encode --> ak_venc_open
  ht_video_codec_start_encode --> ak_venc_set_stream_buff
  ht_video_codec_start_encode --> ak_thread_create
  ht_rtsp_start --> ak_thread_create
  ak_venc_open --> FUN_000d1930
  ak_venc_open --> ak_thread_rwlock_wrlock
  ak_venc_open --> ak_print
  ak_venc_open --> ak_thread_rwlock_unlock
  ak_venc_open --> ak_open
  ak_venc_set_stream_buff --> ak_thread_rwlock_rdlock
  ak_venc_set_stream_buff --> ak_thread_rwlock_unlock
  ak_venc_set_stream_buff --> ak_debug_info
  ak_venc_set_stream_buff --> FUN_000d21f0
  ak_venc_set_stream_buff --> ak_print
  ak_thread_create --> ak_print
  FUN_000d1930 --> ak_print
  FUN_000d21f0 --> ak_print
```

## Audio : capture et lecture

```mermaid
flowchart TD
  ht_audio_codec_start_encode["ht_audio_codec_start_encode"]
  ak_aenc_open["ak_aenc_open"]
  ak_aenc_set_attr["ak_aenc_set_attr"]
  ak_thread_create["ak_thread_create"]
  ak_aenc_close["ak_aenc_close"]
  ht_audio_codec_start_decode["ht_audio_codec_start_decode"]
  ak_adec_open["ak_adec_open"]
  ak_adec_set_output_pcm_len["ak_adec_set_output_pcm_len"]
  ak_adec_close["ak_adec_close"]
  ak_thread_mutex_destroy["ak_thread_mutex_destroy"]
  ak_print["ak_print"]
  ak_thread_rwlock_wrlock["ak_thread_rwlock_wrlock"]
  ak_thread_rwlock_unlock["ak_thread_rwlock_unlock"]
  ak_get_os_timestamp["ak_get_os_timestamp"]
  ak_mem_alloc_["ak_mem_alloc_"]
  ak_thread_rwlock_rdlock["ak_thread_rwlock_rdlock"]
  FUN_000cc7a0["FUN_000cc7a0"]
  FUN_000cc7e0["FUN_000cc7e0"]
  FUN_000cc420["FUN_000cc420"]
  ak_mem_free["ak_mem_free"]
  FUN_000cee3c["FUN_000cee3c"]
  FUN_000cc348["FUN_000cc348"]
  ht_audio_codec_start_encode --> ak_aenc_open
  ht_audio_codec_start_encode --> ak_aenc_set_attr
  ht_audio_codec_start_encode --> ak_thread_create
  ht_audio_codec_start_encode --> ak_aenc_close
  ht_audio_codec_start_decode --> ak_adec_open
  ht_audio_codec_start_decode --> ak_adec_set_output_pcm_len
  ht_audio_codec_start_decode --> ak_thread_create
  ht_audio_codec_start_decode --> ak_adec_close
  ht_audio_codec_start_decode --> ak_thread_mutex_destroy
  ak_aenc_open --> ak_print
  ak_aenc_open --> ak_thread_rwlock_wrlock
  ak_aenc_open --> ak_thread_rwlock_unlock
  ak_aenc_open --> ak_get_os_timestamp
  ak_aenc_open --> ak_mem_alloc_
  ak_aenc_set_attr --> ak_thread_rwlock_rdlock
  ak_aenc_set_attr --> ak_thread_rwlock_unlock
  ak_aenc_set_attr --> ak_print
  ak_aenc_set_attr --> FUN_000cc7a0
  ak_thread_create --> ak_print
  ak_aenc_close --> ak_print
  ak_aenc_close --> ak_thread_rwlock_wrlock
  ak_aenc_close --> FUN_000cc7e0
  ak_aenc_close --> FUN_000cc420
  ak_aenc_close --> ak_mem_free
  ak_adec_open --> ak_print
  ak_adec_open --> ak_thread_rwlock_rdlock
  ak_adec_open --> ak_thread_rwlock_unlock
  ak_adec_open --> ak_get_os_timestamp
  ak_adec_open --> ak_mem_alloc_
  ak_adec_set_output_pcm_len --> ak_thread_rwlock_rdlock
  ak_adec_set_output_pcm_len --> ak_thread_rwlock_unlock
  ak_adec_set_output_pcm_len --> ak_thread_rwlock_wrlock
  ak_adec_set_output_pcm_len --> ak_mem_free
  ak_adec_set_output_pcm_len --> ak_get_os_timestamp
  ak_adec_close --> ak_thread_rwlock_wrlock
  ak_adec_close --> ak_print
  ak_adec_close --> FUN_000cee3c
  ak_adec_close --> ak_thread_mutex_destroy
  ak_adec_close --> ak_mem_free
  FUN_000cc7a0 --> ak_print
  FUN_000cc7e0 --> FUN_000cc348
  FUN_000cc7e0 --> ak_thread_mutex_destroy
  FUN_000cc7e0 --> ak_print
  FUN_000cee3c --> ak_mem_free
  FUN_000cee3c --> ak_print
```

## PTZ

```mermaid
flowchart TD
  ht_ptz_set_cmd["ht_ptz_set_cmd"]
  FUN_000ac618["FUN_000ac618"]
  FUN_000ace24["FUN_000ace24"]
  ht_ptz_goto_preset["ht_ptz_goto_preset"]
  ht_montion_detect_pause["ht_montion_detect_pause"]
  FUN_000ac554["FUN_000ac554"]
  FUN_000aecd0["FUN_000aecd0"]
  FUN_000acd64["FUN_000acd64"]
  FUN_000ac678["FUN_000ac678"]
  FUN_000ac9b8["FUN_000ac9b8"]
  ak_mt_update_camera_angle["ak_mt_update_camera_angle"]
  FUN_000ac3c8["FUN_000ac3c8"]
  FUN_000ac494["FUN_000ac494"]
  ak_print["ak_print"]
  ht_ptz_set_cmd --> FUN_000ac618
  ht_ptz_set_cmd --> FUN_000ace24
  ht_ptz_goto_preset --> ht_montion_detect_pause
  ht_ptz_goto_preset --> FUN_000ac554
  ht_ptz_goto_preset --> FUN_000aecd0
  FUN_000ace24 --> FUN_000acd64
  FUN_000aecd0 --> FUN_000ac678
  FUN_000aecd0 --> FUN_000ace24
  FUN_000aecd0 --> FUN_000ac9b8
  FUN_000aecd0 --> ak_mt_update_camera_angle
  FUN_000ac678 --> FUN_000ac3c8
  FUN_000ac9b8 --> FUN_000ac494
  ak_mt_update_camera_angle --> ak_print
```

## Jour/nuit

```mermaid
flowchart TD
  ht_night_mode_init["ht_night_mode_init"]
  FUN_000a9ffc["FUN_000a9ffc"]
  FUN_000aa774["FUN_000aa774"]
  FUN_000a8d38["FUN_000a8d38"]
  FUN_000a968c["FUN_000a968c"]
  ht_night_mode_set_fillled_state["ht_night_mode_set_fillled_state"]
  ak_thread_create["ak_thread_create"]
  ht_night_mode_set_irled_mode["ht_night_mode_set_irled_mode"]
  ht_montion_detect_pause["ht_montion_detect_pause"]
  ht_config_get_int["ht_config_get_int"]
  FUN_000a9334["FUN_000a9334"]
  FUN_000a90a4["FUN_000a90a4"]
  ht_video_codec_switch_mode["ht_video_codec_switch_mode"]
  ht_montion_detect_set_mode["ht_montion_detect_set_mode"]
  FUN_000a93f4["FUN_000a93f4"]
  FUN_000a9844["FUN_000a9844"]
  ak_print["ak_print"]
  FUN_000a8ff4["FUN_000a8ff4"]
  ak_vi_switch_mode["ak_vi_switch_mode"]
  ht_video_codec_set_human_detect_day_night["ht_video_codec_set_human_detect_day_night"]
  ht_montion_detect_set_sensitivity["ht_montion_detect_set_sensitivity"]
  FUN_000a952c["FUN_000a952c"]
  ht_night_mode_init --> FUN_000a9ffc
  ht_night_mode_init --> FUN_000aa774
  ht_night_mode_init --> FUN_000a8d38
  ht_night_mode_init --> FUN_000a968c
  ht_night_mode_init --> ht_night_mode_set_fillled_state
  ht_night_mode_init --> ak_thread_create
  ht_night_mode_set_irled_mode --> ht_montion_detect_pause
  ht_night_mode_set_irled_mode --> FUN_000a968c
  FUN_000a9ffc --> ht_config_get_int
  FUN_000aa774 --> FUN_000a8d38
  FUN_000aa774 --> FUN_000a9334
  FUN_000a968c --> FUN_000a90a4
  FUN_000a968c --> ht_video_codec_switch_mode
  FUN_000a968c --> ht_montion_detect_set_mode
  FUN_000a968c --> FUN_000a93f4
  ht_night_mode_set_fillled_state --> FUN_000a9844
  ht_night_mode_set_fillled_state --> ht_config_get_int
  ak_thread_create --> ak_print
  FUN_000a9334 --> FUN_000a90a4
  FUN_000a90a4 --> FUN_000a8ff4
  ht_video_codec_switch_mode --> ak_vi_switch_mode
  ht_montion_detect_set_mode --> ht_video_codec_set_human_detect_day_night
  ht_montion_detect_set_mode --> ht_montion_detect_set_sensitivity
  FUN_000a9844 --> FUN_000a952c
```

## Détection de mouvement

```mermaid
flowchart TD
  ht_montion_detect_init["ht_montion_detect_init"]
  ak_md_init["ak_md_init"]
  ak_md_set_sensitivity["ak_md_set_sensitivity"]
  ht_night_mode_get_state["ht_night_mode_get_state"]
  ak_md_set_area["ak_md_set_area"]
  ak_md_enable["ak_md_enable"]
  ak_thread_create["ak_thread_create"]
  ht_montion_detect_open["ht_montion_detect_open"]
  ak_print["ak_print"]
  ak_mrd_init["ak_mrd_init"]
  ak_thread_sem_init["ak_thread_sem_init"]
  ak_mrd_set_mode["ak_mrd_set_mode"]
  ak_vi_get_sensor_resolution["ak_vi_get_sensor_resolution"]
  ak_mrd_set_filters["ak_mrd_set_filters"]
  ak_mrd_set_motion_region_size["ak_mrd_set_motion_region_size"]
  ak_debug_info["ak_debug_info"]
  ht_montion_detect_init --> ak_md_init
  ht_montion_detect_init --> ak_md_set_sensitivity
  ht_montion_detect_init --> ht_night_mode_get_state
  ht_montion_detect_init --> ak_md_set_area
  ht_montion_detect_init --> ak_md_enable
  ht_montion_detect_init --> ak_thread_create
  ht_montion_detect_open --> ak_md_enable
  ak_md_init --> ak_print
  ak_md_init --> ak_mrd_init
  ak_md_init --> ak_thread_sem_init
  ak_md_init --> ak_mrd_set_mode
  ak_md_init --> ak_vi_get_sensor_resolution
  ak_md_init --> ak_mrd_set_filters
  ak_md_set_sensitivity --> ak_print
  ak_md_set_sensitivity --> ak_mrd_set_motion_region_size
  ak_md_set_sensitivity --> ak_debug_info
  ak_md_set_area --> ak_print
  ak_md_enable --> ak_print
  ak_thread_create --> ak_print
  ak_vi_get_sensor_resolution --> ak_print
```

## Enregistrement SD

```mermaid
flowchart TD
  ht_record_init["ht_record_init"]
  ak_thread_create["ak_thread_create"]
  ak_print["ak_print"]
  ht_record_init --> ak_thread_create
  ak_thread_create --> ak_print
```

## Réseau et Wi-Fi

```mermaid
flowchart TD
  ht_network_init["ht_network_init"]
  ht_thread_create["ht_thread_create"]
  ht_network_connect_wifi["ht_network_connect_wifi"]
  ht_network_get_cable_state["ht_network_get_cable_state"]
  FUN_00087fa4["FUN_00087fa4"]
  FUN_0008803c["FUN_0008803c"]
  ht_network_init --> ht_thread_create
  ht_network_connect_wifi --> ht_network_get_cable_state
  ht_network_get_cable_state --> FUN_00087fa4
  ht_network_get_cable_state --> FUN_0008803c
```

## Service Tuya (cloud)

```mermaid
flowchart TD
  ht_tuya_service_start["ht_tuya_service_start"]
  FUN_000709b4["FUN_000709b4"]
  ht_tuya_stream_storage_init["ht_tuya_stream_storage_init"]
  IPC_APP_Get_Mqtt_Status["IPC_APP_Get_Mqtt_Status"]
  IPC_APP_ptz_preset_sync["IPC_APP_ptz_preset_sync"]
  IPC_APP_Sync_Utc_Time["IPC_APP_Sync_Utc_Time"]
  TUYA_APP_Enable_P2PTransfer["TUYA_APP_Enable_P2PTransfer"]
  IPC_APP_upload_all_status["IPC_APP_upload_all_status"]
  IPC_APP_Init_SDK["IPC_APP_Init_SDK"]
  ht_tuya_av_capture_start["ht_tuya_av_capture_start"]
  IPC_APP_get_light_onoff["IPC_APP_get_light_onoff"]
  IPC_APP_set_light_onoff["IPC_APP_set_light_onoff"]
  IPC_APP_get_flip_onoff["IPC_APP_get_flip_onoff"]
  IPC_APP_set_flip_onoff["IPC_APP_set_flip_onoff"]
  IPC_APP_get_basic_anti_flicker["IPC_APP_get_basic_anti_flicker"]
  ak_thread_create["ak_thread_create"]
  tuya_ipc_preset_get["tuya_ipc_preset_get"]
  tuya_ipc_preset_del["tuya_ipc_preset_del"]
  tuya_ipc_get_service_time["tuya_ipc_get_service_time"]
  tuya_ipc_check_in_dls["tuya_ipc_check_in_dls"]
  tuya_ipc_tranfser_init["tuya_ipc_tranfser_init"]
  IPC_APP_get_sleep_mode["IPC_APP_get_sleep_mode"]
  FUN_0007a374["FUN_0007a374"]
  IPC_APP_get_watermark_onoff["IPC_APP_get_watermark_onoff"]
  IPC_APP_get_night_mode["IPC_APP_get_night_mode"]
  FUN_0007a3b8["FUN_0007a3b8"]
  ht_tuya_service_start --> FUN_000709b4
  ht_tuya_service_start --> ht_tuya_stream_storage_init
  ht_tuya_service_start --> IPC_APP_Get_Mqtt_Status
  ht_tuya_service_start --> IPC_APP_ptz_preset_sync
  ht_tuya_service_start --> IPC_APP_Sync_Utc_Time
  ht_tuya_service_start --> TUYA_APP_Enable_P2PTransfer
  ht_tuya_service_start --> IPC_APP_upload_all_status
  FUN_000709b4 --> IPC_APP_Init_SDK
  FUN_000709b4 --> ht_tuya_av_capture_start
  FUN_000709b4 --> IPC_APP_get_light_onoff
  FUN_000709b4 --> IPC_APP_set_light_onoff
  FUN_000709b4 --> IPC_APP_get_flip_onoff
  FUN_000709b4 --> IPC_APP_set_flip_onoff
  FUN_000709b4 --> IPC_APP_get_basic_anti_flicker
  ht_tuya_stream_storage_init --> ak_thread_create
  IPC_APP_ptz_preset_sync --> tuya_ipc_preset_get
  IPC_APP_ptz_preset_sync --> tuya_ipc_preset_del
  IPC_APP_Sync_Utc_Time --> tuya_ipc_get_service_time
  IPC_APP_Sync_Utc_Time --> tuya_ipc_check_in_dls
  TUYA_APP_Enable_P2PTransfer --> tuya_ipc_tranfser_init
  IPC_APP_upload_all_status --> IPC_APP_get_sleep_mode
  IPC_APP_upload_all_status --> FUN_0007a374
  IPC_APP_upload_all_status --> IPC_APP_get_light_onoff
  IPC_APP_upload_all_status --> IPC_APP_get_flip_onoff
  IPC_APP_upload_all_status --> IPC_APP_get_watermark_onoff
  IPC_APP_upload_all_status --> IPC_APP_get_night_mode
  IPC_APP_upload_all_status --> FUN_0007a3b8
```

## Privacy / veille

```mermaid
flowchart TD
  IPC_APP_set_sleep_mode["IPC_APP_set_sleep_mode"]
  ht_tuya_config_set_boolean["ht_tuya_config_set_boolean"]
  tuya_ipc_cloud_storage_pause["tuya_ipc_cloud_storage_pause"]
  ht_ptz_set_cmd["ht_ptz_set_cmd"]
  tuya_ipc_media_service_pause["tuya_ipc_media_service_pause"]
  tuya_ipc_media_service_resume["tuya_ipc_media_service_resume"]
  tuya_ipc_cloud_storage_resume["tuya_ipc_cloud_storage_resume"]
  ht_tuya_config_set_integer["ht_tuya_config_set_integer"]
  FUN_000ac618["FUN_000ac618"]
  FUN_000ace24["FUN_000ace24"]
  ht_config_set_int["ht_config_set_int"]
  FUN_000acd64["FUN_000acd64"]
  IPC_APP_set_sleep_mode --> ht_tuya_config_set_boolean
  IPC_APP_set_sleep_mode --> tuya_ipc_cloud_storage_pause
  IPC_APP_set_sleep_mode --> ht_ptz_set_cmd
  IPC_APP_set_sleep_mode --> tuya_ipc_media_service_pause
  IPC_APP_set_sleep_mode --> tuya_ipc_media_service_resume
  IPC_APP_set_sleep_mode --> tuya_ipc_cloud_storage_resume
  ht_tuya_config_set_boolean --> ht_tuya_config_set_integer
  ht_ptz_set_cmd --> FUN_000ac618
  ht_ptz_set_cmd --> FUN_000ace24
  ht_tuya_config_set_integer --> ht_config_set_int
  FUN_000ace24 --> FUN_000acd64
```

