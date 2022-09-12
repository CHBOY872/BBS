#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "server.h"

int init_socket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
        return -1;
    socklen_t opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (-1 == bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
        return -1;
    if (listen(fd, QLEN) == -1)
        return -1;
    return 0;
}