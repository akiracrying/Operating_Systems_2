#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define MAX 120
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
enum ERROR_CODES {
	NO_ERRORS = 0,
	EMPTY_LINE = -1,
	STOP = -2,
	PUT_ERROR = -6,
	NO_MSG = -3,
	SELECT_ERROR = 4,
	BIND_ERROR = -8
};
enum STAT {
	DELIVERED = 2,
	NOT_DELIVERED = 1
};
int num_of_msg = 0;
bool stop = false;

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

	char* msg;

	int error_code;
	int msg_len;
	u_long num;
	int status;
	char* ip;
	int port;
}message;
/*message msg_init(char* str, unsigned int number) {
	message cur_msg;
	if (str[0] == '\n') {
		cur_msg.error_code = EMPTY_LINE;
		return cur_msg;
	}

	cur_msg.dd1[0] = str[0];
	cur_msg.dd1[1] = str[1];
	cur_msg.dd1[2] = '\0';


	cur_msg.mm1[0] = str[3];
	cur_msg.mm1[1] = str[4];
	cur_msg.mm1[2] = '\0';

	cur_msg.yyyy1[0] = str[6];
	cur_msg.yyyy1[1] = str[7];
	cur_msg.yyyy1[2] = str[8];
	cur_msg.yyyy1[3] = str[9];
	cur_msg.yyyy1[4] = '\0';

	cur_msg.dd2[0] = str[11];
	cur_msg.dd2[1] = str[12];
	cur_msg.dd2[2] = '\0';

	cur_msg.mm2[0] = str[14];
	cur_msg.mm2[1] = str[15];
	cur_msg.mm2[2] = '\0';

	cur_msg.yyyy2[0] = str[17];
	cur_msg.yyyy2[1] = str[18];
	cur_msg.yyyy2[2] = str[19];
	cur_msg.yyyy2[3] = str[20];
	cur_msg.yyyy2[4] = '\0';

	cur_msg.hh[0] = str[22];
	cur_msg.hh[1] = str[23];
	cur_msg.hh[2] = '\0';

	cur_msg.mm3[0] = str[25];
	cur_msg.mm3[1] = str[26];
	cur_msg.mm3[2] = '\0';

	cur_msg.ss[0] = str[28];
	cur_msg.ss[1] = str[29];
	cur_msg.ss[2] = '\0';

	char* tmp_msg;
	int len = 0;
	for (int j = 31; str[j] != '\n'; j++) {
		len++;
	}
	tmp_msg = (char*)malloc(sizeof(char) * len + 1);
	tmp_msg[0] = '\0';
	int ind = 0;
	for (int j = 31; j < 31 + len; j++) {
		tmp_msg[ind] = str[j];
		tmp_msg[ind + 1] = '\0';
		ind++;
	}

	cur_msg.msg = tmp_msg;
	cur_msg.msg[len] = '\0';
	if (strcmp(tmp_msg, "stop") == 0) {
		cur_msg.error_code = STOP;
		//return cur_msg;
	}
	else {
		cur_msg.error_code = NO_ERRORS;
	}
	cur_msg.msg_len = len;
	cur_msg.num = number;
	return cur_msg;
}
*/
// array, which hodls all msgs iniit
message MSG_ARRAY[100];
int init() {
	WSADATA wsa_data;
	return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data));
}
void deinit() {
	WSACleanup();
}
void s_close(int s) {
	closesocket(s);
}
int set_non_block_mode(int s){
	unsigned long mode = 1;
	return ioctlsocket(s, FIONBIO, &mode);
}
int recv_info(int port, char* ip, char *buf, int size) {
	int idx;
	char buf_msg[4];
	for (int j = 0; j < 4; j++)
	{
		buf_msg[j] = buf[j];
	}
	memcpy(&idx, buf_msg, 4);
	idx = ntohl(idx);
	int cur_msg = num_of_msg;
	num_of_msg++;
	MSG_ARRAY[cur_msg].port = port;

	MSG_ARRAY[cur_msg].ip = (char*)malloc(strlen(ip) * sizeof(char));
	strcpy(MSG_ARRAY[cur_msg].ip, ip);

	MSG_ARRAY[cur_msg].dd1 = buf[4];
	MSG_ARRAY[cur_msg].mm1 = buf[5];

	char tmp[3] = { 0 };
	tmp[0] = buf[6];
	tmp[1] = buf[7];
	u_short tmp_year;
	memcpy(&tmp_year, tmp, sizeof(u_short));
	tmp_year = ntohs(tmp_year);
	MSG_ARRAY[cur_msg].yyyy1 = tmp_year;

	MSG_ARRAY[cur_msg].dd2 = buf[8];

	MSG_ARRAY[cur_msg].mm2 = buf[9];

	tmp[0] = buf[10];
	tmp[1] = buf[11];
	memcpy(&tmp_year, tmp, sizeof(u_short));
	tmp_year = ntohs(tmp_year);
	MSG_ARRAY[cur_msg].yyyy2 = tmp_year;

	MSG_ARRAY[cur_msg].hh = buf[12];
	MSG_ARRAY[cur_msg].mm3 = buf[13];
	MSG_ARRAY[cur_msg].ss = buf[14];


	MSG_ARRAY[cur_msg].msg = (char*)malloc((size - 15) * sizeof(char));

	int i = 0;
	while (i!= size - 16) {
		MSG_ARRAY[cur_msg].msg[i] = buf[15 + i];
		i++;
	}
	MSG_ARRAY[cur_msg].msg[size - 16] = '\0';
	if (strcmp("stop", MSG_ARRAY[cur_msg].msg) == 0)
		stop = true;

	return idx;

}

int main(int* argc, char* argv[]) {
	struct sockaddr_in addr;
	init();

	int port_1 = atoi(argv[1]);
	int port_2 = atoi(argv[2]);
	int port = port_2 - port_1;
	int port_arr[MAX], sock_arr[MAX];

	for (int i = 0; i <= port; i++){
		sock_arr[i] = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock_arr[i] < 0) {
			printf("socket error\n");
			return BIND_ERROR;
		}


		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		port_arr[i] = port_1 + i;
		addr.sin_port = htons(port_arr[i]);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(sock_arr[i], (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			printf("bind error\n");
			return BIND_ERROR;
		}
		set_non_block_mode(sock_arr[i]);
	}

	fd_set rfd;
	fd_set wfd;
	int nfds = sock_arr[0];
	struct timeval tv = { 10, 10 };
	while (1){
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		socklen_t addrlen = sizeof(addr);
		char buffer[10000] = { 0 };
		for (int i = 0; i <= port; i++)
		{
			FD_SET(sock_arr[i], &rfd);
			FD_SET(sock_arr[i], &wfd);
			if (nfds < sock_arr[i])
				nfds = sock_arr[i];
		}
		if (select(nfds + 1, &rfd, &wfd, 0, &tv) > 0)
		{
			for (int i = 0; i <= port; i++)
			{
				if (FD_ISSET(sock_arr[i], &rfd))
				{
					printf("message arrived\n");
					int recv = recvfrom(sock_arr[i], buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addrlen);
					if (recv > 0)
					{
						char* ip = inet_ntoa(addr.sin_addr); 
						int answer = recv_info(port_arr[i], ip, buffer, recv);

						char buf_recv[4] = { 0 };
						answer = htonl(answer);
						memcpy(buf_recv, &answer, 4);
						int send_len = sendto(sock_arr[i], buf_recv, 4, 0, (struct sockaddr*)&addr, addrlen);
						if (stop == true)
							break;
					}
				}
			}
		}
		if (stop == true) {
			printf("stop received\n");
			break;
		}
	}
	FILE* f = fopen("msg.txt", "a+");

	for (int i = 0; i < num_of_msg; i++){

		fprintf(f, "%s:%u %d.%d.%d %d.%d.%d %d:%d:%d %s\n", MSG_ARRAY[i].ip, MSG_ARRAY[i].port, MSG_ARRAY[i].dd1, MSG_ARRAY[i].mm1, MSG_ARRAY[i].yyyy1, MSG_ARRAY[i].dd2, MSG_ARRAY[i].mm2, MSG_ARRAY[i].yyyy2, MSG_ARRAY[i].hh, MSG_ARRAY[i].mm3, MSG_ARRAY[i].ss, MSG_ARRAY[i].msg);
	}

	for (int i = 0; i <= port; i++)
		closesocket(sock_arr[i]);

	fclose(f);
	deinit();

	return EXIT_SUCCESS;
}

