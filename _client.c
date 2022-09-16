#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFFERSIZE 1024

enum steps
{
    step_dialog,
    step_commands
};

enum put_get_steps
{
    put_get_step_nothing,

    put_get_step_put_start,
    put_get_step_put_perms,
    put_get_step_put_error,

    put_get_step_get_start,
    put_get_step_get_name,
    put_get_step_get_errors
};

enum auth_steps
{
    auth_step_unknown,
    auth_step_authorized,
    auth_step_no_authorized
};

static const char *commands[] = {"login",    /* log in to account */
                                 "logout",   /* logout from an account */
                                 "q",        /* quit */
                                 "register", /* register an acoount */
                                 "password", /* change password */
                                 "put",      /* put a file */
                                 "get",      /* get a file */
                                 "remove",   /* remove a file */
                                 "rename"};  /* rename a file */

static const char *responds[] = {"REGISTER\n",
                                 "NOREGISTER\n",
                                 "WRITE\n",
                                 "READ\n",
                                 "DIALOG\n",
                                 "ENDDIALOG\n"};

struct client
{
    enum steps st;
    enum put_get_steps pg_st;
    enum auth_steps au_st;

    int fd_from;
    int fd_to;

    char buffer[BUFFERSIZE];
    int buf_used;

    char want_read;
    char want_write;
};

void take_file(int fd_from, int fd_to, char *buffer, int size)
{
    memset(buffer, 0, size);
    int rc;
    do
    {
        rc = read(fd_from, buffer, size);
        write(fd_to, buffer, rc);
        memset(buffer, 0, size);
    } while (rc == size);
}

int main(int argc, const char **argv)
{
    struct client cl;
    cl.st = step_dialog;
    cl.au_st = auth_step_unknown;
    cl.buf_used = 0;
    memset(cl.buffer, 0, BUFFERSIZE);
    cl.fd_from = 0;
    cl.fd_to = 0;
    cl.want_read = 1;
    cl.want_write = 0;
    cl.pg_st = put_get_step_nothing;

    if (argc != 3)
    {
        write(1, "Usage: <ip> <port>\n", 20);
        return 1;
    }

    cl.fd_to = socket(AF_INET, SOCK_STREAM, 0);
    if (cl.fd_to == -1)
    {
        perror("socket");
        return 2;
    }

    int opt = 1;
    setsockopt(cl.fd_to, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr_server;
    addr_server.sin_addr.s_addr = inet_addr(argv[1]);
    addr_server.sin_port = htons(atoi(argv[2]));
    addr_server.sin_family = AF_INET;

    if (-1 ==
        connect(cl.fd_to, (struct sockaddr *)&addr_server,
                sizeof(addr_server)))
    {
        perror("connect");
        return 3;
    }

    fd_set rds, wrs;
    for (;;)
    {
        FD_ZERO(&rds);
        FD_ZERO(&wrs);
        FD_SET(0, &rds); /* 0 - the standard input stream */
        FD_SET(cl.fd_to, &rds);
        int stat = select(cl.fd_to + 1, &rds, &wrs, NULL, NULL);
        if (stat <= 0)
        {
            perror("select");
            return 4;
        }
        if (FD_ISSET(cl.fd_to, &rds))
        {
            int rc = read(cl.fd_to, cl.buffer, BUFFERSIZE - cl.buf_used);
            if (rc == -1)
            {
                perror("read");
                return 5;
            }
            if (rc == 0)
                break;

            if (strstr(cl.buffer, responds[0])) /* REGISTER */
            {
                cl.au_st = auth_step_authorized;
            }
            if (strstr(cl.buffer, responds[1])) /* NOREGISTER */
            {
                cl.au_st = auth_step_no_authorized;
            }
            if (strstr(cl.buffer, responds[2])) /* WRITE */
            {
                take_file(cl.fd_from, cl.fd_to, cl.buffer, BUFFERSIZE);
                close(cl.fd_from);
                cl.st = step_commands;
                cl.pg_st = put_get_step_nothing;
            }
            if (strstr(cl.buffer, responds[3])) /* READ */
            {
                write(cl.fd_to, "1\n", 3);
                take_file(cl.fd_to, cl.fd_from, cl.buffer, BUFFERSIZE);
                close(cl.fd_from);
                cl.st = step_commands;
                cl.pg_st = put_get_step_nothing;
            }
            if (strstr(cl.buffer, responds[4])) /* DIALOG */
            {
                cl.st = step_dialog;
            }
            if (strstr(cl.buffer, responds[5])) /* ENDDIALOG */
            {
                cl.st = step_commands;
            }
            write(1, cl.buffer, rc); /* 1 - the standard output stream */
            memset(cl.buffer, 0, rc);
        }
        if (FD_ISSET(0, &rds))
        {
            int rc = read(0, cl.buffer, BUFFERSIZE - cl.buf_used);
            if (rc == -1)
            {
                perror("read");
                return 5;
            }
            char *msg = malloc(sizeof(char) * rc + cl.buf_used);
            memmove(msg, cl.buffer, rc + cl.buf_used);
            msg[cl.buf_used + rc - 1] = 0;

            switch (cl.pg_st)
            {
            case put_get_step_put_start:
                cl.fd_from = open(msg, O_RDONLY, 0666);
                if (-1 == cl.fd_from)
                    write(cl.fd_to, "/", 2);
                else
                    cl.pg_st = put_get_step_put_perms;
                break;
            case put_get_step_get_start:
                cl.fd_from = open(msg, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (-1 == cl.fd_from)
                    write(cl.fd_to, "/", 2);
                else
                    cl.pg_st = put_get_step_get_name;
                break;
            default:
                break;
            }

            if (!strcmp(msg, commands[5]))
            {
                if (cl.au_st == auth_step_authorized &&
                    cl.pg_st == put_get_step_nothing)
                    cl.pg_st = put_get_step_put_start;
            }
            else if (!strcmp(msg, commands[6]))
            {
                if (cl.au_st == auth_step_authorized ||
                    cl.au_st == auth_step_no_authorized)
                    cl.pg_st = put_get_step_get_start;
            }

            write(cl.fd_to, cl.buffer, rc); /* 1 - the standard
                                            output stream */

            free(msg);
            memset(cl.buffer, 0, rc);
        }
    }

    return 0;
}