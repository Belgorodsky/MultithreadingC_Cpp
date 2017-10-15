#include <stdio.h>

#if !defined(__APPLE__) || !defined(__MACH__)
#include <sys/types.h>
#else
#include <unistd.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(12345);
	sockAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	int ec = 0;
	if( (ec = connect(s, (struct sockaddr *)(&sockAddr), sizeof(sockAddr))) )
		printf("Connect error: %d", ec);

		char buffer[] = "PING";
#if defined(__APPLE__) && defined(__MACH__)
		send(s, buffer, 4, SO_NOSIGPIPE);
		recv(s, buffer, 4, SO_NOSIGPIPE);
#else
		send(s, buffer, 4, MSG_NOSIGNAL);
		recv(s, buffer, 4, MSG_NOSIGNAL);
#endif
#if 0
	shutdown(s, SHUT_RDWR);
	close(s);
#endif
	printf("%s\n", buffer);

	return 0;
}
