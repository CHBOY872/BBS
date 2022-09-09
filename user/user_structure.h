#ifndef USER_STRUCTURE_H_SENTRY
#define USER_STRUCTURE_H_SENTRY

#define USER_NAME 250
#define USER_PASSWORD 250

struct user_structure
{
    char nickname[USER_NAME];
    char password[USER_PASSWORD];
};

#endif