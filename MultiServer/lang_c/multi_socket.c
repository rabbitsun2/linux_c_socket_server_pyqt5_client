/*

	Subject: Multi Thread, Multiple Client - Server, Text Parser
	Create Date: 2022-04-22
	Author: Doyun Jung(정도윤)
	License: Apache License v2.0
	Description:

*/

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

	
#define TRUE 1
#define FALSE 0
#define PORT 8888

const int CHOOSE_EXIT = 0;
const int CHOOSE_CONFIG_PORT = 1;
const int CHOOSE_ACTIVE = 2;

struct Computer
{
	char address[128];
	int port;
};

struct Datainfo
{
	char* str_type;
	char* str_value;

};

struct Datainfo *getParser(char* data);
int server_program(struct Computer com);
struct Computer com_init();

int login_checker(struct Datainfo *data);

int main(int argc , char *argv[])
{
	struct Computer server = com_init();
	int choose = 1;

	while (choose)
	{
		printf("메뉴를 선택하세요:");
		scanf("%d", &choose);

		if ( choose == CHOOSE_EXIT ){
			printf("프로그램을 종료합니다.");
		}
		else if ( choose == CHOOSE_CONFIG_PORT ){
			int port = 0;
			printf("포트 번호를 입력하세요:");
			scanf("%d", &port);
			server.port = port;

		}
		else if ( choose == CHOOSE_ACTIVE ){
			server_program(server);
		}

	}
	
	return 0;
}

struct Computer com_init(){

	struct Computer com;
	strcpy(com.address, "");
	com.port = 8888;

	return com;

}

int server_program(struct Computer com){

	int opt = TRUE;
	int master_socket , addrlen , new_socket , client_socket[30] ,
		max_clients = 30 , activity, i , valread , sd;
	int max_sd;

	int usrPort = com.port;

	struct sockaddr_in address;
		
	char buffer[1025]; //data buffer of 1K
		
	//set of socket descriptors
	fd_set readfds;
		
	//a message
	char *message = "메아리 Daemon v1.0 \r\n";
	//char message[] = "메아리 Daemon v1.1 \r\n";
	
	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}
		
	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	
	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( usrPort );
		
	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf( "Listener on port %d \n", usrPort );
		
	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
		
	//accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");
		
	while(TRUE)
	{
		//clear the socket set
		FD_ZERO(&readfds);
	
		//add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
			
		//add child sockets to set
		for ( i = 0 ; i < max_clients ; i++)
		{
			//socket descriptor
			sd = client_socket[i];
				
			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);
				
			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
		}
	
		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
	
		if ((activity < 0) && (errno!=EINTR))
		{
			printf("select error");
		}
			
		//If something happened on the master socket ,
		//then its an incoming connection
		if (FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket,
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			
			//inform user of socket number - used in send and receive commands
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n",
                 new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
		
			//send new connection greeting message
			if( send(new_socket, message, strlen(message), 0) != strlen(message) )
			{
				perror("send");
			}

            puts("Welcome message sent successfully");

            // Received client Text.
			valread = read(new_socket, buffer, 1025);
    		printf("%s\n", buffer);
			
            struct Datainfo *pInfo = getParser(buffer);
			
			if (pInfo != NULL){
				printf("zzzzzz %s %s", pInfo->str_type, pInfo->str_value);
				//printf(" %d", login_checker(pInfo));
				login_checker(pInfo);
			}

			puts("Welcome message read successfully");
				
			//add new socket to array of sockets
			for (i = 0; i < max_clients; i++)
			{
				//if position is empty
				if( client_socket[i] == 0 )
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n" , i);
						
					break;
				}
			}
		}
			
		//else its some IO operation on some other socket
		for (i = 0; i < max_clients; i++)
		{
			sd = client_socket[i];
				
			if (FD_ISSET( sd , &readfds))
			{
				//Check if it was for closing , and also read the
				//incoming message
				if ((valread = read( sd , buffer, 1024)) == 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(sd , (struct sockaddr*)&address , \
						(socklen_t*)&addrlen);
					printf("Host disconnected , ip %s , port %d \n" ,
						inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
						
					//Close the socket and mark as 0 in list for reuse
					close( sd );
					client_socket[i] = 0;
				}
					
				//Echo back the message that came in
				else
				{
					//set the string terminating NULL byte on the end
					//of the data read
					buffer[valread] = '\0';
					send(sd , buffer , strlen(buffer) , 0 );
				}
			}
		}
	}
		
	return 0;

}

struct Datainfo *getParser(char* data){

    //printf("%d", strlen(data));

	int status = 0;

	int flag = 0;
	int tmp_cnt = 0;

    char *tmp = NULL;
	char *strType = NULL;
	char *strTxt = NULL;

	struct Datainfo *pDataInfo = NULL;
	struct Datainfo createDataInfo;

	//printf("한글 %s", data);

    if ( strlen(data) != 0 ){

        if ( strstr(data, "id,") != NULL){
			status = 1;
        }

		else if ( strstr(data, "passwd,") != NULL){
			status = 1;
        }

		// 문자열 구분하기
		if ( status == 1 ){

			pDataInfo = &createDataInfo;

            //tmp = strstr(data, "id,");
			tmp = data;
    //        printf("한글 %s %d", *tmp, strlen(tmp));

			// 데이터 항목명
			tmp_cnt = 0;
			for (int i = 0; i < strlen(tmp) && tmp[i] != ','; i++){
				tmp_cnt++;
			}

			strType = malloc(tmp_cnt);
			for (int i = 0; i < strlen(tmp) && tmp[i] != ','; i++){
				strType[i] = tmp[i];
			}

			//printf("%s ---", strType);
			pDataInfo->str_type = strType;

			// 데이터 내용
			flag = 0;

            for (int i = 0; i < strlen(tmp); i++){
				
				if (tmp[i] == ',' && flag == 0){
					flag = 1;
				}
				else if ( tmp[i] == '#' && flag == 1){
					flag = 0;
				}

				if ( flag == 1 && tmp[i] != ',' ){
					tmp_cnt++;
				}

            }

			flag = 0;
			strTxt = malloc(tmp_cnt);

			tmp_cnt = 0;

			for (int i = 0; i < strlen(tmp); i++){
				
				if (tmp[i] == ',' && flag == 0){
					flag = 1;
				}
				else if ( tmp[i] == '#' && flag == 1){
					flag = 0;
				}

				if ( flag == 1 && tmp[i] != ',' ){
					//printf("%c", tmp[i]);
					strTxt[tmp_cnt] = tmp[i];
					tmp_cnt++;
				}

            }

			// printf("출력: %s\n", strTxt);
			pDataInfo->str_value = strTxt;
		
		}

    }

    return pDataInfo;

}

int login_checker(struct Datainfo *data){


	//printf("%d", strstr(data->str_type, "id") );
	
	if( strcmp(data->str_type, "id") == 0){

		if( strcmp(data->str_value, "rabbit.white@daum.net") == 0){
			return 1;
		}
		//printf("참1: %d", strcmp(data->str_value, "rabbit.white@daum.net"));
	}
	else if ( strcmp(data->str_type, "passwd") == 0){

		if( strcmp(data->str_value, "a1234b") == 0){
			return 1;
		}
		//printf("참2: %d", strcmp(data->str_value, "a1234b"));
	}

	return 0;

}