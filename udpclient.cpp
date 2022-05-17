#define _CRT_SECURE_NO_WARNINGS

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
#define u_char unsigned char 

int count = 0;
enum ERROR_CODES {
	NO_ERRORS = 0,
	EMPTY_LINE = -1,
	STOP = -2,
	PUT_ERROR = -6,
    NO_MSG = -3,
    SELECT_ERROR = 4
};
enum STAT{
    DELIVERED = 2,
    NOT_DELIVERED = 1
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
    int status;
}message;
// array, which hodls all msgs iniit
message MSG_ARRAY[100];


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


int recv_info(int s){
	char datagram[1024];
	struct timeval tv = { 0, 100 * 1000 }; 
	int res;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(s, &fds); 

	res = select(s + 1, &fds, 0, 0, &tv);
	if (res > 0){

		struct sockaddr_in addr;
		unsigned int addrlen = sizeof(addr);

		int received = recvfrom(s, datagram, sizeof(datagram), 0, (struct sockaddr*) & addr, &addrlen);

		if(received <= 0){
			printf("datagramm error\n");
			return NO_MSG;
		}
		int index;
		memcpy(&index, datagram, 4);
		index = ntohl(index);
		count++;
        return DELIVERED;
	}
	else if (res == 0){   
		return NO_MSG;  
	}  
	else{   
		return SELECT_ERROR; 
	}
}
unsigned int get_host_ipn(const char* name){
	struct addrinfo* addr = 0;
	unsigned int ip4addr = 0;

	if (0 == getaddrinfo(name, 0, 0, &addr)){
		struct addrinfo* cur = addr;
		while (cur){
			if (cur->ai_family == AF_INET){
				ip4addr = ((struct sockaddr_in*) cur->ai_addr)->sin_addr.s_addr;
				break;
			}
			cur = cur->ai_next;
		}
		freeaddrinfo(addr);
	}
	return ip4addr;
}
int send_info(int s, int index, struct sockaddr_in* addr, int message_count, int msgs){
	int sizebuf = 0;
	char* buf = (char*)calloc((33 + MSG_ARRAY[index].msg_len + 1), sizeof(char));
	int k = 0, j = 0;

	char char_num[5] = { 0 };
	int line_num = htonl(index);
	memcpy(char_num, &line_num, 4); 

	while (k != 4){
		buf[j++] = char_num[k];
		k++;
	}
	k = 0;
	//
	buf[j++] = atoi(MSG_ARRAY[index].dd1);
	buf[j++] = atoi(MSG_ARRAY[index].mm1);

	u_short checkme = (u_short)strtoul(MSG_ARRAY[index].yyyy1, NULL, 0);
	u_short yyyy_ht_one = htons(checkme);
	char yyyy_one[3] = { 0 };
	memcpy(yyyy_one, &yyyy_ht_one, sizeof(int));

	buf[j++] = yyyy_one[0];
	buf[j++] = yyyy_one[1];

	//

	buf[j++] = atoi(MSG_ARRAY[index].dd2);
	buf[j++] = atoi(MSG_ARRAY[index].mm2);

	checkme = (u_short)strtoul(MSG_ARRAY[index].yyyy2, NULL, 0);
	u_short yyyy_ht_two = htons(checkme);
	char yyyy_two[3] = { 0 };
	memcpy(yyyy_two, &yyyy_ht_two, sizeof(int));

	buf[j++] = yyyy_two[0];
	buf[j++] = yyyy_two[1];

	k = 0;
	//
	buf[j++] = atoi(MSG_ARRAY[index].hh);
	buf[j++] = atoi(MSG_ARRAY[index].mm3);
	buf[j++] = atoi(MSG_ARRAY[index].ss);

	while( k < MSG_ARRAY[index].msg_len){
		buf[j++] = MSG_ARRAY[index].msg[k];
		k++;
	}
	buf[j] = '\0';
	sizebuf = j+1;
	sendto(s, buf, sizebuf, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	//
	free(buf);

	int rec_check = recv_info(s);
	if(rec_check == DELIVERED){
		MSG_ARRAY[index].status = DELIVERED;
	}else{
		MSG_ARRAY[index].status = NOT_DELIVERED;
	}
}
int main(int argc, char* argv[]){
	struct sockaddr_in addr;

	char* ip = argv[1];
	char* file_name = argv[2];

	char* port;
	port = strtok(ip, ":");
	port = strtok(NULL, ":");
	//printf("Hello\n");

	u_short u_port = (u_short)strtoul(port, NULL, 0);


    int s = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(u_port);
        addr.sin_addr.s_addr = get_host_ipn(ip);
    
	
	FILE* f;
	f = fopen(file_name, "r");
	if (f == NULL) {
		printf("no such file\n");
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
	//printf("msgs = %d\n", msgs);
    int message_count = 0;
	for (u_long i = 0; i < msgs; i++) {
        message str = msg_init(values[i], message_count);

        if (str.error_code == EMPTY_LINE) {
            continue;
        }
		MSG_ARRAY[message_count] = str;
        message_count++;
    }
	printf("msg count = %d\n", message_count);
    while(count != message_count){

		for(int i=0; i < message_count;i++){
			if(MSG_ARRAY[i].status != DELIVERED){
        		send_info(s, i, &addr, message_count, msgs);
			}
		}
        if (count >= 20 || (count == message_count && message_count < 20)){
			break;
		}
    }

    close(s);

    return EXIT_SUCCESS;
}