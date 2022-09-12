#ifndef SERVER_H_SENTRY
#define SERVER_H_SENTRY

#define QLEN 15
#ifndef BUFFERSIZE
#define BUFFERSIZE 256
#endif

static const char *commands[] = {"login",
                                 "register",
                                 "password",
                                 "put",
                                 "get",
                                 "remove",
                                 "rename"};

enum authorization_step
{
    step_authorization_uninitialized,
    step_authorization_noauthorized, /* Connected without an account */

    step_authorization_unauthorized_login,
    step_authorization_unauthorized_password,
    step_authorization_authorized
};

struct session
{
    char *name;
    int fd;
    char buf[BUFFERSIZE];
    int buf_used;
    enum authorization_step auth_step;
};

static int port = 8808;

int init_socket();

void init_sessions(struct session ***sess, int len);
int run(int fd_server, const char *user_file_path,
        const char *file_file_path, const char *directive_path);
void close_server(struct session ***sess, int len);

struct session *make_session(int fd);
void end_session(struct session **sess);

void accept_client(int server_fd, int *max_fd, struct session ***sess);

void find_max_descriptor(struct session **sess, int *max_fd);

int session_handle(struct session *sess, const char *user_file_path,
                   const char *file_file_path, const char *directive_path);

#endif
