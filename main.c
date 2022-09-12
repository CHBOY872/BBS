#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#include "file/file_database.h"
#include "user/user_database.h"
#include "server/server.h"

#define MAX_PATH_LEN 256

static const char list_of_users[] = "list_of_users.txt";
static const char list_of_files[] = "list_of_files.txt";
static const char downloadings_path[] = "downloadings";

static const char *main_dir_path = NULL;

int init_file(const char *path, int flags, int perms)
{
	int fd = open(path, flags, perms);
	if (-1 == fd)
		return -1;
	close(fd);
	return 0;
}

int init_directive(const char *path, int perms)
{
	DIR *download_dir = opendir(path);
	if (!download_dir)
	{
		int stat = mkdir(path, perms);
		if (-1 == stat)
			return -1;
	}
	closedir(download_dir);
	return 0;
}

int init_programm(DIR *main_dir)
{
	char full_path[MAX_PATH_LEN];
	int len;
	len = sprintf(full_path, "%s/%s", main_dir_path, list_of_users);

	if (len >= MAX_PATH_LEN)
		return -1;
	full_path[len] = 0;

	if (init_file(full_path, O_CREAT | O_RDWR, 0666))
		return -1;

	len = sprintf(full_path, "%s/%s", main_dir_path, list_of_files);
	if (len >= MAX_PATH_LEN)
		return -1;

	full_path[len] = 0;

	if (init_file(full_path, O_CREAT | O_RDWR, 0666))
		return -1;

	len = sprintf(full_path, "%s/%s", main_dir_path, downloadings_path);
	if (len >= MAX_PATH_LEN)
		return -1;

	if (init_directive(full_path, 0666))
		return -1;

	return 0;
}

int main(int argc, const char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: <directory>\n");
		return 1;
	}
	DIR *dir = opendir(argv[1]);
	if (!dir)
	{
		fprintf(stderr, "No such directory\n");
		return 2;
	}
	closedir(dir);
	main_dir_path = argv[1];
	if (-1 == init_programm(dir))
	{
		perror("init");
		return 3;
	}
	int fd = init_socket();
	if (-1 == fd)
	{
		perror("socket");
		return 4;
	}
	
	return 0;
}
