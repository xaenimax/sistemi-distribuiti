#include <netinet/ip.h>


void buildip_nf {

        struct iphdr *ip;

        ip = (struct *iphdr) malloc(sizeof(struct iphdr));
        ip->ihl = 5;
        ip->version = 4; /* ipv4 */
        ip->tos = 0; /* non utilizzato */
        ip->tot_len = sizeof(struct iphdr) +  452; 
        ip->id = htons(getuid());
        ip->ttl = 255; /* numero rmax di hop */
        ip->protocol = IPPROTO_ICMP; /* Protocollo utilizzato */
        ip->saddr = inet_addr("127.0.0.1"); /* sorgente */
        ip->daddr = inet_addr("127.0.0.1"); /* destinazione */
        ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));
} 

