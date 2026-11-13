# Modules (fichiers sources) de `anyka_ipc`

Généré par `tools/firmware/gen_reference.py`. Le fichier source vient des chaînes de journal (`xxx.c:ligne`) référencées par chaque fonction : 1479 fonctions rattachées directement, 997 déduites par contiguïté (objets contigus dans `.text`).

| Module | Groupe | Fonctions | Octets | Fonctions publiques `ht_*`/`IPC_APP_*` (extrait) |
|---|---|---|---|---|
| `(inconnu)` | libc / divers | 6284 | 1840769 | `ht_av_init`, `ht_av_set_audio_param`, `ht_av_deinit`, `ht_gpios_deinit`, `ht_set_gpios_value` … |
| `ssl_tls.c` | TLS | 124 | 35512 | — |
| `tuya_ipc_dp_handler.c` | SDK Tuya | 117 | 19560 | `IPC_APP_set_light_onoff`, `IPC_APP_get_light_onoff`, `IPC_APP_set_flip_onoff`, `IPC_APP_get_flip_onoff`, `IPC_APP_set_watermark_onoff` … |
| `tuya_ipc_dp_utils.c` | SDK Tuya | 89 | 11104 | `IPC_APP_upload_all_status`, `IPC_APP_report_sd_format_status`, `IPC_APP_report_sd_status_changed`, `IPC_APP_report_sd_storage`, `IPC_APP_update_battery_status` … |
| `tuya_stun_message.c` | SDK Tuya | 83 | 12060 | — |
| `tuya_rtc.c` | SDK Tuya | 79 | 36888 | — |
| `gw_intf.c` | SDK Tuya | 66 | 28188 | — |
| `CRingBuf.c` | RTSP | 62 | 47492 | — |
| `tuya_ipc_api.c` | SDK Tuya | 61 | 12444 | — |
| `tuya_ipc_stream_storage.c` | SDK Tuya | 58 | 32492 | — |
| `tuya_ipc_p2p.c` | SDK Tuya | 54 | 42780 | — |
| `iot_httpc.c` | SDK Tuya | 54 | 23032 | — |
| `core.c` | Autres bibliothèques | 49 | 13788 | — |
| `rtsp_demo.c` | RTSP | 49 | 26016 | — |
| `tuya_ipc_cloud_storage.c` | SDK Tuya | 44 | 23416 | — |
| `tuya_misc.c` | SDK Tuya | 41 | 7140 | — |
| `mqc_app.c` | SDK Tuya | 37 | 11580 | — |
| `ht_record.c` | Enregistrement | 36 | 16860 | `ht_record_repair`, `ht_record_init`, `ht_record_deinit`, `ht_record_check_and_clean`, `ht_record_set_record_type` … |
| `ht_ptz.c` | PTZ | 36 | 22968 | `ht_zoom_init`, `ht_zoom_deinit`, `ht_ptz_init`, `ht_ptz_factory_test`, `ht_ptz_deinit` … |
| `tuya_ipc_webrtc.c` | SDK Tuya | 36 | 13800 | — |
| `ht_video_codec.c` | Vidéo | 35 | 20092 | `ht_video_codec_auto_change_fps`, `ht_video_codec_init`, `ht_video_codec_start_encode`, `ht_video_codec_stop_encode`, `ht_video_codec_set_encode_param` … |
| `rtsp_msg.c` | RTSP | 35 | 16456 | — |
| `ht_audio_codec.c` | Audio | 34 | 12720 | `ht_audio_codec_init`, `ht_audio_codec_start_encode`, `ht_audio_codec_stop_encode`, `ht_audio_codec_start_decode`, `ht_audio_codec_stop_decode` … |
| `tuya_ws_db.c` | SDK Tuya | 31 | 10720 | — |
| `tuya_frame.c` | SDK Tuya | 31 | 3124 | — |
| `tuya_ice.c` | SDK Tuya | 29 | 10836 | — |
| `fs.c` | Autres bibliothèques | 28 | 10168 | — |
| `smart_frame.c` | SDK Tuya | 25 | 19016 | — |
| `tuya_iot_com_api.c` | SDK Tuya | 25 | 4524 | — |
| `tuya_ring_buffer.c` | SDK Tuya | 24 | 12120 | — |
| `ht_night_mode.c` | Jour/nuit | 23 | 10256 | `ht_night_mode_light_sensor_test`, `ht_night_mode_init`, `ht_night_mode_set`, `ht_night_mode_get`, `ht_night_mode_set_irled_mode` … |
| `app_agent.c` | SDK Tuya | 23 | 8124 | — |
| `stream.c` | Autres bibliothèques | 23 | 9992 | — |
| `udp.c` | Autres bibliothèques | 23 | 7088 | — |
| `online_log_serv.c` | SDK Tuya | 23 | 8328 | — |
| `tuya_ipc_wifi_demo.c` | SDK Tuya | 22 | 9532 | — |
| `mqtt_client.c` | SDK Tuya | 22 | 4596 | — |
| `simplekv.c` | SDK Tuya | 21 | 15492 | — |
| `httpc.c` | SDK Tuya | 20 | 14260 | — |
| `ht_factory_test.c` | Application (main, config, système) | 19 | 16088 | `ht_factory_test_read_ini`, `ht_factory_test_check`, `ht_factory_test_enter` |
| `tuya_ipc_mqt_proccess.c` | SDK Tuya | 19 | 4360 | — |
| `tuya_tcp_socket.c` | SDK Tuya | 19 | 4320 | — |
| `ht_montion_detect.c` | Détection | 17 | 4676 | `ht_montion_detect_init`, `ht_montion_detect_set_mode`, `ht_montion_detect_open`, `ht_montion_detect_close`, `ht_montion_detect_update_sensi_level` … |
| `ht_playback.c` | Enregistrement | 16 | 10600 | `ht_playback_init`, `ht_playback_deinit`, `ht_playback_query_by_month`, `ht_playback_query_by_day`, `ht_playback_start` … |
| `ht_network.c` | Réseau | 16 | 11016 | `ht_network_connect_wifi`, `ht_network_get_cable_state`, `ht_network_wifi_need_reconnect`, `ht_network_service_thread`, `ht_network_init` … |
| `cloud_operation.c` | SDK Tuya | 16 | 13824 | — |
| `tuya_nat_detector.c` | SDK Tuya | 16 | 4588 | — |
| `tuya_stun_session.c` | SDK Tuya | 16 | 3096 | — |
| `tuya_turn_socket.c` | SDK Tuya | 16 | 3364 | — |
| `sam_core_lic.c` | Autres bibliothèques | 16 | 13676 | — |
| `tuya_udp_socket.c` | SDK Tuya | 15 | 3788 | — |
| `signal.c` | Autres bibliothèques | 15 | 3328 | — |
| `ht_tuya_stream_storage.c` | Colle Tuya (ht_tuya_*) | 14 | 3328 | `ht_tuya_stream_storage_init`, `ht_tuya_stream_storage_reinit`, `ht_tuya_stream_storage_deinit`, `ht_tuya_stream_storage_tf_format` |
| `tuya_ipc_notification.c` | SDK Tuya | 14 | 9792 | — |
| `ht_tuya_event.c` | Colle Tuya (ht_tuya_*) | 13 | 6124 | `ht_system_call_cb`, `ht_tuya_event_sd_cb`, `ht_tuya_event_cry_cb`, `ht_tuya_event_md_cb`, `ht_tuya_event_start` … |
| `ht_tuya_func.c` | Colle Tuya (ht_tuya_*) | 13 | 2176 | `ht_tuya_func_set_indicator_light`, `ht_tuya_func_set_flip_mirror`, `ht_tuya_func_set_flip`, `ht_tuya_func_set_mirror`, `ht_tuya_func_set_osd` … |
| `http_inf.c` | Autres bibliothèques | 13 | 2208 | — |
| `ipc_httpc.c` | SDK Tuya | 13 | 3512 | — |
| `tuya_tls.c` | SDK Tuya | 13 | 4512 | — |
| `tuya_ipc_system_control_demo.c` | SDK Tuya | 12 | 3036 | `IPC_APP_Reset_System_CB`, `IPC_APP_Restart_Process_CB`, `IPC_APP_Upgrade_Firmware_CB`, `IPC_APP_Upgrade_Inform_cb`, `IPC_APP_Notify_LED_Sound_Status_CB` … |
| `tuya_ipc_p2p_common.c` | SDK Tuya | 12 | 3924 | — |
| `ikcp.c` | Autres bibliothèques | 12 | 3488 | — |
| `tuya_ipc_streamer.c` | SDK Tuya | 11 | 9752 | — |
| `ht_system_ctrl.c` | Application (main, config, système) | 10 | 3348 | `ht_system_ctrl_init`, `ht_system_ctrl_red_led`, `ht_system_ctrl_blue_led`, `ht_system_ctrl_state_led`, `ht_system_set_reset_cb` … |
| `tuya_ipc_ai_face_detect.c` | SDK Tuya | 10 | 7752 | — |
| `wf_nw_cfg.c` | Autres bibliothèques | 10 | 3608 | — |
| `tuya_stun_socket.c` | SDK Tuya | 10 | 2432 | — |
| `wf_basic_intf.c` | Autres bibliothèques | 10 | 764 | — |
| `sam_core.c` | Autres bibliothèques | 10 | 4276 | — |
| `ht_pir_detect.c` | Détection | 9 | 2100 | `ht_pir_detect_init`, `ht_pir_detect_open`, `ht_pir_detect_close`, `ht_pir_detect_set_sensitivity`, `ht_pir_detect_get_sensitivity` … |
| `ht_video_osd.c` | Vidéo | 9 | 5952 | `ht_video_osd_th`, `ht_video_osd_init`, `ht_video_osd_open`, `ht_video_osd_close`, `ht_video_osd_set_format` … |
| `tuya_tlv.c` | SDK Tuya | 9 | 1648 | — |
| `process.c` | Autres bibliothèques | 9 | 5408 | — |
| `tuya_iot_wifi_api.c` | SDK Tuya | 9 | 1484 | — |
| `mqc_media_app.c` | SDK Tuya | 9 | 3864 | — |
| `storage.c` | Autres bibliothèques | 8 | 2896 | — |
| `sys_timer.c` | Autres bibliothèques | 8 | 1892 | — |
| `tlink_bc.c` | Autres bibliothèques | 8 | 5668 | — |
| `main.c` | Application (main, config, système) | 7 | 2472 | `ht_system_reset`, `ht_check_id_valid_cb` |
| `preset_operation.c` | Autres bibliothèques | 7 | 3020 | — |
| `airkiss_bc.c` | Autres bibliothèques | 7 | 4916 | — |
| `direct_conect_tuya.c` | Autres bibliothèques | 7 | 2940 | — |
| `inotify.c` | Autres bibliothèques | 7 | 3384 | — |
| `log_seq.c` | Autres bibliothèques | 7 | 2524 | — |
| `tuya_ipc_h264_rtp.c` | SDK Tuya | 7 | 3164 | — |
| `utils.c` | Autres bibliothèques | 7 | 2840 | — |
| `ht_tuya_wrapper.c` | Colle Tuya (ht_tuya_*) | 6 | 1928 | `ht_tuya_av_capture_start`, `ht_tuya_uuid_check`, `ht_tuya_service_start` |
| `tuya_ipc_media_demo.c` | SDK Tuya | 6 | 3468 | `TUYA_APP_Init_Ring_Buffer`, `TUYA_APP_Put_Frame`, `TUYA_APP_Get_Frame_Backwards`, `TUYA_APP_Get_Frame` |
| `tuya_ipc_p2p_demo.c` | SDK Tuya | 6 | 4996 | `TUYA_APP_Enable_P2PTransfer` |
| `ht_gpio.c` | Application (main, config, système) | 6 | 1912 | `ht_gpio_export`, `ht_gpio_unexport`, `ht_gpio_direction`, `ht_gpio_write`, `ht_gpio_read` … |
| `ht_hsr_detect.c` | Application (main, config, système) | 6 | 3492 | `ht_hsr_ruixing_detect_send`, `ht_hsr_detect_init` |
| `tuya_ipc_encrypt.c` | SDK Tuya | 6 | 2128 | — |
| `poll.c` | Autres bibliothèques | 6 | 1248 | — |
| `common.c` | Autres bibliothèques | 6 | 1040 | — |
| `thing_config.c` | Autres bibliothèques | 6 | 2704 | — |
| `tuya_relay_session.c` | SDK Tuya | 6 | 5456 | — |
| `ht_device_id.c` | Application (main, config, système) | 5 | 4540 | `ht_device_id_init`, `ht_device_id_set`, `ht_device_id_get` |
| `tuya_ipc_ai_face_db.c` | SDK Tuya | 5 | 2300 | — |
| `uni_thread.c` | Autres bibliothèques | 5 | 1688 | — |
| `active_reqs.c` | SDK Tuya | 5 | 1720 | — |
| `timer_task.c` | Autres bibliothèques | 5 | 5936 | — |
| `tuya_h264_packetizer.c` | SDK Tuya | 5 | 1424 | — |
| `tuya_ipc_aac_rtp.c` | SDK Tuya | 5 | 1520 | — |
| `sam_msgq.c` | Autres bibliothèques | 5 | 1176 | — |
| `sam_osa_thread.c` | Autres bibliothèques | 5 | 1112 | — |
| `ht_tuya_baby_monitor.c` | Colle Tuya (ht_tuya_*) | 4 | 3084 | `ht_tuya_baby_monitor_init`, `ht_tuya_baby_monitor_deinit` |
| `ht_encrypt.c` | Application (main, config, système) | 4 | 2176 | `ht_aes_init`, `ht_aes_encrypt_data`, `ht_aes_decrypt_data`, `ht_aes_encrypt_data2file` |
| `ht_ipc_discovery.c` | Application (main, config, système) | 4 | 2124 | `ht_ipc_discovery_deinit`, `ht_ipc_discovery_init` |
| `ht_ping.c` | Application (main, config, système) | 4 | 2612 | `ht_network_ping` |
| `ht_system_msg.c` | Application (main, config, système) | 4 | 1148 | — |
| `ht_uart.c` | Application (main, config, système) | 4 | 2300 | `ht_uart_recv_thread`, `ht_uart_init`, `ht_uart_send_data` |
| `tuya_sdp.c` | SDK Tuya | 4 | 7384 | — |
| `tuya_tcp.c` | SDK Tuya | 4 | 2336 | — |
| `tuya_udp.c` | SDK Tuya | 4 | 2188 | — |
| `ez_mc.c` | Autres bibliothèques | 4 | 2020 | — |
| `tuya_ipc_moto_mqtt.c` | SDK Tuya | 4 | 1336 | — |
| `ssl_srv.c` | TLS | 4 | 15652 | — |
| `user_main.c` | Autres bibliothèques | 3 | 788 | `IPC_APP_Get_Mqtt_Status`, `IPC_APP_Init_SDK` |
| `ht_config.c` | Application (main, config, système) | 3 | 1240 | `ht_config_init` |
| `_ht_get_log.c` | Autres bibliothèques | 3 | 1200 | `ht_log_init`, `ht_log_save_start`, `ht_log_save_stop` |
| `ht_time.c` | Application (main, config, système) | 3 | 1184 | `ht_set_timezone`, `ht_get_system_time`, `ht_set_system_time` |
| `ht_pwm_ctrl.c` | Application (main, config, système) | 3 | 3764 | `ht_pwm_open`, `ht_pwm_set`, `ht_pwm_close` |
| `tuya_ipc_skill.c` | SDK Tuya | 3 | 1440 | — |
| `com_protocol.c` | Autres bibliothèques | 3 | 5216 | — |
| `ai.c` | Autres bibliothèques | 3 | 772 | — |
| `tuya_pool.c` | SDK Tuya | 3 | 576 | — |
| `ssl_cli.c` | TLS | 3 | 14076 | — |
| `sam_osa_sync.c` | Autres bibliothèques | 3 | 684 | — |
| `resolv.c` | Autres bibliothèques | 2 | 764 | `ht_network_set_dns`, `ht_network_get_dns` |
| `ht_monitor.c` | Application (main, config, système) | 2 | 748 | `ht_start_monitor` |
| `tuya_rtcp.c` | SDK Tuya | 2 | 832 | — |
| `tuya_rtp.c` | SDK Tuya | 2 | 572 | — |
| `dns.c` | Autres bibliothèques | 2 | 1020 | — |
| `uni_time.c` | Autres bibliothèques | 2 | 664 | — |
| `device.c` | Autres bibliothèques | 2 | 964 | — |
| `dynamic.c` | Autres bibliothèques | 2 | 812 | — |
| `tuyacn.c` | SDK Tuya | 2 | 796 | — |
| `tuya_ipc_g711_rtp.c` | SDK Tuya | 2 | 504 | — |
| `tuya_stun_transaction.c` | SDK Tuya | 2 | 352 | — |
| `ty_work_queue.c` | Autres bibliothèques | 2 | 488 | — |
| `uni_work_queue.c` | Autres bibliothèques | 2 | 824 | — |
| `tuya_stun_auth.c` | SDK Tuya | 2 | 796 | — |
| `sam_core_proxy.c` | Autres bibliothèques | 2 | 2900 | — |
| `ht_pwm_led_ctrl.c` | Application (main, config, système) | 1 | 332 | `ht_pwm_led_init` |
| `_ht_av_tuning.c` | Autres bibliothèques | 1 | 676 | — |
| `_ht_ap_mode.c` | Autres bibliothèques | 1 | 116 | — |
| `tuya_ipc_qrcode_demo.c` | SDK Tuya | 1 | 300 | — |
| `wpa_supplicant.c` | Autres bibliothèques | 1 | 588 | `ht_network_create_wpaconfig` |
| `ht_thread.c` | Application (main, config, système) | 1 | 760 | `ht_thread_create` |
| `windows.c` | Autres bibliothèques | 1 | 308 | `ht_sync_utc_time` |
| `ht_rtsp.c` | Application (main, config, système) | 1 | 748 | — |
| `lv_hsr_para.h` | Autres bibliothèques | 1 | 748 | — |
| `isp.c` | Autres bibliothèques | 1 | 1192 | — |
| `osal_drv.c` | Autres bibliothèques | 1 | 912 | — |
| `Lambdas.h` | Autres bibliothèques | 1 | 1004 | — |
| `wf_sniffer_intf.c` | Autres bibliothèques | 1 | 548 | — |
| `timer.c` | Autres bibliothèques | 1 | 476 | — |
| `region.c` | Autres bibliothèques | 1 | 540 | — |
| `domain.c` | Autres bibliothèques | 1 | 268 | — |
| `token.c` | Autres bibliothèques | 1 | 140 | — |
| `restriction.c` | Autres bibliothèques | 1 | 340 | — |
| `p2p.c` | Autres bibliothèques | 1 | 324 | — |
| `by.c` | Autres bibliothèques | 1 | 396 | — |
| `kv_storge.c` | Autres bibliothèques | 1 | 120 | — |
| `getaddrinfo.c` | Autres bibliothèques | 1 | 192 | — |
| `loop.c` | Autres bibliothèques | 1 | 324 | — |
| `tcp.c` | Autres bibliothèques | 1 | 548 | — |
| `mem_pool.c` | Autres bibliothèques | 1 | 108 | — |
| `tuya_iot_internal_api.c` | SDK Tuya | 1 | 184 | — |
| `tuya_ipc_packager.c` | SDK Tuya | 1 | 200 | — |
| `uni_msg_queue.c` | Autres bibliothèques | 1 | 352 | — |
| `uni_semaphore.c` | Autres bibliothèques | 1 | 244 | — |
| `ak_lan_protocol.c` | Autres bibliothèques | 1 | 504 | — |
| `libemqtt.c` | Autres bibliothèques | 1 | 532 | — |
| `async.c` | Autres bibliothèques | 1 | 744 | — |
| `stream_queue.c` | Autres bibliothèques | 1 | 248 | — |
| `audiocoding.c` | Autres bibliothèques | 1 | 32 | — |
| `filtbank.c` | Autres bibliothèques | 1 | 1024 | — |
| `mdct.c` | Autres bibliothèques | 1 | 192 | — |
| `sam_msgm.c` | Autres bibliothèques | 1 | 212 | — |
| `sam_core_cb.c` | Autres bibliothèques | 1 | 836 | — |
