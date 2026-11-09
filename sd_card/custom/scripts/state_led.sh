#!/bin/sh
# Remplace /usr/sbin/state_led.sh : quel que soit l'ordre (on|off|blink), la LED reste éteinte.
led=/sys/class/leds/fill_light_led
echo none > "$led/trigger" 2>/dev/null
echo 0 > "$led/brightness" 2>/dev/null
exit 0
