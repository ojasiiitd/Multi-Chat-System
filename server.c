#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>

struct clientInfo
{
	int socketfd;
	int status;
	char username[50];
};

struct clientInfo allClients[100];
int n = 0;

struct clients
{
	int socketNum;
};

void* receiveMsg(void* curSocket)
{
	struct clients newClient = *((struct clients *)curSocket);
	int len , i , j , left;
	char buffer[1000] , bufferUpdate[1000];

	len = recv(newClient.socketNum , buffer , 1000 , 0);
	while(len > 0)
	{
		len = recv(newClient.socketNum , buffer , 1000 , 0);
		buffer[len] = '\0';

		if(len == 0)
		{
			printf("HEREEE\n");
			int flag = 1;
			for(i=0 ; i<n ; i++)
			{
				if(allClients[i].socketfd == newClient.socketNum)
				{
					flag = -1;
					left = i;
					printf("%d\n" , left);
					break;
				}
			}
			if(flag == -1)
				break;
		}

		int colon;
		for(colon=0 ; buffer[colon]!='\0' ; colon++)
			if(buffer[colon] == ':')
				break;
		colon += 2;

		if(buffer[colon++] == '@')
		{
			printf("PRIVATE MESSAGE\n");
			int to = buffer[colon++] - '0';

			if(to > n || to <= 0)
			{
				printf("Input Error : please enter a valid client number.\n");
				continue;
			}

			if(allClients[to-1].status == -1)
			{
				printf("The client has already left the chat.\n");
				continue;
			}

			int filled = 0;
			for(int i=0 ; buffer[i]!=':' ; i++)
				bufferUpdate[filled++] = buffer[i];
			bufferUpdate[filled++] = ':';
			bufferUpdate[filled++] = '\t';
			for(int i=colon ; buffer[i]!='\0' ; i++)
				bufferUpdate[filled++] = buffer[i];
			bufferUpdate[filled] = '\0';

			// sending to only 'to' client
			int check = send(allClients[to-1].socketfd , bufferUpdate , strlen(bufferUpdate),0);
			if(check < 0)
			{
				perror("error while sending");
				continue;
			}
		}

		else
		{
			printf("BROADCAST MESSAGE\n");
			
			// sending to all clients
			for(i=0 ; i<n ; i++)
			{
				if(allClients[i].socketfd != newClient.socketNum)
				{
					int check = send(allClients[i].socketfd , buffer , strlen(buffer),0);
					if(check < 0)
					{
						perror("error while sending");
						continue;
					}
				}
			}
		}

		bzero(buffer , sizeof(buffer));
		bzero(bufferUpdate , sizeof(bufferUpdate));
	}

	allClients[left].status = -1;
	printf("User #%d has left!\n" , left+1);
}

int main(int argc , char* argv[])
{
	struct sockaddr_in serverAddress , clientAddress;
	struct clients newClient;
	socklen_t cliLen , serLen;
	int oldsockfd , newsockfd , port , len , check;
	char buffer[1000];
	
	pthread_t receivedThread;

	port = atoi(argv[1]);
	oldsockfd = socket(AF_INET , SOCK_STREAM , 0);
	bzero((char *) &serverAddress , sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	
	cliLen = sizeof(clientAddress);

	check = bind(oldsockfd , (struct sockaddr *)&serverAddress , sizeof(serverAddress));
	if(check < 0)
	{
		perror("error while binding");
		exit(1);
	}

	check = listen(oldsockfd , 7);
	if(check < 0)
	{
		perror("error while listening");
		exit(1);
	}

	for(;;)
	{
		newsockfd = accept(oldsockfd , (struct sockaddr *)&clientAddress , &cliLen); 
		if(newsockfd < 0)
		{
			perror("error while accepting");
			exit(1);
		}
		
		char uName[50];
		recv(newsockfd , uName , 50 , 0);
		printf("A new user has joined :- %s , #%d\n" , uName , n+1);

		newClient.socketNum = newsockfd;
		allClients[n].socketfd = newsockfd;
		allClients[n].status = 1;
		strcpy(allClients[n++].username , uName);

		pthread_create(&receivedThread , NULL , receiveMsg , &newClient);
	}

	close(oldsockfd);
	close(newsockfd);

	return 0;
}