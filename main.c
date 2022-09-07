#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef BUFFERSIZE
#define BUFFERSIZE 256
#endif
#define MAX_PATH_LEN 256

static const char list_of_users[] = "list_of_users.txt";
static const char list_of_files[] = "list_of_files.txt";
static const char downloadings_path[] = "downloadings";

enum step
{
	unauthorized,
	authorized
};

struct session
{
	int fd;
	char buf[BUFFERSIZE];
	enum step st;
};

static const char *main_dir_path = NULL;

int init_programm(DIR *main_dir)
{
	char full_path[MAX_PATH_LEN];
	int len;
	len = sprintf(full_path, "%s/%s", main_dir_path, list_of_users);
	if (len >= MAX_PATH_LEN)
		return -1;
	full_path[len] = 0;
	int fd = open(full_path, O_CREAT | O_RDWR, 0666);
	if (-1 == fd)
		return -1;
	close(fd);
	len = sprintf(full_path, "%s/%s", main_dir_path, list_of_files);
	if (len >= MAX_PATH_LEN)
		return -1;
	full_path[len] = 0;
	fd = open(full_path, O_CREAT | O_RDWR, 0666);
	if (-1 == fd)
	{
		return -1;
	}
	close(fd);
	DIR *download_dir = opendir(downloadings_path);
	if (!download_dir)
	{
		len = sprintf(full_path, "%s/%s", main_dir_path, downloadings_path);
		if (len >= MAX_PATH_LEN)
			return -1;
		int stat = mkdir(full_path, 0666);
		if (-1 == stat)
			return -1;
	}
	closedir(download_dir);
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

	

	return 0;
}
