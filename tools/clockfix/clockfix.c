/*
 * clockfix.so : empêche anyka_ipc de décaler l'horloge de +8 h (fuseau Chine).
 * Chargée par LD_PRELOAD. ht_set_timezone(28800) relit l'heure, ajoute 28800 s et
 * appelle settimeofday(). On refuse ce réglage précis (écart de 28800 s +/- 120 s
 * par rapport à l'heure courante) et on laisse passer tous les autres.
 * Le fuseau d'affichage reste géré par /tmp/TZ (TZ_STRING dans hack.conf).
 * Limite connue : l'heure incrustée dans l'image (OSD) est l'heure UTC du système.
 */
#define SYS_gettimeofday_arm 78
#define SYS_settimeofday_arm 79
#define CN_OFFSET 28800
#define TOL 120

extern long syscall(long n, ...);

struct tv32 {
    long tv_sec;
    long tv_usec;
};

/* Nom forcé : les en-têtes glibc le renommeraient en __settimeofday64, absent d'uClibc. */
int settimeofday(const struct tv32 *tv, const void *tz) __asm__("settimeofday");

int settimeofday(const struct tv32 *tv, const void *tz)
{
    struct tv32 now;
    long d;

    if (tv && syscall(SYS_gettimeofday_arm, &now, 0) == 0) {
        d = tv->tv_sec - now.tv_sec;
        if (d > CN_OFFSET - TOL && d < CN_OFFSET + TOL)
            return 0;
    }
    return (int)syscall(SYS_settimeofday_arm, tv, tz);
}
