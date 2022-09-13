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
static char type_a_file_name_msg[] = "Please type a password without any "
                                     "spaces and '/' '.'\n";
static char type_another_file_name_msg[] = "Type another file name: ";

static char incorrect_cred[] = "Incorrect password or user not found\n";

static char unknown_msg[] = "Unknown...\n";

static char user_is_exist_msg[] = "User with that nickname has already exist\n";
static char success_registation_msg[] = "Registation successfull\n";
static char change_pass_msg[] = "Type a new password: ";
static char reset_pass_fail_msg[] = "Failed reseting password\n";

static char write_name_file_msg[] = "Write a name of file: ";
static char set_perms_msg[] = "Please set permissions to file: ";

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
    int flags = fcntl(client_fd, F_GETFL);
    fcntl(client_fd, F_SETFD, flags | O_NONBLOCK);
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
    sess->file_fd = -1;
    sess->step = step_authorization_uninitialized;
    sess->reg_step = step_registration_no;
    sess->name = NULL;
    sess->user = NULL;
    sess->buf_used = 0;
    sess->want_read = 1;
    sess->want_write = 0;
    memset(sess->buf, 0, BUFFERSIZE);
    return sess;
}

void end_session(struct session **sess) /* delete session */
{
    close((*sess)->fd);
    close((*sess)->file_fd);
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
    struct user_structure user;
    switch (sess->step)
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
                send_msg(sess->fd, type_password_msg,
                         sizeof(type_password_msg));
            else
            {
                strcpy(sess->user->password, msg);
                append_user(sess->user, user_file_path);
                sess->reg_step = step_registration_no;
                sess->step = step_authorization_noauthorized;
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
            sess->step = step_authorization_unauthorized_login;
            send_msg(sess->fd, login_msg, sizeof(login_msg));
        }
        else if (!strcmp("N", msg) || !strcmp("n", msg))
        {
            sess->step = step_authorization_noauthorized;
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
            sess->step = step_authorization_unauthorized_password;
            send_msg(sess->fd, password_msg, sizeof(password_msg));
        }
        break;
    case step_authorization_unauthorized_password:
        if (!strcmp("", msg) || strstr(msg, " "))
            send_msg(sess->fd, type_password_msg, sizeof(type_password_msg));
        else
        {
            if (-1 == get_user_by_name(sess->name, &user, user_file_path))
            {
                free(sess->name);
                sess->name = NULL;
                sess->step = step_authorization_uninitialized;
                send_msg(sess->fd, incorrect_cred, sizeof(incorrect_cred));
                send_msg(sess->fd, account_have_msg, sizeof(account_have_msg));
            }
            else
            {
                if (!strcmp(msg, user.password))
                    sess->step = step_authorization_authorized;
                else
                {
                    send_msg(sess->fd, incorrect_cred, sizeof(incorrect_cred));
                    send_msg(sess->fd, account_have_msg,
                             sizeof(account_have_msg));
                    sess->step = step_authorization_uninitialized;
                }
            }
        }
        break;
    case step_authorization_authorized:
        if (!strcmp(msg, commands[1])) /* logout */
        {
            free(sess->name);
            sess->step = step_authorization_noauthorized;
            send_msg(sess->fd, no_authorized_msg, sizeof(no_authorized_msg));
        }
        else if (!strcmp(msg, commands[4])) /* password */
        {
            sess->step = step_authorization_change_password;
            send_msg(sess->fd, change_pass_msg, sizeof(change_pass_msg));
        }
        else if (!strcmp(msg, commands[5])) /* put */
        {
            sess->step = step_want_put;
            send_msg(sess->fd, write_name_file_msg,
                     sizeof(write_name_file_msg));
        }

    case step_authorization_noauthorized:
        if (!strcmp(msg, commands[2])) /* q */
            return -1;
        else if (!strcmp(msg, commands[0])) /* login */
        {
            sess->step = step_authorization_unauthorized_login;
            send_msg(sess->fd, login_msg, sizeof(login_msg));
        }
        else if (!strcmp(msg, commands[3])) /* register */
        {
            sess->step = step_authorization_register;
            sess->reg_step = step_registration_login;
            send_msg(sess->fd, login_msg, sizeof(login_msg));
        }
        break;
    case step_authorization_change_password:
        strcpy(user.nickname, sess->name);
        strcpy(user.password, msg);
        if (-1 == edit_user_by_name(&user, sess->name, user_file_path))
        {
            sess->step = step_authorization_authorized;
            send_msg(sess->fd, reset_pass_fail_msg,
                     sizeof(reset_pass_fail_msg));
        }
        sess->step = step_authorization_authorized;
        break;
    case step_want_put:
        if (strstr(msg, " ") || strstr(msg, "/") || msg[0] == '.')
            send_msg(sess->fd, type_a_file_name_msg,
                     sizeof(type_a_file_name_msg));
        else
        {
            char *file_name = malloc(sizeof(char) * strlen(msg) + 2 +
                                     strlen(directive_path));
            sprintf(file_name, "%s/%s", directive_path, msg);
            if (-1 == get_file_by_name(msg, NULL, file_file_path))
            {
                sess->file_fd = open(file_name, O_CREAT | O_WRONLY, 0666);
                if (sess->file_fd == -1)
                    send_msg(sess->fd, type_another_file_name_msg,
                             sizeof(type_another_file_name_msg));
                else
                {
                    sess->file = malloc(sizeof(struct file_structure));
                    strcpy(sess->file->author_nickname, sess->name);
                    strcpy(sess->file->file_name, msg);
                    sess->file->perms = 0;
                    append_file(sess->file, file_file_path);
                    sess->step = step_set_perms;
                    send_msg(sess->fd, set_perms_msg, sizeof(set_perms_msg));
                }
            }
            else
                send_msg(sess->fd, type_another_file_name_msg,
                         sizeof(type_another_file_name_msg));
            free(file_name);
        }
        break;
    case step_is_put:
        sscanf(msg, "%4o", &(sess->file->perms));
        edit_file_by_name(sess->file, sess->file->file_name, file_file_path);

        free(sess->file);
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
                    if (sess[i]->want_read)
                        FD_SET(i, &rds);
                    if (sess[i]->want_write)
                        FD_SET(i, &wrs);
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
                                                 file_file_path,
                                                 directive_path))
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
