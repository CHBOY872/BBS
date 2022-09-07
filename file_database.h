#ifndef FILE_DATABASE_H_SENTRY
#define FILE_DATABASE_H_SENTRY

#include "file_structure.h"

#define WRITING_FORMAT "%250s%250s%4d\n"

void append_file(struct file_structure *file, const char *file_name);

#endif