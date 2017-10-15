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
#include <poll.h>

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
# ifdef SO_NOSIGPIPE
#  define CEPH_USE_SO_NOSIGPIPE
# else
#  error "Cannot block SIGPIPE!"
# endif
#endif


#define POLL_SIZE 2048 // return less then 32 events per moment (custom unlimited value)

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

    struct pollfd set[POLL_SIZE];
    set[0].fd = masterSocket;
    set[0].events = POLLIN;

	while(true)
	{
        size_t index = 1;
        for(auto it = slaveSockets.begin(); it != slaveSockets.end(); ++it )
        {
            set[index].fd = *it;
            set[index].events = POLLIN;
            index++;
        }
        
        size_t setSize = 1 + slaveSockets.size();

        poll(set, setSize, -1);

		for(size_t i = 0; i < setSize; ++i)
        {
            if(set[i].revents & POLLIN)
            {
                if(i)
                {
                    static char buffer[1500];
                    int recvSize = recv(set[i].fd, buffer, 1500, MSG_NOSIGNAL);
                    if( !recvSize && errno != EAGAIN )
                    {
                        shutdown(set[i].fd, SHUT_RDWR);
                        close(set[i].fd);
                        slaveSockets.erase(set[i].fd);
                    }
                    else if( 0 < recvSize)
                    {
                        send(set[i].fd, buffer, recvSize, MSG_NOSIGNAL);
                    }
                }
                else
                {
                    int slaveSocket = accept(masterSocket, 0, 0);
                    if( 0 < slaveSocket)
                    {
                        set_nonblock(slaveSocket);
                        slaveSockets.insert(slaveSocket);
                    }
                }
            }
        }
	}
	
	return 0;
}
