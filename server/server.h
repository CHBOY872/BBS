#ifndef SERVER_H_SENTRY
#define SERVER_H_SENTRY

#define QLEN 15
#ifndef BUFFERSIZE
#define BUFFERSIZE 256
#endif

static const char *commands[] = {"register",
                                 "password",
                                 "put",
                                 "get",
                                 "remove",
                                 "rename"};

enum step
{
    step_uninitialized,
    step_unauthorized,
    step_authorized
};

struct session
{
    int fd;
    char buf[BUFFERSIZE];
    enum step st;
};

static int port = 8808;

int init_socket();

#endif