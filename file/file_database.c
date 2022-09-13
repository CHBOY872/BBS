#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_database.h"

int get_file_by_name(const char *name, struct file_structure *to,
                     const char *file_path)
{
    struct file_structure *tmp = to;
    if (!to)
        tmp = malloc(sizeof(struct file_structure));
    FILE *f = fopen(file_path, "r");
    if (!f)
        return -1;

    int i = 0;
    while (fscanf(f, WRITING_FORMAT_FILE, tmp->file_name,
                  tmp->author_nickname, &tmp->perms) != EOF)
    {
        if (!strcmp((char *)name, tmp->file_name))
        {
            fclose(f);
            return i;
        }
        i++;
    }
    fclose(f);
    if (!to)
        free(tmp);
    return -1;
}

void append_file(struct file_structure *file, const char *file_name)
/* write a file in the last position */
{
    FILE *to = fopen(file_name, "a");
    if (!to)
        return;
    fprintf(to, WRITING_FORMAT_FILE,
            file->file_name, file->author_nickname, file->perms);
    fclose(to);
}

int edit_file_by_name(struct file_structure *file,
                      const char *file_name, const char *file_path)
{
    struct file_structure temp;
    int stat = get_file_by_name(file_name, &temp, file_path);
    if (stat == -1)
        return -1;
    FILE *where = fopen(file_path, "r+");
    if (!where)
    {
        perror("edit file");
        return -1;
    }

    fseek(where, WRITING_FORMAT_LEN_FILE * stat, SEEK_SET);
    fprintf(where, WRITING_FORMAT_FILE, file->file_name,
            file->author_nickname, file->perms);
    fclose(where);
    return stat;
}

int delete_file_by_name(struct file_structure *file,
                        const char *file_name, const char *file_path)
{
    struct file_structure temp;
    int stat = get_file_by_name(file->file_name, &temp, file_path);
    if (stat == -1)
        return -1;
    FILE *where = fopen(file_path, "r+");
    if (!where)
    {
        perror("delete file");
        return -1;
    }

    fseek(where, WRITING_FORMAT_LEN_FILE * stat, SEEK_SET);
    fprintf(where, WRITING_FORMAT_FILE, "NULL", "NULL", 0000);
    fclose(where);
    return stat;
}