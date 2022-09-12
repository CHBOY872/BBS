#ifndef USER_DATABASE_H_SENTRY
#define USER_DATABASE_H_SENTRY

#include "user_structure.h"

#define WRITING_FORMAT_USER "%250s %250s\n"
#define WRITING_FORMAT_LEN_USER 502

void append_user(struct user_structure *user, const char *file_name);
int edit_user_by_name(struct user_structure *user,
                      const char *user_name, const char *file_path);
int delete_user_by_name(struct user_structure *user,
                        const char *user_name, const char *file_path);
int get_user_by_name(const char *name, struct user_structure *to,
                     const char *file_path);

#endif