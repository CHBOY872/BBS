#ifndef FILE_DATABASE_H_SENTRY
#define FILE_DATABASE_H_SENTRY

#include "file_structure.h"

#define WRITING_FORMAT "%250s %250s %4o\n"
#define WRITING_FORMAT_LEN 506

void append_file(struct file_structure *file, const char *file_name);
int edit_file_by_name(struct file_structure *file,
                      const char *file_name, const char *file_path);
int delete_file_by_name(struct file_structure *file,
                        const char *file_name, const char *file_path);
int get_file_by_name(const char *name, struct file_structure *to,
                     const char *file_path);

#endif