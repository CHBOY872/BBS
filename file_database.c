#include <stdio.h>
#include <stdlib.h>

#include "file_database.h"

void append_file(struct file_structure *file, const char *file_name)
/* write a file in the last position */
{
    FILE *to = fopen(file_name, "a");
    if (!to)
        return;
    fprintf(to, WRITING_FORMAT,
            file->file_name, file->author_nickname, file->perms);
    fclose(to);
}
