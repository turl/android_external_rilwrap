#include <netinet/in.h>
#include <arpa/inet.h>

#define MULTICAST_ADDRESS	"224.0.0.1"
#define MULTICAST_TTL		1

struct RIL_Sock {
	int sock;
	struct sockaddr sockaddr;
	struct ip_mreq request;
};

struct RIL_Sock* RIL_create_multicast_sock(unsigned int port);
struct RIL_Sock* RIL_create_receive_multicast_sock(unsigned int port);
int RIL_send_multicast_sock(struct RIL_Sock *sock, void *content, int length);
void RIL_close_multicast_sock(struct RIL_Sock *sock);
void RIL_close_receive_multicast_sock(struct RIL_Sock *sock);

