#include <stdio.h>

#if !defined(__APPLE__) || !defined(__MACH__)
#include <sys/types.h>
#endif
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <time.h> 

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
# ifdef SO_NOSIGPIPE
#  define CEPH_USE_SO_NOSIGPIPE
# else
#  error "Cannot block SIGPIPE!"
# endif
#endif

int main(int argc, char **argv)
{
	int masterSocket = socket(
		AF_INET /* IP v4 */,
		SOCK_STREAM /* TCP */,
		IPPROTO_TCP);

	int optval = 1;
	setsockopt(masterSocket,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
	struct sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(12345);
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0
	int ec = 0;
	if( (ec = bind(masterSocket, (struct sockaddr *) (&sockAddr), sizeof(sockAddr))) )
		printf("Bad bind: %d", ec);

	if( (ec = listen(masterSocket, SOMAXCONN)) )
		printf("Bad listen: %d", ec);

	int slaveSocket = 0;
	const size_t expected = 1500;
	char* buffer = malloc(sizeof(char)* expected);
	while(1)
	{
		slaveSocket = accept(masterSocket, 0, 0);
		int res = 1;
		while( 0 < res )
		{		
			res = recv(slaveSocket, buffer, expected - 1, MSG_NOSIGNAL);
			if( 0 < res) send(slaveSocket, buffer, res, MSG_NOSIGNAL);
			sleep(1);
			memset(buffer, 0, strlen(buffer));
		}
		shutdown(slaveSocket, SHUT_RDWR);
		close(slaveSocket);
	}
	
	free(buffer);
	
	return 0;
}
