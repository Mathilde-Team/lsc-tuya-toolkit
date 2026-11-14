/* Imprime les messages de découverte HA tels que publiés par la caméra (comparés à gen_ha_discovery.py --print). */
#include <stdio.h>
#include <string.h>
#include "gateway.h"

static int pub(const char *t, const char *p) { printf("%s\t%s\n", t, p); return 0; }

int main(int argc, char **argv)
{
    char o[16];

    /* refus des caractères dangereux, dépassement détecté */
    if (gw_ha_announce("homeassistant", "cam\"x", "m", pub, 0) == 0)
        return 2;
    if (gw_ha_expand("%B%B", "abcdef", "n", "m", o, 8) != -1)
        return 3;
    return gw_ha_announce(argc > 3 ? argv[3] : "homeassistant", argv[1], argv[2], pub, 0) ? 1 : 0;
}
