#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/ip.h>

int count = 0, max_fd = 0;
int ids[65536];
char *msgs[65536];

fd_set rfds, wfds, afds;
char buf_read[1001], buf_write[42];

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void brodcast(int sender, char *msg)
{
	for (int fd = 0; fd <= max_fd; fd++)
		if (FD_ISSET(fd, &wfds) && fd != sender)
			send(fd, msg, strlen(msg), 0);
}

void join_client(int fd)
{
	max_fd = fd > max_fd ? fd : max_fd;
	ids[fd] = count++;
	msgs[fd] = NULL;
	FD_SET(fd, &afds);
	sprintf(buf_write, "server: client %d just arrived\n", ids[fd]);
	brodcast(fd, buf_write);
}

void rm_client(int fd)
{
	sprintf(buf_write, "server: client %d just left\n", ids[fd]);
	brodcast(fd, buf_write);
	free(msgs[fd]);
	FD_CLR(fd, &afds);
	close(fd);
}

void send_msg(int fd)
{
	char *msg;
	
	while (extract_message(&(msgs[fd]), &msg))
	{
		sprintf(buf_write, "client %d: ", ids[fd]);
		brodcast(fd, buf_write);
		brodcast(fd, msg);
		free(msg);
	}
}

void err(char *str)
{
	write(2, str, strlen(str));
	exit(1);
}

int create_socket()
{
	max_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (max_fd < 0)
		err("Fatal error\n");
	FD_SET(max_fd, &afds);
	return max_fd;
}

int main(int ac, char **av)
{
	if (ac != 2)
		err("Wrong number of arguments\n");
	
	FD_ZERO(&afds);
	int sockfd = create_socket();

	struct sockaddr_in servaddr, cli;
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		err("Fatal error\n");
	if (listen(sockfd, SOMAXCONN))
		err("Fatal error\n");

	while (1)
	{
		rfds = wfds = afds;

		if (select(max_fd+1, &rfds, &wfds, NULL, NULL) < 0)
			err("Fatal error\n");

		for (int fd = 0; fd <= max_fd; fd++)
		{
			if (!FD_ISSET(fd, &rfds))
				continue;

			if (fd == sockfd)
			{
				socklen_t addr_len = sizeof(servaddr);
				int cfd = accept(sockfd, (struct sockaddr *)&servaddr, &addr_len);
				if (cfd >= 0)
				{
					join_client(cfd);
					break;
				}
			}
			else
			{
				int res = recv(fd, buf_read, 1000, 0);
				if (res <= 0)
				{
					rm_client(fd);
					break;
				}
				buf_read[res] = '\0';
				msgs[fd] = str_join(msgs[fd], buf_read);
				send_msg(fd);
			}
		}
	}
	return (0);
}