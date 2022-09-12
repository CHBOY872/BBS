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

#include "../user/user_database.h"
#include "../file/file_database.h"

#include "server.h"

static char greetings_msg[] = "Welcome!\n";
static char account_have_msg[] = "Do you have an account? ([Y/y] - yes,"
                                 " [N/n] - no) : ";
static char no_authorized_msg[] = "You are connected as NOauthorized user\n";
static char login_msg[] = "Type your nickname: ";
static char password_msg[] = "Type your password: ";

static char type_nickname_msg[] = "Please type a nickname without any spaces\n";
static char type_password_msg[] = "Please type a password without any spaces\n";
static char incorrect_cred[] = "Incorrect password or user not found\n";

static char unknown_msg[] = "Unknown...\n";

static char user_is_exist_msg[] = "User with that nickname has already exist\n";
static char success_registation_msg[] = "Registation successfull\n";

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
    return fd;
}

void close_server(struct session ***sess, int len)
{
    int i;
    for (i = 0; i <= len; i++)
    {
        if ((*sess)[i])
            end_session(&(*sess)[i]);
    }
    free(*sess);
}

void find_max_descriptor(struct session **sess, int *max_fd)
{
    int i;
    int len = *max_fd;
    for (i = 0; i <= len; i++)
    {
        if (sess[i])
        {
            if (*max_fd < i)
                *max_fd = i;
        }
    }
}

void send_msg(int fd, const char *msg, int size)
{
    write(fd, msg, size);
}

void accept_client(int server_fd, int *max_fd, struct session ***sess)
{ /* accept connection to server */
    int i;
    struct sockaddr_in client_addr;
    socklen_t len = 0;

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
    if (-1 == client_fd)
        return;

    if (!*sess)
    {
        if (*max_fd < client_fd)
            *max_fd = client_fd;
        init_sessions(sess, *max_fd);
    }
    if (*max_fd < client_fd)
    {
        struct session **sess_tmp =
            malloc(sizeof(struct session *) * (client_fd + 1));
        for (i = 0; i <= client_fd; i++)
            sess_tmp[i] = i <= *max_fd ? (*sess)[i] : NULL;
        *max_fd = client_fd;
        free(*sess);
        *sess = sess_tmp;
    }
    (*sess)[client_fd] = make_session(client_fd);
    send_msg(client_fd, greetings_msg, sizeof(greetings_msg));
    send_msg(client_fd, account_have_msg, sizeof(account_have_msg));
}

struct session *make_session(int fd)
{
    struct session *sess = malloc(sizeof(struct session));
    sess->fd = fd;
    sess->auth_step = step_authorization_uninitialized;
    sess->reg_step = step_registration_no;
    sess->name = NULL;
    sess->user = NULL;
    sess->buf_used = 0;
    memset(sess->buf, 0, BUFFERSIZE);
    return sess;
}

void end_session(struct session **sess) /* delete session */
{
    close((*sess)->fd);
    if ((*sess)->name)
        free((*sess)->name);
    free(*sess);
    *sess = NULL;
}

void init_sessions(struct session ***sess, int len)
{
    *sess = malloc(sizeof(struct session *) * (len + 1));
    int i;
    for (i = 0; i < len; i++)
        (*sess)[i] = NULL;
}

int handle(const char *msg, struct session *sess, const char *user_file_path,
           const char *file_file_path, const char *directive_path)
{
    switch (sess->auth_step)
    {
    case step_authorization_register:

        switch (sess->reg_step)
        {
        case step_registration_login:
            if (!strcmp("", msg) || strstr(msg, " "))
                send_msg(sess->fd, type_nickname_msg,
                         sizeof(type_nickname_msg));
            else
            {
                sess->user = malloc(sizeof(struct user_structure));
                if (-1 != get_user_by_name(msg, sess->user, user_file_path))
                    send_msg(sess->fd, user_is_exist_msg,
                             sizeof(user_is_exist_msg));
                else
                {
                    bzero(sess->user->nickname, USER_NAME);
                    bzero(sess->user->password, USER_PASSWORD);
                    strcpy(sess->user->nickname, msg);
                    sess->reg_step = step_registration_password;
                    send_msg(sess->fd, password_msg, sizeof(password_msg));
                }
            }
            break;
        case step_registration_password:
            if (!strcmp("", msg) || strstr(msg, " "))
                send_msg(sess->fd, type_nickname_msg,
                         sizeof(type_password_msg));
            else
            {
                strcpy(sess->user->password, msg);
                append_user(sess->user, user_file_path);
                sess->reg_step = step_registration_no;
                sess->auth_step = step_authorization_noauthorized;
                send_msg(sess->fd, success_registation_msg,
                         sizeof(success_registation_msg));
                free(sess->user);
                sess->user = NULL;
            }
            break;
        default:
            break;
        }

        break;
    case step_authorization_uninitialized:
        if (!strcmp("Y", msg) || !strcmp("y", msg))
        {
            sess->auth_step = step_authorization_unauthorized_login;
            send_msg(sess->fd, login_msg, sizeof(login_msg));
        }
        else if (!strcmp("N", msg) || !strcmp("n", msg))
        {
            sess->auth_step = step_authorization_noauthorized;
            send_msg(sess->fd, no_authorized_msg, sizeof(no_authorized_msg));
        }
        else
            send_msg(sess->fd, unknown_msg, sizeof(unknown_msg));
        break;
    case step_authorization_unauthorized_login:
        if (!strcmp("", msg) || strstr(msg, " "))
            send_msg(sess->fd, type_nickname_msg, sizeof(type_nickname_msg));
        else
        {
            int name_len = strlen(msg);
            sess->name = malloc(name_len + 1);
            strcpy(sess->name, msg);
            sess->name[name_len] = 0;
            sess->auth_step = step_authorization_unauthorized_password;
            send_msg(sess->fd, password_msg, sizeof(password_msg));
        }
        break;
    case step_authorization_unauthorized_password:
        if (!strcmp("", msg) || strstr(msg, " "))
            send_msg(sess->fd, type_password_msg, sizeof(type_password_msg));
        else
        {
            struct user_structure user;
            if (-1 == get_user_by_name(sess->name, &user, user_file_path))
            {
                free(sess->name);
                sess->name = NULL;
                sess->auth_step = step_authorization_uninitialized;
                send_msg(sess->fd, incorrect_cred, sizeof(incorrect_cred));
                send_msg(sess->fd, account_have_msg, sizeof(account_have_msg));
            }
            else
                sess->auth_step = step_authorization_authorized;
        }
        break;
    case step_authorization_authorized:
        if (!strcmp(msg, commands[1]))
        {
            free(sess->name);
            sess->auth_step = step_authorization_noauthorized;
            send_msg(sess->fd, no_authorized_msg, sizeof(no_authorized_msg));
        }
    case step_authorization_noauthorized:
        if (!strcmp(msg, commands[2]))
            return -1;
        else if (!strcmp(msg, commands[0]))
        {
            sess->auth_step = step_authorization_unauthorized_login;
            send_msg(sess->fd, login_msg, sizeof(login_msg));
        }
        else if (!strcmp(msg, commands[3]))
        {
            sess->auth_step = step_authorization_register;
            sess->reg_step = step_registration_login;
            send_msg(sess->fd, login_msg, sizeof(login_msg));
        }
        break;
    default:
        break;
    }
    return 0;
}

int session_handle(struct session *sess, const char *user_file_path,
                   const char *file_file_path, const char *directive_path)
{
    int fd = sess->fd;
    int i, buf_used = sess->buf_used, pos = -1;
    int rc = read(fd, sess->buf, BUFFERSIZE - sess->buf_used);
    if (rc < 0)
        return -1;
    if (rc + sess->buf_used > BUFFERSIZE)
        return -1;
    for (i = 0; i < rc; i++)
    {
        if (sess->buf[buf_used + i] == '\n')
        {
            pos = i;
            break;
        }
    }
    if (pos == -1)
    {
        sess->buf_used += rc;
        return 1;
    }

    char *str = malloc(sizeof(char) * (pos + buf_used));
    memcpy(str, sess->buf, buf_used + pos);
    if (str[buf_used + pos - 1] == '\r')
        str[buf_used + pos - 1] = 0;
    int stat = handle(str, sess, user_file_path, file_file_path,
                      directive_path);
    free(str);
    bzero(sess->buf, sess->buf_used + rc);
    sess->buf_used = 0;
    return stat;
}

int run(int fd_server, const char *user_file_path,
        const char *file_file_path, const char *directive_path)
/* main server cycle */
{
    fd_set rds, wrs;
    int max_fd = fd_server;
    int i, last_fd;
    struct session **sess = NULL;
    for (;;)
    {
        FD_ZERO(&rds);
        FD_ZERO(&wrs);
        FD_SET(fd_server, &rds);
        if (sess)
        {
            for (i = 0; i <= max_fd; i++)
            {
                if (sess[i])
                {
                    FD_SET(i, &rds);
                    last_fd = i;
                }
            }
            if (last_fd > max_fd)
                max_fd = last_fd;
        }

        int stat = select(max_fd + 1, &rds, &wrs, NULL, NULL);
        if (stat == -1)
        {
            perror("select");
            return 1;
        }

        if (FD_ISSET(fd_server, &rds))
            accept_client(fd_server, &max_fd, &sess);

        if (sess)
        {
            for (i = 0; i <= max_fd; i++)
            {
                if (sess[i])
                {
                    if (FD_ISSET(i, &rds))
                    {
                        if (-1 == session_handle(sess[i], user_file_path,
                                                 file_file_path, directive_path))
                        {
                            end_session(&sess[i]);
                            find_max_descriptor(sess, &max_fd);
                        }
                    }
                }
            }
        }
    }
    close_server(&sess, max_fd);
    return 0;
}
