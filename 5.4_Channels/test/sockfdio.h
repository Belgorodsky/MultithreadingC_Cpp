#ifndef SOCKFDIO_H
#define SOCKFDIO_H

#include <stdlib.h>
#include <unistd.h>

ssize_t
sock_fd_write(int sock, void *buf, ssize_t buflen, int fd);

ssize_t
sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd);

#endif// SOCKFDIO_H
