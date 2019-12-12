#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<pthread.h>
#include<time.h>

#define MAXLINE 512
#define MAX_MEM 10
#define NAME_LEN 20
#define SERV_PORT 8080
#define LISTENQ 5
#define MAX_GAME 5

int listen_fd,connect_fd[MAX_MEM];
int game_state[MAX_MEM];
int board[MAX_MEM][9];
char user[MAX_MEM][NAME_LEN];

void server(){
	char msg[10];
	while(1){
		scanf("%s", msg);
		if(strcmp(msg, "/quit") == 0 || strcmp(msg, "/Q") == 0)
		{	
			printf("CLOSING...........\n");
			close(listen_fd);
			exit(0);
		}
}
}

void game(int p1,int p2){
	int i, len, move;
	char msg_send[MAXLINE];
	char msg_rcv[MAXLINE];

	char game0[]="<WAIT> Waiting........\n";
	char game1[]="<TURN> It's your turn\n";
	char game2[]="<GAME> Loser\n";
	char game3[]="<GAME> winer\n";
	char game4[]="<GAME> Draw\n";

	memset(board[p1], -1, sizeof(board[p1]));
	memset(board[p2], -1, sizeof(board[p2]));

	for(i = 0; i < 5; i++){
		memset(msg_rcv, '\0', sizeof(msg_rcv));
		memset(msg_send, '\0', sizeof(msg_send));

		send(connect_fd[p1], game1, strlen(game1), 0);
		send(connect_fd[p2], game0, strlen(game0), 0);

		len = recv(connect_fd[p1], msg_rcv, MAXLINE, 0);
		move =atoi(msg_rcv);
		board[p1][move] = 0;
		board[p2][move] = 1;
		
		sprintf(msg_send, "%d %d %d %d %d %d %d %d %d", board[p1][0], board[p1][1], board[p1][2], board[p1][3], board[p1][4], board[p1][5], board[p1][6], board[p1][7], board[p1][8]);
		send(connect_fd[p1], msg_send, MAXLINE, 0);
		sprintf(msg_send, "%d %d %d %d %d %d %d %d %d", board[p2][0], board[p2][1], board[p2][2], board[p2][3], board[p2][4], board[p2][5], board[p2][6], board[p2][7], board[p2][8]);
		send(connect_fd[p2], msg_send, MAXLINE, 0);

		if(game_ckeck(p1)){
			send(connect_fd[p1], game3, strlen(game3), 0);
			send(connect_fd[p2], game2, strlen(game2), 0);
			break;
		}
		
		if(i == 4){
			send(connect_fd[p1], game4, strlen(game4), 0);
			send(connect_fd[p2], game4, strlen(game4), 0);
			break;
		}
		// 交換玩家
		send(connect_fd[p1], game0, strlen(game0), 0);
		send(connect_fd[p2], game1, strlen(game1), 0);

		len = recv(connect_fd[p2], msg_rcv, MAXLINE, 0);
		move =atoi(msg_rcv);
		board[p1][move] = 1;
		board[p2][move] = 0;
		sprintf(msg_send, "%d %d %d %d %d %d %d %d %d", board[p1][0], board[p1][1], board[p1][2], board[p1][3], board[p1][4], board[p1][5], board[p1][6], board[p1][7], board[p1][8]);
		send(connect_fd[p1], msg_send, MAXLINE, 0);
		sprintf(msg_send, "%d %d %d %d %d %d %d %d %d", board[p2][0], board[p2][1], board[p2][2], board[p2][3], board[p2][4], board[p2][5], board[p2][6], board[p2][7], board[p2][8]);
		send(connect_fd[p2], msg_send, MAXLINE, 0);
		if(game_ckeck(p2)){
			send(connect_fd[p1], game2, strlen(game2), 0);
			send(connect_fd[p2], game3, strlen(game3), 0);
			break;
		}
		send(connect_fd[p1], msg_send, MAXLINE, 0);
	}

	game_state[p1] = -1;
	game_state[p2] = -1;
	printf("END..........\n");
}
int game_ckeck(int idx){
	int i;

	for(i = 0; i < 7 && board[idx][i] == -1; i++);	// 檢查操作
	if(board[idx][i] != -1){
		switch(i){
			case 0:
				if(board[idx][i] == board[idx][i+1] && board[idx][i] == board[idx][i+2])
					return 1;
				else if(board[idx][i] == board[idx][i+3] && board[idx][i] == board[idx][i+6])	
					return 1;
				else if(board[idx][i] == board[idx][i+4] && board[idx][i] == board[idx][i+8])
					return 1;
				break;
			case 1:
				if(board[idx][i] == board[idx][i+3] && board[idx][i] == board[idx][i+6])
					return 1;
				break;
			case 2:
				if(board[idx][i] == board[idx][i+3] && board[idx][i] == board[idx][i+6])
					return 1;
				else if(board[idx][i] == board[idx][i+2] && board[idx][i] == board[idx][i+4])
					return 1;
				break;
			case 3:
				if(board[idx][i] == board[idx][i+1] && board[idx][i] == board[idx][i+2])
					return 1;
				break;
			case 6:
				if(board[idx][i] == board[idx][i+1] && board[idx][i] == board[idx][i+2])
					return 1;
		}
	}
	return 0;
}

void receive_send(int n){
	char msg_notify[MAXLINE];
	char msg_rcv[MAXLINE];
	char msg_send[MAXLINE];
	char target_user[MAXLINE];
	char user_name[NAME_LEN];
	char message[MAXLINE];

	char msg1[]=" <SERVER> Who do you want to chat with? ";
	char msg2[]=" <SERVER> Which player do you want to play with: ";
	char msg3[]=" <SERVER> Accept the game <Y or y>";
	char msg4[]=" <SERVER> Be rejected\n";
	char msg5[]=" <SERVER> Game start\n";
	char game1[]="<GAME> game created!\n";
	
	int i = 0;
	int target_idx;
	int retval;
	memset(user_name, '\0', sizeof(user_name));

	int length = recv(connect_fd[n], user_name, NAME_LEN, 0);
	if(length > 0){	
		user_name[length-1] = '\0';	
		strcpy(user[n], user_name);
		printf("User name: %s\n",user[n]);
	}

	// receiving message
	while(1){
		memset(msg_rcv, '\0', sizeof(msg_rcv));
		memset(msg_send, '\0', sizeof(msg_send));
		memset(message,'\0',sizeof(message));
		target_idx = -1;

		length = recv(connect_fd[n], msg_rcv, MAXLINE, 0);
		if(length > 0){	//Message received
			msg_rcv[length] = 0;

			// 離開
			if(strcmp(msg_rcv, "/quit") == 0 || strcmp(msg_rcv, "/q") == 0){
				printf("%s quitted\n", user_name);
				close(connect_fd[n]);
				connect_fd[n] = -1;
				pthread_exit(&retval);
			}
			// 查找
			else if(strncmp(msg_rcv, "/look", 5) == 0){
				strcpy(msg_send, "\n<SERVER> Online:\n");
				for(i = 0; i < MAX_MEM; i++){
					if(connect_fd[i] != -1){
						strcat(msg_send, user[i]);
						strcat(msg_send, "\n");
					}
				}
				send(connect_fd[n], msg_send, strlen(msg_send), 0);
			}
			// 聊天
			else if(strncmp(msg_rcv, "/chat", 5) == 0){
				printf("\nprivate message from %s ...\n", user_name);
				// target
				send(connect_fd[n], msg1, strlen(msg1), 0);
				length = recv(connect_fd[n], target_user, MAXLINE, 0);
				target_user[length - 1] = '\0';	// '\n' to '\0'
				sprintf(msg_send, ">>%s : ", target_user);
				send(connect_fd[n], msg_send, strlen(msg_send), 0);

				// 私人訊息
				length = recv(connect_fd[n], message, MAXLINE, 0);
				message[length] = '\0';
				sprintf(msg_send, " <PRIVATE> %s : %s", user_name, message);

				//寄出私人訊息
				for(i = 0; i < MAX_MEM; i++){
					if(connect_fd[i] != -1 && strncmp(target_user, user[i], strlen(target_user)) == 0)
						send(connect_fd[i], msg_send, strlen(msg_send), 0);
				}
			}
			// 遊戲
			else if(strncmp(msg_rcv, "/play", 5) == 0){
				memset(game_state, -1, sizeof(game_state));
				printf("\n %s PLAY a game with ", user_name);
				
				//  target
				send(connect_fd[n], msg2, strlen(msg2), 0);
				length = recv(connect_fd[n], target_user, MAXLINE, 0);
				target_user[length - 1] = '\0';
				printf("%s...\n", target_user);
				
				//等待同意
				sprintf(msg_send, "%s %s?\n", msg3, user_name);
				for(i = 0; i < MAX_MEM; i++){
					if(connect_fd[i] != -1 && strncmp(target_user, user[i], strlen(target_user)) == 0){
						if(game_state[n] == -1 && game_state[i] == -1){
							printf("Founding...\n");
							send(connect_fd[i], msg_send, strlen(msg_send), 0);
							target_idx = i;
							game_state[n] = target_idx;
							break;
						}
						else{
							printf("Playing now...\n");
							break;
						}
					}
				}
				if(game_state[n] == -1){
					printf("Game Failed\n");
					continue;
				}
				// 回覆
				length = recv(connect_fd[target_idx], message, MAXLINE, 0);	
				message[length] = '\0';	

				if(strncmp(message, "y", 1) != 0 && strncmp(message, "Y", 1) != 0){
					printf("Player %s reject...\n", target_user);
					game_state[n] = -1;
					send(connect_fd[n], msg4, strlen(msg4), 0);
					continue;
				}
				printf("Game start ... %s vs %s\n", user_name, target_user);
				send(connect_fd[n], game1, strlen(game1), 0);
				send(connect_fd[target_idx], game1, strlen(game1), 0);
				game_state[n] = target_idx;
				game_state[target_idx] = n;
				game(n, target_idx);
				printf("%s vs %s GAME END ...\n", user_name, target_user);
			}
			// 聊天
			else{
				strcpy(msg_send, user_name);
				strcat(msg_send,": ");
				strcat(msg_send, msg_rcv);

				for(i = 0; i < MAX_MEM; i++){
					if(connect_fd[i] != -1 && strcmp(user_name, user[i]) != 0){	// client exist
						send(connect_fd[i], msg_send, strlen(msg_send), 0);
					}
				}
			}

		}
	}
}

int main(){
	printf("Available number of user is %d\n", MAX_MEM);
	printf("Maximum user name is %d\n", NAME_LEN);
	printf("Server port is set to be: %d\n", SERV_PORT);
	printf("Maximum meg is %d\n\n", MAXLINE);
	printf("Function Intro:\n");
	printf("/q, /quit :quit the server\n");
	int i;
	pthread_t thread;
	struct sockaddr_in server_addr, client_addr;
	socklen_t sock_len;	
	char buff[MAXLINE];

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0){
		printf("Socket Failed\n");
		return -1;
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


	if(bind(listen_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0 ){
		printf("Bind Failed\n");
		return -1;
	}
	printf("Listening\n");
	listen(listen_fd, LISTENQ);

	pthread_create(&thread, NULL, (void*)(&server), NULL);

	for(i = 0; i < MAX_MEM; i++){
		connect_fd[i] = -1;
	}
	memset(user, '\0', sizeof(user));
	printf("Init\n");

	//等待客戶端
	while(1){
		sock_len = sizeof(client_addr);
		for(i = 0; (i < MAX_MEM) && (connect_fd[i] != -1); i++);	
		connect_fd[i] = accept(listen_fd, (struct sockaddr*)&client_addr, &sock_len);
		
		printf("\nWaiting for user name...\n");
		pthread_create(malloc(sizeof(pthread_t)), NULL, (void*)(&receive_send), (void *)i);
	}

	return 0;
}
