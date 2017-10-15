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
#include <sys/event.h>

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
# ifdef SO_NOSIGPIPE
#  define CEPH_USE_SO_NOSIGPIPE
# else
#  error "Cannot block SIGPIPE!"
# endif
#endif


#define EV_SIZE 32 // return less then 32 events per moment (custom unlimited value)

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

    int kQueue = kqueue();

    struct kevent kEvents[EV_SIZE];
    size_t kevSize = sizeof(*kEvents)*EV_SIZE;
    bzero(kEvents, kevSize );
    EV_SET(kEvents, masterSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
    kevent(kQueue, kEvents, 1, nullptr, 0, nullptr);
    std::set<int> slaves;
	while(true)
	{
        size_t kevSize = sizeof(*kEvents)*EV_SIZE;
        bzero(kEvents, kevSize );
        int n = kevent(kQueue, nullptr, 0, kEvents, EV_SIZE, nullptr);
        
        if( n <= 0 ) continue;

        for( size_t i = 0; i < n; ++i)
        {
            auto& ke = kEvents[i];
            if(ke.filter == EVFILT_READ)
            {
                if(ke.ident == masterSocket)
                {
                    int slaveSocket = accept(masterSocket, 0, 0);
                    set_nonblock(slaveSocket);

                    bzero(&ke, sizeof(ke));
                    EV_SET(&ke, slaveSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
                    slaves.insert(slaveSocket);
                    kevent(kQueue, &ke, 1, nullptr, 0, nullptr);
                }
                else
                {
                    static char buffer[1500];
                    int recvSize = recv(ke.ident, buffer, 1500, MSG_NOSIGNAL);
                    if(recvSize <= 0)
                    {
                        slaves.erase(ke.ident);
                        close(ke.ident);
                        std::cerr << "disconnected!" << std::endl;
                    }
                    else
                    {
                        for(auto slave : slaves)
                        {
                            send(slave, buffer, recvSize, MSG_NOSIGNAL);
                        }
                    }
                }
            }
        }
	}
	
	return 0;
}
