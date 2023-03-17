/**
 *   In nic promisc mode, we recv pkt and forward to application layer.
 *    Use socket.
 *
 *    author: xxxx
 *    date: 2014-06-09
 *
 */
#include <sys/socket.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_packet.h>

int up_xmit(int socket, char *buff, int n)
{
	char *ip_buff = buff;
	struct iphdr* ip = (struct iphdr*)ip_buff; 
	struct udphdr * udp = (struct udphdr*) (ip_buff + sizeof(struct iphdr));

	if (ip->protocol == IPPROTO_UDP 
			&& ntohs(udp->dest) == 53) 
	{  
		send(socket, buff + sizeof(struct udphdr), n - sizeof(struct udphdr), 
					MSG_CONFIRM | MSG_DONTROUTE | MSG_DONTWAIT); 
	}

	return 0;
}

int create_xmit_socket(const char *ip, int port)
{
	int sin_len;

	int socket_descriptor;
	struct sockaddr_in sin;

	bzero(&sin,sizeof(sin));
	sin.sin_family=AF_INET;
	inet_aton(ip, &(sin.sin_addr));
	sin.sin_port=htons(port);
	sin_len=sizeof(sin);

	socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);
		
	connect(socket_descriptor, (struct sockaddr *)&sin, sizeof(sin));
	return socket_descriptor;
}


int main(argc,argv)
	int argc;
	char *argv[];
{
	int recv_sock, send_sock;
	char buffer[2048];  
	int n ;
	struct ifreq ifr;
	
	if (argc != 3) 
	{
		printf("Usage: %s ip port\n", argv[0]);
		exit(0);
	}
	
	char *ip = argv[1];
	int port = atoi(argv[2]);
	if (port <= 0) 
	{
		printf("transmit port must > 0\n");
		exit(0);
	}
	
	if (fork() > 0) {
		// parent
		exit(0);
	} 
	
	if (fork() > 0) {
		exit(0);
	}
	
	close(0);
	close(1);
	close(2);

	send_sock = create_xmit_socket(ip, port); 
	if (send_sock < 0)
	{
		printf("create send sock failed.\n");
		return 0;
	}

	recv_sock = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_IP));
	if (recv_sock < 0)
	{
		printf("create recv sock failed.\n");
		return 0;
	}

	strcpy(ifr.ifr_name, "eth2");
	if(ioctl(recv_sock, SIOCGIFFLAGS, &ifr) < 0)
	{
		perror("siocgifflags");
		exit(0);
	}

	while (1)
	{
		if((n = recvfrom(recv_sock, buffer, 2048, 0, NULL, NULL)) > 14 + 20 + 8 + 2)
        { 
			up_xmit(send_sock, buffer + 14, n - 14);
		}
	}

	return 0;
}

