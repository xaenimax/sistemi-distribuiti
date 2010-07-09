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

#define	DEFDATALEN	(64-ICMP_MINLEN)	/* default data length */
#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define	MAXPACKET	(65536 - 60 - ICMP_MINLEN)/* max packet size */

int ping(char *target)
{

  int s, i, cc, packlen, datalen = DEFDATALEN;
	struct hostent *hp;
	struct sockaddr_in to, from;
	//struct protoent	*proto;
	struct ip *ip;
	u_char *packet, outpack[MAXPACKET];
	char hnamebuf[MAXHOSTNAMELEN];
	char *hostname;
	struct icmp *icp;
	int ret, fromlen, hlen;
	fd_set rfds;
	struct timeval tv;
	int retval;
	struct timeval start, end;
	int /*start_t, */end_t;
	int cont = 1;

	to.sin_family = AF_INET;

	// try to convert as dotted decimal address, else if that fails assume it's a hostname


	inet_aton(target, &(to.sin_addr));
	
	//Credo che controlli la risoluzione da DNS a IP
//	if (to.sin_addr.s_addr != (u_int)-1)
//		hostname = target;
//	else 
//	{
//		hp = gethostbyname(target);
//		if (!hp)
//		{
//			cerr << "unknown host "<< target << endl;
//			return -1;
//		}
//		to.sin_family = hp->h_addrtype;
//		bcopy(hp->h_addr, (caddr_t)&to.sin_addr, hp->h_length);
//		strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
//		hostname = hnamebuf;
//	}

	packlen = datalen + MAXIPLEN + MAXICMPLEN;
	if ( (packet = (u_char *)malloc((u_int)packlen)) == NULL)
	{
		perror("malloc error\n");
		return -1;
	}

/*
	if ( (proto = getprotobyname("icmp")) == NULL)
	{
		cerr << "unknown protocol icmp" << endl;
		return -1;
	}
*/
	if ( (s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		perror("errore nella creazione del socket icmp");	/* probably not running as superuser */
		return -1;
	}

	icp = (struct icmp *)outpack;
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = 12345;	/* seq and id must be reflected */
	icp->icmp_id = getpid();


	cc = datalen + ICMP_MINLEN;
	icp->icmp_cksum = in_cksum((unsigned short *)icp,cc);

	gettimeofday(&start, NULL);

	i = sendto(s, (char *)outpack, cc, 0, (struct sockaddr*)&to, (socklen_t)sizeof(struct sockaddr_in));
	if (i < 0 || i != cc)
	{
		if (i < 0)
			perror("sendto error");
//		cout << "wrote " << hostname << " " <<  cc << " chars, ret= " << i << endl;
	}
	
	// Watch stdin (fd 0) to see when it has input.
	FD_ZERO(&rfds);
	FD_SET(s, &rfds);
	// Wait up to one seconds.
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	while(cont != 0)
	{
		retval = select(s+1, &rfds, NULL, NULL, &tv);
		if (retval == -1)
		{
			perror("errore nella select()");
			return -1;
		}
		else if (retval == 1)
		{
			fromlen = sizeof(struct sockaddr_in);
			if ( (ret = recvfrom(s, (char *)packet, packlen, 0,(struct sockaddr *)&from, (socklen_t*)&fromlen)) < 0)
			{
				perror("recvfrom error");
				return -1;
			}

			// Check the IP header
			ip = (struct ip *)((char*)packet); 
			hlen = sizeof( struct ip ); 
			if (ret < (hlen + ICMP_MINLEN)) 
			{ 
//				cerr << "packet too short (" << ret  << " bytes) from " << hostname << endl;;
				return -1; 
			} 

			// Now the ICMP part 
			icp = (struct icmp *)(packet + hlen); 
			if (icp->icmp_type == ICMP_ECHOREPLY)
			{
				printf("Recv: echo reply\n");
				if (icp->icmp_seq != 12345)
				{
					printf("received sequence # %d\n", icp->icmp_seq);
					continue;
				}
				if (icp->icmp_id != getpid())
				{
					printf("received id %d\n", icp->icmp_id);
					continue;
				}
				cont = 0;
			}
			else
			{
				printf("Recv: not an echo reply\n");
				continue;
			}
	
			gettimeofday(&end, NULL);
			end_t = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);

			if(end_t < 1)
				end_t = 1;

			printf("Elapsed time = %d usec\n", end_t);
			return end_t;
		}
		else
		{
			printf("No data within one seconds.\n");
			return 0;
		}
	}
	return 0;
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
  while (len > 1)  {
    sum += *addr++;
    len -= 2;
  }

  // mop up an odd byte, if necessary
  if (len == 1) {
    *(unsigned char *)&answer = *(unsigned char *)addr ;
    sum += answer;
  }

  // add back carry outs from top 16 bits to low 16 bits
  sum = (sum >> 16) + (sum & 0xffff); // add high 16 to low 16
  sum += (sum >> 16); // add carry
  answer = ~sum; // truncate to 16 bits
  return answer;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Usage: ping hostname\n");
		exit(1);
	}
	printf("ping returned %d", ping(argv[1]));
	return 0;
}
