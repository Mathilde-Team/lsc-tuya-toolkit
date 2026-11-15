/*
 * clockfix.so : empêche anyka_ipc de régler l'horloge système.
 * Chargée par LD_PRELOAD. anyka_ipc a 3 appelants de settimeofday (ht_set_timezone(28800) qui
 * ajoute +8 h, IPC_APP_Sync_Utc_Time, ht_set_system_time) : on ignore tous ses réglages.
 * L'OSD affiche l'horloge système brute ; keep_tz (entrypoint.sh) la maintient à
 * « heure locale lue comme UTC » en tenant compte de l'heure d'été.
 */
extern long syscall(long n, ...);

/* Nom forcé : les en-têtes glibc le renommeraient en __settimeofday64, absent d'uClibc. */
int settimeofday(const void *tv, const void *tz) __asm__("settimeofday");

int settimeofday(const void *tv, const void *tz)
{
    (void)tv;
    (void)tz;
    return 0;
}
