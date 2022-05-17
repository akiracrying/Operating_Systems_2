
#define _CRT_SECURE_NO_WARNINGS
#define CONNECTIONS_MAX 200

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>


bool stop = 0;
bool put = false;

typedef struct {
	char dd1;
	char mm1;
	u_short yyyy1;

	char dd2;
	char mm2;
	u_short yyyy2;

	char hh;
	char mm3;
	char ss;

	char msg[10000];

	int error_code;
	int msg_len;
	char num[5];
}message;

enum ERROR_CODES{
	BIND_ERROR = -2,
	LISTEN_ERROR = -3,
	PUT_ERROR,
	NO_MESSAGE
};
int set_non_block_mode(int s){
	int fl = fcntl(s, F_GETFL, 0);
	return fcntl(s, F_SETFL, fl | O_NONBLOCK);
}
int send_info(int s, unsigned int ip, unsigned short port){
	if (!put){
		char buf[3];
		recv(s, buf, 3, 0);

		if ((buf[0] == 'p') && (buf[1] == 'u') && (buf[2] == 't')){
			put = true;
		}
		else{
			printf("put receiving error\n");
			return PUT_ERROR;
		}
	}
	else {
		message income;
		char year_test[2];
		int temp;
		memset(&income.msg, 0, sizeof(income.msg));

		int recieved = recv(s, income.num, 4, 0);

		recieved = recv(s, &income.dd1, 1, 0);
		//printf("dd1 = %d\n", income.dd1);
		recieved = recv(s, &income.mm1, 1, 0);
		//printf("mm1 = %d\n", income.mm1);

		//memset(&income.yyyy1, 0, sizeof(income.yyyy1));
		recieved = recv(s, &income.yyyy1, 2, 0);
		income.yyyy1 = ntohs(income.yyyy1);
		//printf("yyyy1 = %hu\n", income.yyyy1);

		recieved = recv(s, &income.dd2, 1, 0);
		//printf("dd2 = %d\n", income.dd2);
		recieved = recv(s, &income.mm2, 1, 0);
		//printf("mm2 = %d\n", income.mm2);

		recieved = recv(s, &income.yyyy2, 2, 0);
		income.yyyy2 = ntohs(income.yyyy2);
		//printf("yyyy2 = %hu\n", income.yyyy2);


		recieved = recv(s, &income.hh, 1, 0);
		//printf("hh = %d\n", income.hh);
		recieved = recv(s, &income.mm3, 1, 0);
		//printf("mm3 = %d\n", income.mm3);
		recieved = recv(s, &income.ss, 1, 0);
		//printf("ss = %d\n", income.ss);

		recieved = recv(s, income.msg, 1, 0);
		while (income.msg[recieved - 1] != '\0' && recieved < 10000){
			recieved += recv(s, income.msg + recieved, 1, 0);
			if (recieved == 0){
				printf("no msg recieved\n");
				return NO_MESSAGE;
			}
		}
		income.msg[recieved] = '\0';


		FILE* f = fopen("msg.txt", "a+");
		fprintf(f, "%d.%d.%d.%d:%u %d.%d.%d %d.%d.%d %d:%d:%d %s\n", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF, port, income.dd1, income.mm1, income.yyyy1, income.dd2, income.mm2, income.yyyy2,income.hh, income.mm3, income.ss, income.msg	);
		fclose(f);

		if (strncmp("stop", income.msg, 4) == 0 && strlen(income.msg) == 4){
			stop = 1;
		}
		//printf("%s\n", income.msg);

		memset(&income.msg, 0, 10000);

		char ok[] = "ok";
		int ret = send(s, ok, 2, 0);
		if (ret < 0){
			printf("ok recive error");
		}else{
			//printf("ok\n");
		}
	}
}
void close_sockets(int* cc, int cnt ){
	for(int i = 0; i < cnt; i++){
		close(cc[i]);
	}
}
struct pollfd* init_pfd(struct pollfd* pfd, int ls, int cnt) {
	if (pfd == NULL) {
		pfd = (struct pollfd*)malloc(sizeof(struct pollfd));
		pfd[0].fd = ls;
		pfd[0].events = POLLIN;

		return pfd;
	}

	struct pollfd* new_pfd = (struct pollfd*)malloc((cnt + 1) * sizeof(struct pollfd));

	memcpy(new_pfd, pfd, cnt * sizeof(struct pollfd));

	new_pfd[cnt].fd = ls;
	new_pfd[cnt].events = POLLIN;

	free(pfd);
	return new_pfd;
}

int main(int argc, char *argv[]) {
	
	unsigned short port = (atoi(argv[1]));

	int i;
	int s;
	int cnt = 0;
	unsigned int ip;

	struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);


	int cc[CONNECTIONS_MAX]; 
	bool c_cl_cn[CONNECTIONS_MAX];

	s = socket(AF_INET, SOCK_STREAM, 0);
	if(set_non_block_mode(s) != 0){
		printf("block mode error");
		return BIND_ERROR;
	}
	if (bind(s, (struct sockaddr*) &addr, sizeof(addr)) < 0){
		printf("bind error\n");
		return BIND_ERROR;
	}

	if (listen(s, 1) < 0){
		printf("listen error\n");
		return LISTEN_ERROR;
	}
	printf("Listeining port %d\n", port);
	struct pollfd* pfd;
	pfd = init_pfd(NULL, s, cnt);

	while (!stop) { 
		int ev_cnt = poll(pfd, cnt + 1, 100); 
		if (ev_cnt > 0) 
		{
			for (i = 0; i < cnt; i++) 
			{
				if (pfd[i].revents & POLLHUP) 
				{
					c_cl_cn[i] = true;
					close(cc[i]);
					printf("\nPeer: %u.%u.%u.%u:%d disconnected\n", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip) & 0xFF, port);
				}
				if ((!c_cl_cn[i]) && (pfd[i].revents & POLLIN))
				{
					send_info(cc[i], ip, port);
					if (stop) {
						printf("Peer: stop message recieved\n");
						printf("Peer: %u.%u.%u.%u:%d disconnected\n", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip) & 0xFF, port);
						break;
					}
				}
			}

			if (pfd[cnt].revents & POLLIN){
				int addrlen = sizeof(addr);
				cc[cnt] = accept(s, (struct sockaddr*) &addr, (socklen_t*)&addrlen);
				ip = ntohl(addr.sin_addr.s_addr);
				put = false;
				c_cl_cn[cnt] = false;

				pfd[cnt].fd = cc[i];
				pfd[cnt].events = POLLIN | POLLOUT;

				printf("\nPeer connected: %u.%u.%u.%u\n", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip) & 0xFF);
				cnt++;
				pfd = init_pfd(pfd, s, cnt);
			}
		}
		else if (ev_cnt < 0){
			printf("event error");
		}
	}

	free(pfd);
	
	(cc, cnt);
	return EXIT_SUCCESS;
}