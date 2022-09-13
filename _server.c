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

int init_directory(const char *path, int perms)
{
	DIR *download_dir = opendir(path);
	if (!download_dir)
	{
		int stat = mkdir(path, perms);
		if (-1 == stat)
			return -1;
	}
	else
	{
		chmod(path, 0777);
		closedir(download_dir);
	}
	return 0;
}

int init_programm(char *file_file_path, char *user_file_path,
				  char *directory_path)
{
	int len;
	len = sprintf(user_file_path, "%s/%s", main_dir_path, list_of_users);

	if (len >= MAX_PATH_LEN)
		return -1;

	user_file_path[len] = 0;
	if (init_file(user_file_path, O_CREAT | O_RDWR, 0666))
		return -1;

	len = sprintf(file_file_path, "%s/%s", main_dir_path, list_of_files);
	if (len >= MAX_PATH_LEN)
		return -1;

	file_file_path[len] = 0;
	if (init_file(file_file_path, O_CREAT | O_RDWR, 0666))
		return -1;

	len = sprintf(directory_path, "%s/%s", main_dir_path, downloadings_path);
	if (len >= MAX_PATH_LEN)
		return -1;

	directory_path[len] = 0;
	if (init_directory(directory_path, 0777))
		return -1;

	return 0;
}

int main(int argc, const char **argv)
{
	char file_path[MAX_PATH_LEN];
	char user_path[MAX_PATH_LEN];
	char directory_path[MAX_PATH_LEN];
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
	chmod(main_dir_path, 0777);
	if (-1 == init_programm(file_path, user_path, directory_path))
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

	return run(fd, user_path, file_path, directory_path);
}
