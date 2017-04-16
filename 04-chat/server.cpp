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
#include <queue>
#include <string>
#include <map>

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

struct my_data {
	int fd;
	std::queue<std::string> q;
};

void handle_message(struct epoll_event *events , int to_handle, std::list<int>& listeners);
void send_message(struct epoll_event &eventp, const char* msg, size_t len);
bool send_rest(struct epoll_event &eventp);

std::map<int, struct epoll_event> map_fd;

//inline	
//struct epoll_event* find_event(struct epoll_event* events, int fd) {
//	for (int i = 0; i < MAX_EVENTS; i++) {
//		struct my_data *data = (struct my_data *)events[i].data.ptr;
//		if (data && data->fd == fd) {
//			return events + i;
//		}
//	}
//	return NULL;
//}

int main(int argc, char **argv)
{
	int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(MasterSocket == -1) {
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(12345);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int Result = bind(MasterSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr));

	if(Result == -1) {
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	set_nonblock(MasterSocket);

	Result = listen(MasterSocket, SOMAXCONN);

	if(Result == -1) {
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	struct epoll_event Event;
	std::list<int> listeners;
	Event.data.fd = MasterSocket;
	Event.events = EPOLLIN | EPOLLET;

	struct epoll_event * Events;
	Events = (struct epoll_event *) calloc(MAX_EVENTS, 
			sizeof(struct epoll_event));

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
			if((Events[i].events & EPOLLERR)||(Events[i].events 
						& EPOLLHUP)) {
				shutdown(Events[i].data.fd, SHUT_RDWR);
				close(Events[i].data.fd);
				listeners.remove(Events[i].data.fd);
			}
			else if(Events[i].data.fd == MasterSocket) {
				int SlaveSocket = accept(MasterSocket, 0, 0);
				set_nonblock(SlaveSocket);

				struct epoll_event Event;
				struct my_data data;
				data.fd = SlaveSocket;
				Event.data.ptr = malloc(sizeof(data));
				if (!Event.data.ptr) {
					throw std::runtime_error("can't malloc in epoll_event");
				}
				memcpy(Event.data.ptr, &data, sizeof(data)); // I don't understand what another way to put there needed data can be.
				Event.events = EPOLLIN | EPOLLET;
				listeners.push_back(SlaveSocket);
				map_fd.emplace(SlaveSocket, Event);
				epoll_ctl(EPoll, EPOLL_CTL_ADD, SlaveSocket, &Event); // here is another memcpy inside into array, i guess.
				//so i have to free after it, but that is made inside epoll, i hope
				//free(Event.data.ptr);
			}
			else if (Events[i].events & EPOLLIN) {
				try {
					handle_message(Events, i, listeners);
				} catch(std::runtime_error e) {
					close(MasterSocket);
					close(EPoll);
					throw e;
					}
				}
			else if (Events[i].events & EPOLLOUT) {
				send_rest(Events[i]);
				struct my_data* data = (struct my_data*) Events[i].data.ptr;
				map_fd[data->fd] = Events[i];
			}
		}
	}
	close(MasterSocket);
	close(EPoll);
	return 0;
}

void handle_message(struct epoll_event *events, int to_handle, std::list<int>& listeners)
{
	char buf[BUF_SIZE];
	bzero(buf, BUF_SIZE);
	struct my_data* data = (struct my_data*) events[to_handle].data.ptr;
	int fd = data->fd;
	int len = recv(fd, buf, BUF_SIZE, 0);
	if (len < 0) {
		printf("%d:%s", errno, strerror(errno));
		if (errno == EAGAIN) {
			return;
		}
		throw std::runtime_error("can't read from client");
	}
	if (len == 0) {
		close(fd);
		listeners.remove(fd);
	}
	while (len > 0 && buf[len - 1] != '\n'){
		len = recv(fd, buf + len, BUF_SIZE - len, 0); 
		if (len < 0) {
			printf("%d:%s", errno, strerror(errno));
			if (errno == EAGAIN) {
				break;
			}
			throw std::runtime_error("can't read from client");
		}
	}

	if(listeners.size() == 1) {
		const char* msg = "You are alone in this chat\n";
		send(fd, msg, strlen(msg), MSG_NOSIGNAL);
		return;
	}
   
	std::list<int>::iterator it;
	for(it = listeners.begin(); it != listeners.end();){
		// I can't save the epoll_event in listeners because it's internal object of epoll
		// I need to change it and I can't get its address within creating
		// So I have to find it with O(N) run. But N is 32, constant, so not a big deal.

		struct epoll_event ev;
		try {
			ev = map_fd.at(*it);
		} catch (std::out_of_range e) {
			it = listeners.erase(it);
			continue;
		}
		send_message(ev, buf, len); 
		map_fd[*it] = ev;
		it++;
	}
}

void send_message(struct epoll_event &event, const char* msg, size_t len)
{
	struct my_data* data = (struct my_data*) event.data.ptr;
	int sent = 0;
	if (send_rest(event)) {
		sent = send(data->fd, msg, len, MSG_NOSIGNAL);
		if (sent < 0) {
			return;
		}
	} 
	if (sent < len) {
		data->q.push(std::string(msg + sent));
		event.events |= EPOLLOUT;
	}
}

bool send_rest(struct epoll_event &event)
{
	struct my_data* data = (struct my_data*) event.data.ptr;
	if (data->q.empty()) {
		return true;
	}

	std::queue<std::string>& q = data->q;
	while(!q.empty()) {
		std::string msg = q.front();
		q.pop();
		int sent = send(data->fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
		if (sent < 0) {
			return false;
		}
		if (sent < msg.size()) {
			q.push(msg.substr(sent, msg.size())); // it's better as second parameter to substr use (size - sent), but this case works too
			return false;
		}	
	}
	event.events ^= EPOLLOUT;
	return true;
}
