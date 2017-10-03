#include"mailbox.h"

int main(void)
{
	int server_fd = (int)mailbox_open(0);
	int fd, n, m, full;
	char r[512], w[512];
	mail_t mail, r_mail;

	fcntl(0,F_SETFL,fcntl(0,F_GETFL)|O_NONBLOCK);//stdin
	fcntl(1,F_SETFL,fcntl(1,F_GETFL)|O_NONBLOCK);//stdout

	memset(&mail, 0, sizeof(mail_t));
	m = sprintf(w, "please input your id: ");
	write(1, w, m);
	while(1)
	{
		memset(r, '\0', sizeof(r));
		n = read(0, r, sizeof(r));
		if(n>0)
		{
			mail.from = atoi(r);
			break;
		}
	}
	m = sprintf(w, "please input your name: ");
	write(1, w, m);
	while(1)
	{
		memset(r, '\0', sizeof(r));
		n = read(0, r, sizeof(r));
		if(n>0)
		{
			memset(mail.sstr, '\0', sizeof(mail.sstr));
			memcpy(mail.sstr, r, n-1);
			break;
		}
	}
	fd = (int)mailbox_open(mail.from);
	mail.type = 0;
	while(1)
		if(mailbox_send((mailbox_t)server_fd, &mail)==0)
			break;
	m = sprintf(w, "join\n");
	write(1, w, m);
	m = sprintf(w, "input format:\"type string\" (type 1.broadcast 2.leave 3.whisper 4.list)\n");
	write(1, w, m);
	while(1)
	{
		full = mailbox_check_full((mailbox_t)fd);
		memset(&r_mail, 0, sizeof(mail_t));
		if(mailbox_recv((mailbox_t)fd, &r_mail)==0)
		{
			if(r_mail.type==0)
			{
				m = sprintf(w, "%s join\n", r_mail.sstr);
				write(1, w, m);
			}
			else if(r_mail.type==1)
			{
				m = sprintf(w, "%s broadcast: %s\n", r_mail.sstr, r_mail.lstr);
				write(1, w, m);
			}
			else if(r_mail.type==2)
			{
				m = sprintf(w, "%s leave\n", r_mail.sstr);
				write(1, w, m);
			}
			else if(r_mail.type==3)
			{
				m = sprintf(w, "%s whisper: %s\n", r_mail.sstr, r_mail.lstr);
				write(1, w, m);
			}
			else if(r_mail.type==4)
			{
				m = sprintf(w, "list:\n%s", r_mail.lstr);
				write(1, w, m);
			}
		}
		else if(full==1)
		{
			ftruncate(fd, 0);
			lseek(fd, 0, SEEK_SET);
		}
		memset(r, '\0', sizeof(r));
		n = read(0, r, sizeof(r));
		if(n>0)
		{
			mail.type = r[0]-'0';
			if(mail.type==1)
			{
				memset(mail.lstr, '\0', sizeof(mail.lstr));
				sscanf(r, "%d %s", &(mail.type), mail.lstr);
				mailbox_send((mailbox_t)server_fd, &mail);
			}
			else if(mail.type==2)
			{
				mailbox_send((mailbox_t)server_fd, &mail);
				break;
			}
			else if(mail.type==3)
			{
				mail_t w_mail;
				w_mail.from = mail.from;
				memset(w_mail.sstr, '\0', sizeof(w_mail.sstr));
				memset(w_mail.lstr, '\0', sizeof(w_mail.lstr));
				sscanf(r, "%d %s %s", &(w_mail.type), w_mail.sstr, w_mail.lstr);
				mailbox_send((mailbox_t)server_fd, &w_mail);
			}
			else if(mail.type==4)
				mailbox_send((mailbox_t)server_fd, &mail);
		}
	}
	mailbox_close((mailbox_t)server_fd);
	mailbox_close((mailbox_t)fd);
	mailbox_unlink(mail.from);
	return 0;
}
