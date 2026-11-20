#!/bin/sh
# Remplace /usr/sbin/state_led.sh : quel que soit l'ordre (on|off|blink), les LED restent éteintes.
# Selon le modèle la LED s'appelle fill_light_led ou state_led.
for led in /sys/class/leds/fill_light_led /sys/class/leds/state_led; do
  [ -d "$led" ] || continue
  echo none > "$led/trigger" 2>/dev/null
  echo 0 > "$led/brightness" 2>/dev/null
done
exit 0
