#include <iostream>

#include <cstdlib>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <set>
#include <algorithm>

#include <sys/select.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
# ifdef SO_NOSIGPIPE
#  define CEPH_USE_SO_NOSIGPIPE
# else
#  error "Cannot block SIGPIPE!"
# endif
#endif

int set_nonblock(int fd)
{
	int flags;
#if defined(O_NONBLOCK)
	if(-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}

int main(int argc, char **argv)
{
	int masterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if( masterSocket < 0 )
	{
		std::cerr << "Cannot create socket" << std::endl;
		exit(-1);
	}
	
	std::set<int> slaveSockets;

	struct sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(12345);
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	{
		int b_res = bind(masterSocket, (struct sockaddr *)(&sockAddr), sizeof(sockAddr));
		if (b_res < 0)
		{
			std::cerr << "Cannot bind socket" << std::endl;
			exit(-1);
		}
	}
	
	// make socket nonblockable
	set_nonblock(masterSocket); // for making accept while select

	listen(masterSocket, SOMAXCONN);

	while(true)
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(masterSocket, &set);
		for(auto it = slaveSockets.begin(); it != slaveSockets.end(); ++it )
		{
			FD_SET(*it, &set);
		}
        int max = std::max(
            masterSocket, 
            *std::max_element(
                slaveSockets.begin(),
                slaveSockets.end()
            )
        );
        select(max+1, &set, nullptr, nullptr, nullptr);

		for(auto it = slaveSockets.begin(); it != slaveSockets.end(); ++it ) // if select triggered on some of slave sockets
        {
            if( FD_ISSET(*it, &set) )
            {
                static char buffer[1024];
                int recvSize = recv(
                    *it,
                    buffer,
                    1024,
                    MSG_NOSIGNAL
                );
                if( !recvSize && (errno != EAGAIN))
                {
                    shutdown(*it, SHUT_RDWR);
                    close(*it);
                    slaveSockets.erase(it);
                }
                else if(recvSize > 0 )
                {
                    send(
                            *it,
                            buffer,
                            recvSize, 
                    MSG_NOSIGNAL
                    );
                }
            }
        }
        // if select is triggered on master socket
        if (FD_ISSET(masterSocket, &set))
        {
            int slaveSocket = accept(masterSocket, 0,0);
            if( 0 < slaveSocket)
            {
                set_nonblock(slaveSocket);
                slaveSockets.insert(slaveSocket);
            }
        }
	}
	
	return 0;
}
