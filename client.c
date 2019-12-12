#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<errno.h>
#include <ctype.h>

#define MESSAGE_BUFF 512
#define USERNAME_BUFF 20

typedef struct{
	char* prompt;
	int socket;
}thread_data;

int board[9];

void info0(){
	printf("Information missed\n");
	exit(1);
}

void info1(){
	printf("\nFunction Intro:\n");
	printf("/q or /quit: quit\n");
	printf("/look : look other online users\n");
	printf("/chat : chat to specific user\n");
	printf("/play : play ox game\n");
}

void print_board(){
	int i;
	char mark[9];

	for(i = 0; i < 9 ;i++){
		if(board[i] == -1)
			mark[i] = ' ';
		else if(board[i] == 0){
			mark[i] = 'O';
		}
		else
			mark[i] = 'X';
	}

	printf("\n");
	
    printf(" 0 │ 1 │ 2          %c │ %c │ %c \n", mark[0], mark[1], mark[2]);
    printf("───┼───┼───        ───┼───┼───\n");
    printf(" 3 │ 4 │ 5          %c │ %c │ %c \n", mark[3], mark[4], mark[5]);
    printf("───┼───┼───        ───┼───┼───\n");
    printf(" 6 │ 7 │ 8          %c │ %c │ %c \n\n", mark[6], mark[7], mark[8]);

}


void *server_connection(int socket_fd, struct sockaddr_in *address){
	int response = connect(socket_fd, (struct sockaddr *) address, sizeof *address);
	if(response < 0){
		fprintf(stderr, "connect() failed: %s\n", strerror(errno));
		exit(1);
	}else{
		printf("Server connected\n");
	}
}

void *receive(void *data){
	int game_mode, board_ctr;
	int socket_fd, response;
	char message[MESSAGE_BUFF];
	thread_data *data_ptr = (thread_data *) data;
	socket_fd = data_ptr -> socket;
	char *prompt = data_ptr -> prompt;
	game_mode = 0;
	board_ctr = 0;

	// 接收訊息
	while(1){
		if(board_ctr == 0){
			memset(board, -1, sizeof(board));
		}
		memset(message, '\0', sizeof(message));
		fflush(stdout);

		response = recv(socket_fd, message, MESSAGE_BUFF, 0);
		if(response == -1){
			fprintf(stderr, "recv() failed: %s\n", strerror(errno));
			break;
		}
		else if(response == 0){
			printf("\nPeer discoonected\n");
			break;
		}
		else{
			if(game_mode){
				if(strncmp("<", message, 1) == 0){
					printf("%s", message);
					print_board();
					if(strncmp("<TURN>", message, 6) == 0)
						printf("<GAME> Please insert a number between 0~8\n>");
				}
				else{
		sscanf(message, "%d %d %d %d %d %d %d %d %d", &board[0], &board[1], &board[2], &board[3], &board[4], &board[5], &board[6], &board[7], &board[8]);
				}
				// 遊戲結束
				if(strncmp("<GAME>", message, 6) == 0){
					game_mode = 0;
					board_ctr = 0;
					printf("%s", prompt);
				}
			}
			else{
				printf("%s", message);
				if(strncmp("<GAME>", message, 6) == 0){
					game_mode = 1;
					board_ctr = 1;
				}
				else if(strncmp(">>", message, 2) != 0){	//私人訊息
					printf("%s", prompt);
				}
			}
		}
	}
}

void send_msg(char *prompt, int socket_fd, struct sockaddr_in *address, char* user_name){
	char message[MESSAGE_BUFF];
	char buff[MESSAGE_BUFF];
	memset(message, '\0', sizeof(message));
	memset(buff, '\0', sizeof(buff));

	send(socket_fd, user_name, strlen(user_name), 0);
	
	while(fgets(message, MESSAGE_BUFF, stdin)!=NULL){
		printf("%s",prompt);
		if((strncmp(message, "/quit", 5) ==0) || (strncmp(message, "/q", 2) == 0)){
			printf("Close connection...\n");
			exit(0);
		}
		else if(strncmp(message, "/help", 5) == 0){
			info1();
			printf("%s", prompt);
			memset(message, '\0', sizeof(message));	
			continue;
		}
		send(socket_fd, message, strlen(message), 0);
		memset(message, '\0', sizeof(message));	
  	}
}

int main(int argc, char **argv){
	long port = strtol(argv[2], NULL, 10);
	struct sockaddr_in address, client_addr;
	char *server_addr;
	int socket_fd, response;
	char prompt[USERNAME_BUFF + 4];
	char user_name[USERNAME_BUFF];
	pthread_t thread;

	if(argc < 3){
		info0();
	}
	
	printf("Enter your name: ");
	scanf("%s", user_name);
	strcpy(prompt, user_name);
	strcat(prompt, ">");

	server_addr = argv[1];
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(server_addr);
	address.sin_port = htons(port);
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	server_connection(socket_fd, &address);

	thread_data data;
	data.prompt = strdup(prompt);	// message sending
	data.socket = socket_fd;

	pthread_create(&thread, NULL, (void *)receive, (void *)&data);

	send_msg(prompt, socket_fd, &address, user_name);

	printf("closed\n");
	close(socket_fd);
	pthread_exit(NULL);
	return 0;
}