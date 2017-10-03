#include"mailbox.h"

int main(void)
{
	int fd = (int)mailbox_open(0);
	int m, n, id, i, full;
	char r[512], w[512];
	mail_t mail;
	client_t client[100];

	fcntl(0,F_SETFL,fcntl(0,F_GETFL)|O_NONBLOCK);//stdin
	fcntl(1,F_SETFL,fcntl(1,F_GETFL)|O_NONBLOCK);//stdout

	memset(client, 0, sizeof(client));
	while(1)
	{
		full = mailbox_check_full((mailbox_t)fd);
		memset(&mail, 0, sizeof(mail_t));
		if(mailbox_recv((mailbox_t)fd, &mail)==0)
		{
			if(mail.type==0)
			{
				id = mail.from;
				mail.from = 0;
				client[id].is_join = 1;
				client[id].fd = (int)mailbox_open(id);
				memcpy(client[id].name, mail.sstr, sizeof(client[id].name));
				for(i=0; i<100; i++)
					if(i!=id && client[i].is_join==1)
						if(mailbox_send((mailbox_t)(client[i].fd), &mail)==0)
						{
							m = sprintf(w, "write in %d\n", i);
							write(1, w, m);
						}
				m = sprintf(w, "%s join\n", mail.sstr);
				write(1, w, m);
			}
			else if(mail.type==1)
			{
				id = mail.from;
				mail.from = 0;
				for(i=0; i<100; i++)
					if(i!=id && client[i].is_join==1)
						if(mailbox_send((mailbox_t)(client[i].fd), &mail)==0)
						{
							m = sprintf(w, "write in %d\n", i);
							write(1, w, m);
						}
				m = sprintf(w, "%s broadcast: %s\n", mail.sstr, mail.lstr);
				write(1, w, m);
			}
			else if(mail.type==2)
			{
				id = mail.from;
				mail.from = 0;
				mailbox_close((mailbox_t)client[id].fd);
				client[id].is_join = 0;
				client[id].fd = 0;
				memset(client[id].name, '\0', sizeof(client[id].name));
				for(i=0; i<100; i++)
					if(i!=id && client[i].is_join==1)
						if(mailbox_send((mailbox_t)(client[i].fd), &mail)==0)
						{
							m = sprintf(w, "write in %d\n", i);
							write(1, w, m);
						}
				m = sprintf(w, "%s leave\n", mail.sstr);
				write(1, w, m);
			}
			else if(mail.type==3)
			{
				id = mail.from;
				mail.from = 0;
				char goal[SIZE_OF_SHORT_STRING];
				memcpy(goal, mail.sstr, sizeof(goal));
				memset(mail.sstr, '\0', sizeof(mail.sstr));
				memcpy(mail.sstr, client[id].name, sizeof(mail.sstr));
				for(i=0; i<100; i++)
					if(i!=id && client[i].is_join==1)
						if(strcmp(goal, client[i].name)==0)
							if(mailbox_send((mailbox_t)(client[i].fd), &mail)==0)
							{
								m = sprintf(w, "write in %d\n", i);
								write(1, w, m);
							}
				m = sprintf(w, "%s whisper %s: %s\n", mail.sstr, goal, mail.lstr);
				write(1, w, m);
			}
			else if(mail.type==4)
			{
				id = mail.from;
				mail.from = 0;
				int tmp = 0;
				memset(mail.lstr, '\0', sizeof(mail.lstr));
				for(i=0; i<100; i++)
					if(i!=id && client[i].is_join==1)
					{
						write(1, w, m);
						tmp += sprintf((mail.lstr)+tmp, "%s\n", client[i].name);
					}
				if(mailbox_send((mailbox_t)(client[id].fd), &mail)==0)
				{
					m = sprintf(w, "write in %d\n", id);
					write(1, w, m);
				}
				m = sprintf(w, "%s list\n", mail.sstr);
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
			if(strcmp(r, "leave\n")==0)
				break;
		}
	}

	mailbox_close((mailbox_t)fd);
	mailbox_unlink(0);
	return 0;
}
