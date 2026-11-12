/*
 * talk_inject.so : entrée audio LAN pour anyka_ipc (haut-parleur).
 * Chargée par LD_PRELOAD. Reçoit en UDP du G.711 µ-law 8 kHz mono brut et
 * le passe au décodeur « talk » interne (ht_audio_codec_play_talk_data).
 * Variables : TALK_PORT (défaut 5004), TALK_ALLOW (IPv4 unique autorisée, sinon toutes).
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
#define IDLE_CLOSE_MS   1500

static void *worker(void *arg)
{
    const char *p = getenv("TALK_PORT");
    const char *allow = getenv("TALK_ALLOW");
    int port = p ? atoi(p) : 5004;
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
