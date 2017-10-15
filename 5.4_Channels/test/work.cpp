#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sockfdio.h"

void child(int sock)
{
    int fd;
    char buf[16];
    ssize_t size;

    sleep(1);
    for (;;) {
        size = sock_fd_read(sock, buf, sizeof(buf), &fd);
        if (size <= 0)
            break;
        printf ("read %d\n", size);
        if (fd != -1) {
            write(fd, "hello, world\n", 13);
            close(fd);
        }
    }
}

void parent(int sock)
{
    ssize_t size;
    int i;
    int fd;

    fd = 1;
    size = sock_fd_write(sock, (void*)"1", 1, 1);
    printf ("wrote %d\n", size);
}

int main()
{
    int sv[2];
    int pid;

    if(socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) < 0)
    {
        perror("socketpair");
        exit(1);
    }

    pid = fork();

    switch(pid)
    {
        case 0:
            close(*sv);
            child(sv[1]);
            break;
        case -1:
            perror("fork");
            exit(1);
        default:
            close(sv[1]);
            parent(*sv);
            break;
    }

    return 0;
}

