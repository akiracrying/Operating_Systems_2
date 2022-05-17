#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
// Директива линковщику: использовать библиотеку сокетов 
#pragma comment(lib, "ws2_32.lib")
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define u_char unsigned char 

enum ERROR_CODES {
	NO_ERRORS = 0,
	EMPTY_LINE = -1,
	STOP = -2,
	PUT_ERROR
};
typedef struct {
	char dd1[3];
	char mm1[3];
	char yyyy1[5];

	char dd2[3];
	char mm2[3];
	char yyyy2[5];

	char hh[3];
	char mm3[3];
	char ss[3];

	char* msg;

	int error_code;
	int msg_len;
	u_long num;
}message;
//dd.mm.yyyy dd.mm.yyyy hh:mm:ss Message
message msg_init(char* str, unsigned int number) {
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
		tmp_msg[ind+1] = '\0';
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

int init(){
	WSADATA wsa_data;
	return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data));
}
void deinit(){
	WSACleanup();
}
void s_close(int s) {
	closesocket(s);
}
int send_info(char arr[100][100], int s, int msgs) {
	const char p[4] = "put";
	int a  = send(s, p, 3, 0);
	if (a == 3) {
		printf("send success!\n");
	}
	else {
		printf("sending error\n");
		return PUT_ERROR;
	}
	int res;
	char buff[256];
	int stop = 0;
	int check_send = 0;
	unsigned int tmp_num = 0; // номер не пустого сообщения
	for (u_long i = 0; i < msgs; i++) {
			message str = msg_init(arr[i], tmp_num);

			if (str.error_code == EMPTY_LINE) {
				continue;
			}

			unsigned int str_num = htonl(str.num);
			char out[5] = {0};
			memcpy(out, &str_num, sizeof(int));
			send(s, out, 4, 0);   // str number

			if (str.error_code == STOP) {
				stop = 1;
			}
			//out[0] = '\0';
			char to_send;
			if (str.dd1[0] == '0') {   // first day
				to_send = (char)str.dd1[1];
				send(s, &to_send, 1, 0);
			}
			else {
				to_send = atoi(str.dd1);
				check_send = send(s, &to_send, 1, 0);
			}
			if (str.mm1[0] == '0') {		//first month
				to_send = (char)str.mm1[1];
				send(s, &to_send, 1, 0);

			}
			else {
				to_send = atoi(str.mm2);
				check_send = send(s, &to_send, 1, 0);
			}
												//first year

			//new_msg = htons((u_short)strtoul(str.yyyy1, NULL, 0));
			u_short checkme = (u_short)strtoul(str.yyyy1, NULL, 0);
			u_short yyyy_ht_one = htons(checkme);
			char yyyy_one[3] = { 0 };
			memcpy(yyyy_one, &yyyy_ht_one, sizeof(int));
			send(s, yyyy_one, 2, 0); // sending year
									 // i realise
			if (str.dd2[0] == '0') {   // first day
				to_send = (char)str.dd2[1];
				send(s, &to_send, 1, 0);
			}
			else {
				to_send = atoi(str.dd2);
				check_send = send(s, &to_send, 1, 0);
			}
			if (str.mm2[0] == '0') {		//first month
				to_send = (char)str.mm2[1];
				send(s, &to_send, 1, 0);

			}
			else {
				to_send = atoi(str.mm2);
				check_send = send(s, &to_send, 1, 0);
			}
			checkme = (u_short)strtoul(str.yyyy2, NULL, 0);
			u_short yyyy_ht_two = htons(checkme);
			char yyyy_two[3] = { 0 };
			memcpy(yyyy_two, &yyyy_ht_two, sizeof(int));
			send(s, yyyy_two, 2, 0);

			to_send = atoi(str.hh);;
			check_send = send(s, &to_send, 1, 0);
			to_send = atoi(str.mm3);;
			check_send = send(s, &to_send, 1, 0);
			to_send = atoi(str.ss);;
			check_send = send(s, &to_send, 1, 0);

			if (stop == 1) {
				check_send = send(s, "stop\0", 5, 0);
			}
			check_send = send(s, str.msg, str.msg_len + 1, 0);

			tmp_num++;

			char ok[2] = { 0 };
			int check = recv(s, ok, 2, 0);
			printf("%c%c\n", ok[0], ok[1]);
			if (check == 1) {
				recv(s, ok, 1, 0);
				//break;
			}
			if (stop == 1) {
				break;
			}
		}
	return EXIT_SUCCESS;
}
int main(int* argc, char* argv[]) {
	struct sockaddr_in addr;
	init();

	char* ip = argv[1];
	char* file_name = argv[2];

	char* port;
	port = strtok(ip, ":");
	port = strtok(NULL, ":");

	u_short u_port = (u_short)strtoul(port, NULL, 0);
	int s;

	FILE* f;
	f = fopen(file_name, "r");
	if (f == NULL) {
		printf("no such file");
		return EXIT_FAILURE;
	};
	int msgs = 0;
	char values[100][100];
	for (int i = 0; !feof(f); i++) {
		for (int j = 0; ; j++) {
			fscanf(f, "%c", &values[i][j]);
			if (values[i][j] == '\n' || feof(f))
				break;
		}
		msgs = i;
	}

	s = socket(AF_INET, SOCK_STREAM, 0);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(u_port);
	addr.sin_addr.s_addr = inet_addr(ip);

	int dying_trying = 0;
	while (dying_trying != 10) {
		if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
			printf("no connection\n");
			dying_trying++;
			Sleep(0.1);
			continue;
		}
		else {
			printf("connection established\n");
			send_info(values, s, msgs);
			break;
		}
	}

	s_close(s);
	deinit();
	return EXIT_SUCCESS;

}
