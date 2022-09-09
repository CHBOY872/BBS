#ifndef FILE_STRUCTURE_H_SENTRY
#define FILE_STRUCTURE_H_SENTRY

#define FILE_NAME_LEN 250
#define USER_NICKNAME_LEN 250

struct file_structure
{
    char file_name[FILE_NAME_LEN];
    char author_nickname[USER_NICKNAME_LEN];
    int perms;
};

#endif