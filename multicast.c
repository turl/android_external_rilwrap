#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "multicast.h"

struct RIL_Sock* RIL_create_multicast_sock(unsigned int port)
{
	int sock;
	int reuse_addr = 1;
	unsigned int ttl = MULTICAST_TTL;
	struct sockaddr_in addr;
	struct RIL_Sock *val;

	/* create a socket for sending to the multicast address */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		/* Socket creation failed :( */
		return NULL;
	}

	/* set the TTL (time to live/hop count) for the send */
	if ((setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
			(void*) &ttl, sizeof(ttl))) < 0) {
		/* TTL set up failed :( */
		return NULL;
	}

	/* set reuse port to on to allow multiple binds per host */
	if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,
			sizeof(reuse_addr))) < 0) {
		/* Flag set up failed :( */
		return NULL;
	}

	/* construct a multicast address structure */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family	= AF_INET;
	addr.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
	addr.sin_port = htons(port);

	val = (struct RIL_Sock*) malloc(sizeof(struct RIL_Sock));
	val->sock = sock;
	memcpy(&val->sockaddr, &addr, sizeof(struct sockaddr));

	return val;
}

struct RIL_Sock* RIL_create_receive_multicast_sock(unsigned int port)
{
	struct ip_mreq request;

	struct RIL_Sock *val = RIL_create_multicast_sock(port);

	/* construct an IGMP join request structure */
	request.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDRESS);
	request.imr_interface.s_addr = htonl(INADDR_ANY);

	/* send an ADD MEMBERSHIP message via setsockopt */
	if ((setsockopt(val->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(void*) &request, sizeof(request))) < 0) {
		/* membership failed :( */
		return NULL;
	}

	memcpy(&val->request, &request, sizeof(struct ip_mreq));

	return val;
}

int RIL_send_multicast_sock(struct RIL_Sock *sock, void *content, int length)
{
	/* send string to multicast address */
	if (sendto(sock->sock, content, length, 0, &sock->sockaddr,
			sizeof(sock->sockaddr)) != length) {
		return -1;
	}
	return 0;
}

void RIL_close_multicast_sock(struct RIL_Sock *sock)
{
	close(sock->sock);
	free(sock);
}

void RIL_close_receive_multicast_sock(struct RIL_Sock *sock)
{
	/* send a DROP MEMBERSHIP message via setsockopt */
	setsockopt(sock->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(void*) &sock->request, sizeof(sock->request));

	RIL_close_multicast_sock(sock);
}
