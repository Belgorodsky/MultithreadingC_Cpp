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
#include <sys/epoll.h>

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
# ifdef SO_NOSIGPIPE
#  define CEPH_USE_SO_NOSIGPIPE
# else
#  error "Cannot block SIGPIPE!"
# endif
#endif

#define MAX_EVENTS 2048 // return less then 32 events per moment (custom unlimited value)

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

    int ePoll = epoll_create1(0);

    struct epoll_event event;
    event.data.fd = masterSocket;
    event.events = EPOLLIN;
    epoll_ctl(ePoll, EPOL_CTL_ADD, masterSocket, &event);

	while(true)
	{
        struct epoll_event events[MAX_EVENTS];
        int n = epoll_wait(ePoll, events, MAX_EVENTS, -1);

        for( size_t i = 0; i < n; ++i)
        {
            if(events[i].data.fd = masterSocket)
            {
                int slaveSocket = accept(masterSocket, 0,0);
                sent_nonblock(slaveSocket);
                struct epoll_event event;
                event.data.fd = slaveSocket;
                event.events = EPOLLIN;
                epoll_ctl(ePoll, EPOLL_CTL_ADD, slaveSocket, &event);
            }
            else
            {
                static char boffer[1500];
                int recvResult = recv(events[i].data.fd, buffer, 1500, MSG_NOSIGNAL);
                if( recvResult == 0 && errno != EAGAIN )
                {
                    shutdown(events[i].data.fd, SHUT_RDWD);
                    close(events[i].data.fd);
                }
                else if (recvResult > 0)
                {
                    send(events[i].data.fd, buffer, recvResult, MSG_NOSIGNAL);
                }
            }
        }
	}
	
	return 0;
}
