/* Découverte ONVIF (WS-Discovery) minimale : répond aux Probe sur 239.255.255.250:3702.
 * Usage : wsd <interface> <port ONVIF> <nom> <modèle> [0 si pas de PTZ]
 * Les caméras n'ont pas wsd_simple_server : sans ce démon Home Assistant ne trouve pas la caméra tout seul. */
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define GROUP "239.255.255.250"
#define WSD_PORT 3702

static char uuid[48], mac[24], xaddr[96], name[48], model[48];
static int has_ptz = 1;
static unsigned long seq;

static int get_net(const char *ifname, int port, struct in_addr *ip)
{
    struct ifreq ifr;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    unsigned char *h;

    if (s < 0)
        return -1;
    memset(&ifr, 0, sizeof ifr);
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
        close(s);
        return -1;
    }
    *ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
    if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
        close(s);
        return -1;
    }
    h = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    snprintf(mac, sizeof mac, "%02X:%02X:%02X:%02X:%02X:%02X", h[0], h[1], h[2], h[3], h[4], h[5]);
    snprintf(uuid, sizeof uuid, "urn:uuid:6f6e7669-6600-4000-8000-%02x%02x%02x%02x%02x%02x", h[0], h[1], h[2], h[3], h[4], h[5]);
    snprintf(xaddr, sizeof xaddr, "http://%s:%d/onvif/device_service", inet_ntoa(*ip), port);
    close(s);
    return 0;
}

static int body(char *o, int max, const char *relates)
{
    return snprintf(o, max,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<e:Envelope xmlns:e=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:w=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
        "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\">"
        "<e:Header><w:MessageID>urn:uuid:6f6e7669-6600-4000-9000-%012lx</w:MessageID>"
        "<w:RelatesTo>%s</w:RelatesTo>"
        "<w:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</w:To>"
        "<w:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</w:Action>"
        "<d:AppSequence InstanceId=\"1\" MessageNumber=\"%lu\"/></e:Header>"
        "<e:Body><d:ProbeMatches><d:ProbeMatch><w:EndpointReference><w:Address>%s</w:Address></w:EndpointReference>"
        "<d:Types>dn:NetworkVideoTransmitter</d:Types>"
        "<d:Scopes>onvif://www.onvif.org/type/video_encoder %s"
        "onvif://www.onvif.org/Profile/Streaming onvif://www.onvif.org/Profile/S "
        "onvif://www.onvif.org/name/%s onvif://www.onvif.org/hardware/%s "
        "onvif://www.onvif.org/location/local onvif://www.onvif.org/MAC/%s</d:Scopes>"
        "<d:XAddrs>%s</d:XAddrs><d:MetadataVersion>1</d:MetadataVersion></d:ProbeMatch></d:ProbeMatches></e:Body></e:Envelope>",
        seq, relates, seq, has_ptz ? "onvif://www.onvif.org/type/ptz" : "", uuid, name, model, mac, xaddr);
}

int main(int argc, char **argv)
{
    struct in_addr ip;
    struct sockaddr_in a, from;
    struct ip_mreq m;
    static char buf[4096], out[4096];
    int s, one = 1, n, i;

    if (argc < 5)
        return 1;
    has_ptz = argc < 6 || argv[5][0] != '0';
    snprintf(name, sizeof name, "%s", argv[3]);
    snprintf(model, sizeof model, "%s", argv[4]);
    for (i = 0; i < 120 && get_net(argv[1], atoi(argv[2]), &ip) < 0; i++)
        sleep(2);
    if (i == 120)
        return 1;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons(WSD_PORT);
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&a, sizeof a) < 0)
        return 1;
    m.imr_multiaddr.s_addr = inet_addr(GROUP);
    m.imr_interface = ip;
    setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m, sizeof m);
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &ip, sizeof ip);
    seq = 1;
    for (;;) {
        socklen_t fl = sizeof from;

        n = recvfrom(s, buf, sizeof buf - 1, 0, (struct sockaddr *)&from, &fl);
        if (n <= 0)
            continue;
        buf[n] = 0;
        if (strstr(buf, "Probe") && !strstr(buf, "ProbeMatches") &&
            (!strstr(buf, "Types") || strstr(buf, "NetworkVideoTransmitter") || strstr(buf, "Device"))) {
            char rel[96] = "";
            char *p = strstr(buf, "MessageID>");

            if (p) {
                char *e = strchr(p + 10, '<');

                if (e && e - (p + 10) < (int)sizeof rel) {
                    memcpy(rel, p + 10, e - (p + 10));
                    rel[e - (p + 10)] = 0;
                }
            }
            seq++;
            n = body(out, sizeof out, rel);
            sendto(s, out, n, 0, (struct sockaddr *)&from, fl);
        }
    }
}
