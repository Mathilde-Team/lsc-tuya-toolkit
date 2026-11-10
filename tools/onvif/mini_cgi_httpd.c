/*
 * Mini serveur HTTP/CGI pour onvif_simple_server (camera a RAM limitee).
 * Usage : mini_cgi_httpd <port> <repertoire des binaires onvif>
 * Ne sert que POST /onvif/<service> avec service dans la liste blanche ;
 * une requete = un processus, Connection: close.
 */
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_HDR   16384
#define MAX_BODY  (1024 * 1024)
#define MAX_OUT   (2 * 1024 * 1024)

static const char *SERVICES[] = {"device_service", "media_service", "media2_service", "ptz_service",
                                 "events_service", "deviceio_service", "imaging_service", NULL};

static void reply(int fd, int code, const char *msg)
{
    char b[256];
    int n = snprintf(b, sizeof b, "HTTP/1.1 %d %s\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", code, msg);
    (void)!write(fd, b, n);
}

static int write_all(int fd, const char *p, size_t n)
{
    while (n) {
        ssize_t w = write(fd, p, n);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += w;
        n -= w;
    }
    return 0;
}

static const char *hdr(const char *h, const char *name)
{
    size_t l = strlen(name);
    for (const char *p = h; (p = strstr(p, "\r\n")); ) {
        p += 2;
        if (!strncasecmp(p, name, l) && p[l] == ':') {
            p += l + 1;
            while (*p == ' ') p++;
            return p;
        }
    }
    return NULL;
}

static void handle(int c, const char *dir, const char *peer)
{
    static char buf[MAX_HDR + 1];
    int len = 0, hend = -1;

    alarm(30);
    while (hend < 0 && len < MAX_HDR) {
        ssize_t r = read(c, buf + len, MAX_HDR - len);
        if (r <= 0) return;
        len += r;
        buf[len] = 0;
        char *e = strstr(buf, "\r\n\r\n");
        if (e) hend = (e - buf) + 4;
    }
    if (hend < 0) { reply(c, 431, "Header too large"); return; }

    char method[8], path[256];
    if (sscanf(buf, "%7s %255s", method, path) != 2) { reply(c, 400, "Bad Request"); return; }
    if (strcmp(method, "POST")) { reply(c, 405, "Method Not Allowed"); return; }
    if (strncmp(path, "/onvif/", 7)) { reply(c, 404, "Not Found"); return; }

    const char *svc = NULL;
    for (int i = 0; SERVICES[i]; i++)
        if (!strcmp(path + 7, SERVICES[i])) svc = SERVICES[i];
    if (!svc) { reply(c, 404, "Not Found"); return; }

    const char *cl = hdr(buf, "Content-Length");
    long clen = cl ? atol(cl) : 0;
    if (clen < 0 || clen > MAX_BODY) { reply(c, 413, "Payload Too Large"); return; }
    const char *ct = hdr(buf, "Content-Type");
    char ctype[256] = "application/soap+xml";
    if (ct) {
        size_t n = strcspn(ct, "\r\n");
        if (n >= sizeof ctype) n = sizeof ctype - 1;
        memcpy(ctype, ct, n);
        ctype[n] = 0;
    }

    char *body = malloc(clen + 1);
    if (!body) { reply(c, 500, "Internal Server Error"); return; }
    long have = len - hend;
    if (have > clen) have = clen;
    memcpy(body, buf + hend, have);
    while (have < clen) {
        ssize_t r = read(c, body + have, clen - have);
        if (r <= 0) { free(body); return; }
        have += r;
    }

    int in[2], out[2];
    if (pipe(in) || pipe(out)) { free(body); reply(c, 500, "Internal Server Error"); return; }
    pid_t pid = fork();
    if (pid == 0) {
        char e1[64], e2[300], e3[96], e4[96], path_exe[512];
        dup2(in[0], 0);
        dup2(out[1], 1);
        close(in[0]); close(in[1]); close(out[0]); close(out[1]); close(c);
        snprintf(e1, sizeof e1, "CONTENT_LENGTH=%ld", clen);
        snprintf(e2, sizeof e2, "CONTENT_TYPE=%s", ctype);
        snprintf(e3, sizeof e3, "REMOTE_ADDR=%s", peer);
        snprintf(e4, sizeof e4, "SCRIPT_NAME=/onvif/%s", svc);
        char *envp[] = {e1, e2, e3, e4, "REQUEST_METHOD=POST", "GATEWAY_INTERFACE=CGI/1.1",
                        "SERVER_PROTOCOL=HTTP/1.1", NULL};
        if (chdir(dir)) _exit(127);
        snprintf(path_exe, sizeof path_exe, "%s/onvif_simple_server", dir);
        execle(path_exe, path_exe, svc, (char *)NULL, envp);
        _exit(127);
    }
    close(in[0]);
    close(out[1]);
    if (pid > 0) (void)!write_all(in[1], body, clen);
    close(in[1]);
    free(body);

    char *o = malloc(MAX_OUT + 1);
    long on = 0;
    if (o) {
        for (;;) {
            ssize_t r = read(out[0], o + on, MAX_OUT - on);
            if (r < 0 && errno == EINTR) continue;
            if (r <= 0) break;
            on += r;
            if (on >= MAX_OUT) break;
        }
    }
    close(out[0]);
    waitpid(pid, NULL, 0);
    if (!o || on == 0) { free(o); reply(c, 500, "Internal Server Error"); return; }
    o[on] = 0;

    /* separer les en-tetes CGI du corps */
    char *sep = strstr(o, "\r\n\r\n"), *sep2 = strstr(o, "\n\n");
    long hl, bl;
    char *bstart;
    if (sep && (!sep2 || sep < sep2)) { hl = sep - o; bstart = sep + 4; }
    else if (sep2) { hl = sep2 - o; bstart = sep2 + 2; }
    else { hl = on; bstart = o + on; }
    bl = on - (bstart - o);

    char status[64] = "200 OK";
    char head[4096];
    int hn = 0;
    char *line = o;
    while (line < o + hl) {
        char *nl = memchr(line, '\n', o + hl - line);
        long ll = nl ? nl - line : o + hl - line;
        long l2 = ll;
        if (l2 && line[l2 - 1] == '\r') l2--;
        if (l2 > 7 && !strncasecmp(line, "Status:", 7)) {
            const char *s = line + 7;
            while (*s == ' ') s++;
            snprintf(status, sizeof status, "%.*s", (int)(line + l2 - s), s);
        } else if (l2 > 0 && strncasecmp(line, "Content-Length:", 15) && strncasecmp(line, "Connection:", 11) &&
                   hn + l2 + 2 < (long)sizeof head) {
            memcpy(head + hn, line, l2);
            hn += l2;
            head[hn++] = '\r';
            head[hn++] = '\n';
        }
        line = nl ? nl + 1 : o + hl;
    }
    char first[160];
    int fn = snprintf(first, sizeof first, "HTTP/1.1 %s\r\n", status);
    char tail[96];
    int tn = snprintf(tail, sizeof tail, "Content-Length: %ld\r\nConnection: close\r\n\r\n", bl);
    write_all(c, first, fn);
    write_all(c, head, hn);
    write_all(c, tail, tn);
    write_all(c, bstart, bl);
    free(o);
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <port> <repertoire>\n", argv[0]);
        return 2;
    }
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    int s = socket(AF_INET, SOCK_STREAM, 0), one = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    struct sockaddr_in a = {.sin_family = AF_INET, .sin_port = htons(atoi(argv[1])), .sin_addr.s_addr = INADDR_ANY};
    if (bind(s, (struct sockaddr *)&a, sizeof a) || listen(s, 8)) {
        perror("bind/listen");
        return 1;
    }
    for (;;) {
        struct sockaddr_in pa;
        socklen_t pl = sizeof pa;
        int c = accept(s, (struct sockaddr *)&pa, &pl);
        if (c < 0) continue;
        if (fork() == 0) {
            close(s);
            handle(c, argv[2], inet_ntoa(pa.sin_addr));
            close(c);
            _exit(0);
        }
        close(c);
    }
}
