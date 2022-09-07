#ifndef USER_STRUCTURE_H_SENTRY
#define USER_STRUCTURE_H_SENTRY

#define USER_NAME 256
#define USER_PASSWORD 256

struct user
{
    char nickname[USER_NAME];
    char password[USER_PASSWORD];
};

#endif