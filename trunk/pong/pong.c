#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
//#include <netinet/ip_var.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
//#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

uint16_t in_cksum(uint16_t *addr, unsigned len);
void handlePing();

#define	DEFDATALEN	(64-ICMP_MINLEN)	/* default data length */
#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define	MAXPACKET	(65536 - 60 - ICMP_MINLEN)/* max packet size */

int main()
{
	//avvia il metodo che fa la fork di handle ping
	printf("pong daemon");
	handlePing();
	return 0;
}	
void handlePing()
{
	int sd,sen,maxsd=10;
	int cc, packlen, datalen = DEFDATALEN;
	struct icmp *icp;
	int recv, fromlen, hlen;
	fd_set wfds;
	struct sockaddr_in to, from;
	u_char *packet, outpack[MAXPACKET]; //packet per la ricezione, outpacket per l'invio
	int recval;
	struct timeval start;
	struct ip *ip;
//	char hnamebuf[MAXHOSTNAMELEN];
	
	
	//crea socket
	if ( (sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		perror("errore nella creazione del socket icmp");	/* probably not running as superuser */
		exit(1);
	}
	
	//prepara la memoria per la ricezione del pacchetto
	packlen = datalen + MAXIPLEN + MAXICMPLEN;
	if ( (packet = (u_char *)malloc((u_int)packlen)) == NULL)
	{
		perror("malloc error\n");
		exit(1);
	}

	
	
	//avvia la select in ciclo infinito
	FD_ZERO(&wfds);
	for(;;)
	{
		FD_SET(sd,&wfds);
		recval = select(maxsd+1, NULL, &wfds, NULL, NULL);
		if (recval == -1)
		{
			perror("errore nella select()");
			exit(1);
		}
		else
		{
			fromlen = sizeof(struct sockaddr_in);
			if ( (recv = recvfrom(sd, (char *)packet, packlen, 0,(struct sockaddr *)&from, (socklen_t*)&fromlen)) < 0)
			{
				perror("recvfrom error");
				exit(1);
			}
			// Check the IP header
			ip = (struct ip *)((char*)packet); 
			hlen = sizeof( struct ip ); 
			if (recv < (hlen + ICMP_MINLEN)) 
			{ 
//				cerr << "packet too short (" << ret  << " bytes) from " << hostname << endl;;
				exit(1);
			} 

			// Now the ICMP part 
			icp = (struct icmp *)(packet + hlen);
			if (icp->icmp_type == ICMP_ECHO)
			{
				printf("Recv: echo request\n");
				
				// preparo il pacchetto e l'indirizzo di ricezione
								
				to.sin_family = AF_INET;
				to.sin_addr=from.sin_addr;
				to.sin_port=from.sin_port;
				
				icp = (struct icmp *)outpack;
				icp->icmp_type = ICMP_ECHOREPLY;
				icp->icmp_code = 0;
				icp->icmp_cksum = 0;
				icp->icmp_seq = 12345;	/* seq and id must be reflected DA CONTROLLARE*/
				icp->icmp_id = getpid();
				gettimeofday(&start, NULL);
				
				cc = datalen + ICMP_MINLEN;
				icp->icmp_cksum = in_cksum((unsigned short *)icp,cc);
				
				sen = sendto(sd, (char *)outpack, cc, 0, (struct sockaddr*)&to, (socklen_t)sizeof(struct sockaddr_in));
				if (sen < 0 || sen != cc)
				{
					if (sen < 0)
						perror("sendto error");
//						cout << "wrote " << hostname << " " <<  cc << " chars, ret= " << i << endl;
				}
						
			}
		}
	}
}
	
uint16_t in_cksum(uint16_t *addr, unsigned len)
{
  uint16_t answer = 0;
  /*
   * Our algorithm is simple, using a 32 bit accumulator (sum), we add
   * sequential 16 bit words to it, and at the end, fold back all the
   * carry bits from the top 16 bits into the lower 16 bits.
   */
  uint32_t sum = 0;
  while (len > 1)  
  {
  	sum += *addr++;
    len -= 2;
  }

  // mop up an odd byte, if necessary
  if (len == 1) 
  {
    *(unsigned char *)&answer = *(unsigned char *)addr ;
    sum += answer;
  }

  // add back carry outs from top 16 bits to low 16 bits
  sum = (sum >> 16) + (sum & 0xffff); // add high 16 to low 16
  sum += (sum >> 16); // add carry
  answer = ~sum; // truncate to 16 bits
  return answer;
}