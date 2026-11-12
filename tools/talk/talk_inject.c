/*
 * talk_inject.so : entrée audio LAN pour anyka_ipc (haut-parleur).
 * Chargée par LD_PRELOAD. Reçoit en UDP du G.711 µ-law 8 kHz mono brut et
 * le passe au décodeur « talk » interne (ht_audio_codec_play_talk_data).
 * Variables : TALK_PORT (défaut 5004), TALK_ALLOW (IPv4 unique autorisée, sinon toutes),
 * TALK_GAIN (volume en pour-cent, 1..100, défaut 100).
 */
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>

extern int ht_audio_codec_talk_open(void);
extern int ht_audio_codec_talk_close(void);
extern int ht_audio_codec_play_talk_data(unsigned char *buf, int len);

#define STARTUP_DELAY_S 40

static short ulaw_lin(unsigned char u)
{
    int t;
    u = ~u;
    t = (((u & 0x0F) << 3) + 0x84) << ((u & 0x70) >> 4);
    return (u & 0x80) ? (0x84 - t) : (t - 0x84);
}

static unsigned char lin_ulaw(int s)
{
    int sign = 0, e = 7, m;
    if (s < 0) { s = -s; sign = 0x80; }
    if (s > 32635) s = 32635;
    s += 0x84;
    for (m = 0x4000; !(s & m) && e > 0; m >>= 1) e--;
    return ~(sign | (e << 4) | ((s >> (e + 3)) & 0x0F));
}

static void apply_gain(unsigned char *b, int n, int pct)
{
    int i;
    if (pct >= 100) return;
    for (i = 0; i < n; i++)
        b[i] = lin_ulaw(ulaw_lin(b[i]) * pct / 100);
}
#define IDLE_CLOSE_MS   1500

static void *worker(void *arg)
{
    const char *p = getenv("TALK_PORT");
    const char *allow = getenv("TALK_ALLOW");
    const char *g = getenv("TALK_GAIN");
    int port = p ? atoi(p) : 5004;
    int gain = g ? atoi(g) : 100;
    unsigned char buf[2048];
    struct sockaddr_in sa, from;
    socklen_t fl;
    int s, open = 0, idle = 0, n;
    in_addr_t ok = allow ? inet_addr(allow) : 0;
    struct pollfd pf;

    (void)arg;
    sleep(STARTUP_DELAY_S);
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
        return 0;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&sa, sizeof sa) < 0)
        return 0;
    pf.fd = s;
    pf.events = POLLIN;

    for (;;) {
        n = -1;
        if (poll(&pf, 1, 100) > 0) {
            fl = sizeof from;
            n = recvfrom(s, buf, sizeof buf, 0, (struct sockaddr *)&from, &fl);
        }
        if (n > 0 && (!allow || from.sin_addr.s_addr == ok)) {
            if (!open) {
                ht_audio_codec_talk_open();
                open = 1;
            }
            apply_gain(buf, n, gain < 1 ? 1 : gain);
            ht_audio_codec_play_talk_data(buf, n);
            idle = 0;
        } else if (open && (idle += 100) >= IDLE_CLOSE_MS) {
            ht_audio_codec_talk_close();
            open = 0;
        }
    }
    return 0;
}

__attribute__((constructor)) static void talk_init(void)
{
    pthread_t t;
    pthread_create(&t, 0, worker, 0);
}
