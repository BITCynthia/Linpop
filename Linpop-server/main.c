/******************************************
*                     main.c              *
*                                         *
*******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <glib.h>
#include "Linpop_Server.h"

GArray *array_sock = NULL; // 已经连接 sock 数组


// 用户列表
s_user_info user_list[19] =
{
{ PARENT_ITEM, "07111501", "一班", 0, 1000, 1 },
{ CHILD_ITEM, "1120151709", "洁洁", 0, 1, 0 },
{ CHILD_ITEM, "1120151729", "萱萱", 0, 2, 0 },
{ PARENT_ITEM, "07111502", "二班", 0, 2000, 1 },
{ CHILD_ITEM, "1120151754", "菲菲", 0, 3, 0 },
{ CHILD_ITEM, "1120151751", "王景", 0, 4, 0 },
{ CHILD_ITEM, "1120151745", "翰翰", 0, 5, 0 },
{ CHILD_ITEM, "1120151755", "杨奎", 0, 6, 0 },
{ PARENT_ITEM, "07111503", "三班", 0, 3000, 1 },
{ CHILD_ITEM, "1120151765", "文文", 0, 7, 0 },
{ CHILD_ITEM, "1120151769", "凡凡", 0, 8, 0 },
{ CHILD_ITEM, "1320160216", "童童", 0, 9, 0 },
{ CHILD_ITEM, "1120151764", "乔乔", 0, 10, 0 },
{ CHILD_ITEM, "1120151775", "小花", 0, 11, 0 },
{ CHILD_ITEM, "1120151782", "紫薇", 0, 12, 0 },
{ PARENT_ITEM, "07111504", "四班", 0, 4000, 1 },
{ CHILD_ITEM, "1120151796", "畅畅", 0, 13, 0 },
{ CHILD_ITEM, "1120151805", "旭东", 0, 14, 0 },
{ CHILD_ITEM, "1120151807", "志伟", 0, 15, 0 }, 
};

/* main function */
int main(int argc, char* argv[])
{
	struct sockaddr_in sAddr;
	int listensock;
	int result;
	pthread_t thread_id[THREAD_COUNT];
	int x;
	int val;

	array_sock = g_array_new(FALSE, TRUE, sizeof(sock_item));
	listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// set the SO_REUSEADDR
	val = 1;
	result
			= setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &val,
					sizeof(val));
	if (result < 0)
	{
		perror("server5");
		return 0;
	}
	// bind to a local port
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(SERVER_PORT);
	sAddr.sin_addr.s_addr = INADDR_ANY;

	result = bind(listensock, (struct sockaddr*) &sAddr, sizeof(sAddr));
	if (result < 0)
	{
		perror("server5");
		return 0;
	}

	// put it into listening mode
	result = listen(listensock, 5);
	if (result < 0)
	{
		perror("server5");
		return 0;
	}

	// create our pool of threads
	for (x = 0; x < THREAD_COUNT; ++x)
	{
		result = pthread_create(&thread_id[x], NULL, thread_proc,
				(void *) listensock);
		if (result != 0)
			printf("Could not create thread.\n");
		sched_yield();
	}
	for (x = 0; x < THREAD_COUNT; ++x)
	{
		pthread_join(thread_id[x], NULL);
	}
}

void* thread_proc(void *arg)
{
	int listensock, sock;
	char buffer[MAX_BUF + 1];
	int nread;

	char id_buf[15];
	int i;

	listensock = (int) arg;
	while (1)
	{
		sock = accept(listensock, NULL, NULL);
		printf("client connected to child thread %lu with pid %i.\n",
				pthread_self(), getpid());

		while (1)
		{
			nread = recv(sock, buffer, MAX_BUF, 0);
			if (nread <= 0)
			{
				printf("server log:客户端%d退出!\n", sock);

				memset(id_buf, 0, sizeof(id_buf));
				for (i = 0; i < array_sock->len; ++i)
				{
					sock_item item = g_array_index(array_sock, sock_item, i);
					if (sock == item.new_fd)
					{
						strcpy(id_buf, item.id);

						// 及时删除下线用户 socket
						g_array_remove_index(array_sock, i);
						break;
					}
				}

				// 刷新在线用户列表
				// 查找要删除的 id
				for (i = 0; i < sizeof(user_list) / sizeof(s_user_info); i++)
					if (strcmp(user_list[i].id, id_buf) == 0)
						break;

				if (i < sizeof(user_list) / sizeof(s_user_info))
				{
					user_list[i].online = 0;
					user_list[i].ip_addr.s_addr = 0;

					// 开启线程, 发送当前上线用户
					int thread_ret = pthread_create(&thread_online,
							&thread_attr, &send_online_list, NULL);
					if (thread_ret != 0)
					{
						perror("server log:thread create error");
						exit(EXIT_FAILURE);
					}
				}
				printf(" Be about to close sock!\n");
				close(sock);
				printf(
						"client disconnected from child thread %lu  with pid %i.\n",
						pthread_self(), getpid());
				break;
			}
			else
			{
				// Message Parsing 
				buffer[nread] = '\0';
				message_parser(buffer, sock, nread);
			}
		}
	}
}
