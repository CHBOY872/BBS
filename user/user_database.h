#ifndef FILE_DATABASE_H_SENTRY
#define FILE_DATABASE_H_SENTRY

#include "user_structure.h"

#define WRITING_FORMAT "%250s %250s\n"
#define WRITING_FORMAT_LEN 502

void append_user(struct user_structure *user, const char *file_name);
int edit_user_by_name(struct user_structure *user,
                      const char *user_name, const char *file_path);
int delete_user_by_name(struct user_structure *user,
                        const char *user_name, const char *file_path);
int get_user_by_name(const char *name, struct user_structure *to,
                     const char *file_path);

#endif