#include <iostream>
#include <algorithm>
#include <set>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/epoll.h>

#include <errno.h>
#include <string.h>

#include <list>

#define MAX_EVENTS 32
#define BUF_SIZE 1024

int set_nonblock(int fd)
{
	int flags;
#if defined(O_NONBLOCK)
	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
} 

void handle_message(int fd, std::list<int>& listeners);

int main(int argc, char **argv)
{
	int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(MasterSocket == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(12345);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int Result = bind(MasterSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr));

	if(Result == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	set_nonblock(MasterSocket);

	Result = listen(MasterSocket, SOMAXCONN);

	if(Result == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	struct epoll_event Event;
	std::list<int> listeners;
	Event.data.fd = MasterSocket;
	Event.events = EPOLLIN | EPOLLET;

	struct epoll_event * Events;
	Events = (struct epoll_event *) calloc(MAX_EVENTS, sizeof(struct epoll_event));

	if (!Events) {
		throw std::runtime_error("Can't allocate events\n");
	}
	int EPoll = epoll_create1(0);
	epoll_ctl(EPoll, EPOLL_CTL_ADD, MasterSocket, &Event);

	while(true)
	{
		int N = epoll_wait(EPoll, Events, MAX_EVENTS, -1); 
		if (N < 0) 
			throw std::runtime_error("error in epoll_wait");
		for(int i = 0; i < N; i++)
		{
			if((Events[i].events & EPOLLERR)||(Events[i].events & EPOLLHUP))
			{
				shutdown(Events[i].data.fd, SHUT_RDWR);
				close(Events[i].data.fd);
				listeners.remove(Events[i].data.fd);
			}
			else if(Events[i].data.fd == MasterSocket)
			{
				int SlaveSocket = accept(MasterSocket, 0, 0);
				set_nonblock(SlaveSocket);

				struct epoll_event Event;
				Event.data.fd = SlaveSocket;
				Event.events = EPOLLIN | EPOLLET;
				listeners.push_back(SlaveSocket);
				epoll_ctl(EPoll, EPOLL_CTL_ADD, SlaveSocket, &Event);
			}
			else
			{
				handle_message(Events[i].data.fd, listeners);
			}
		}
	}
	close(MasterSocket);
	close(EPoll);
	return 0;
}

void handle_message(int fd, std::list<int>& listeners)
{
	char buf[BUF_SIZE];
	bzero(buf, BUF_SIZE);
	int len = recv(fd, buf, BUF_SIZE, 0);
	if (len < 0) {
		throw std::runtime_error("can't read from client");
	}
	if (len == 0) {
		close(fd);
		listeners.remove(fd);
	}
	if(listeners.size() == 1) {
		const char* msg = "You are alone in this chat\n";
		send(fd, msg, strlen(msg), MSG_NOSIGNAL);
	}
   
	std::list<int>::iterator it;
	for(it = listeners.begin(); it != listeners.end(); it++){
	        if (send(*it, buf, BUF_SIZE, MSG_NOSIGNAL) < 0) 
			std::cout << "client " << *it << "was disconnected";
	}
}
