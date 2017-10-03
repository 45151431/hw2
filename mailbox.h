#include<sys/types.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#define FILE_SIZE sizeof(mail_t)
#define SIZE_OF_SHORT_STRING 64
#define SIZE_OF_LONG_STRING 512
typedef struct __MAIL{
	int from;
	int type;
	char sstr[SIZE_OF_SHORT_STRING];
	char lstr[SIZE_OF_LONG_STRING];
}mail_t;

typedef void *mailbox_t;

mailbox_t mailbox_open(int id);
int mailbox_unlink(int id);
int mailbox_close(mailbox_t box);
int mailbox_send(mailbox_t box, mail_t *mail);
int mailbox_recv(mailbox_t box, mail_t *mail);
int mailbox_check_empty(mailbox_t box);
int mailbox_check_full(mailbox_t box);

typedef struct __CLIENT{
	int is_join;
	int fd;
	char name[SIZE_OF_SHORT_STRING];
}client_t;

mailbox_t mailbox_open(int id)
{
	char fn[20];
	memset(fn, '\0', sizeof(fn));
	sprintf(fn, "/__mailbox_%d", id);
	int fd = shm_open(fn, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd<0)
		return NULL;
	fcntl(fd,F_SETFL,fcntl(fd,F_GETFL)|O_NONBLOCK);
	return (mailbox_t)fd;
}
int mailbox_unlink(int id)
{
	char fn[20];
	memset(fn, '\0', sizeof(fn));
    sprintf(fn, "/__mailbox_%d", id);
	return shm_unlink(fn);
}
int mailbox_close(mailbox_t box)
{
	return close((int)box);
}
int mailbox_send(mailbox_t box, mail_t *mail)
{
	if(mailbox_check_full(box)==0)
	{
		lseek((int)box, 0, SEEK_END);
		write((int)box, mail, sizeof(mail_t));
		return 0;
	}
	return -1;
}
int mailbox_recv(mailbox_t box, mail_t *mail)
{
	if(mailbox_check_empty(box)==0)
	{
		read((int)box, mail, sizeof(mail_t));
		return 0;
	}
	return -1;
}
int mailbox_check_empty(mailbox_t box)
{
	off_t cur = lseek((int)box, 0, SEEK_CUR);
	off_t end = lseek((int)box, 0, SEEK_END);
	lseek((int)box, cur, SEEK_SET);
	if(cur>=end)
		return 1;
	else
		return 0;
}
int mailbox_check_full(mailbox_t box)
{
	off_t cur = lseek((int)box, 0, SEEK_CUR);
	off_t end = lseek((int)box, 0, SEEK_END);
	lseek((int)box, cur, SEEK_SET);
	if(end>=FILE_SIZE)
		return 1;
	else
		return 0;
}
