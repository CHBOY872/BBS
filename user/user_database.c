#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user_database.h"

int get_user_by_name(const char *name, struct user_structure *to,
                     const char *file_path)
{
    FILE *f = fopen(file_path, "r");
    if (!f)
        return -1;

    int i = 0;
    while (fscanf(f, WRITING_FORMAT, to->nickname, to->password) != EOF)
    {
        if (!strcmp((char *)name, to->nickname))
        {
            fclose(f);
            return i;
        }
        i++;
    }
    fclose(f);
    return -1;
}

void append_user(struct user_structure *user, const char *file_name)
/* write a file in the last position */
{
    FILE *to = fopen(file_name, "a");
    if (!to)
        return;
    fprintf(to, WRITING_FORMAT, user->nickname, user->password);
    fclose(to);
}

int edit_user_by_name(struct user_structure *user,
                      const char *user_name, const char *file_path)
{
    struct user_structure temp;
    int stat = get_user_by_name(user_name, &temp, file_path);
    if (stat == -1)
        return -1;
    FILE *where = fopen(file_path, "r+");
    if (!where)
    {
        perror("edit user");
        return -1;
    }

    fseek(where, WRITING_FORMAT_LEN * stat, SEEK_SET);
    fprintf(where, WRITING_FORMAT, user->nickname, user->password);
    fclose(where);
    return stat;
}

int delete_user_by_name(struct user_structure *user,
                        const char *user_name, const char *file_path)
{
    struct user_structure temp;
    int stat = get_user_by_name(user_name, &temp, file_path);
    if (stat == -1)
        return -1;
    FILE *where = fopen(file_path, "r+");
    if (!where)
    {
        perror("delete user");
        return -1;
    }

    fseek(where, WRITING_FORMAT_LEN * stat, SEEK_SET);
    fprintf(where, WRITING_FORMAT, "NULL", "NULL");
    fclose(where);
    return stat;
}