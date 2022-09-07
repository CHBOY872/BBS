#ifndef FILE_STRUCTURE_H_SENTRY
#define FILE_STRUCTURE_H_SENTRY

#define FILE_NAME_LEN 256
#define USER_NICKNAME_LEN 256

struct file_structure
{
    char file_name[FILE_NAME_LEN];
    char author_nickname[USER_NICKNAME_LEN];
    int perms;
};

#endif