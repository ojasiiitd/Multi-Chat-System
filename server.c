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
	int len , i , j;
	char buffer[1000] , bufferUpdate[1000];

	len = recv(newClient.socketNum , buffer , 1000 , 0);
	while(len > 0)
	{
		len = recv(newClient.socketNum , buffer , 1000 , 0);
		buffer[len] = '\0';

		int colon;
		for(colon=0 ; buffer[colon]!='\0' ; colon++)
			if(buffer[colon] == ':')
				break;
		colon += 2;

		if(buffer[colon++] == '@')
		{
			printf("PRIVATE MESSAGE\n");
			int to = buffer[colon++] - '0';

			int filled = 0;
			for(int i=0 ; buffer[i]!=':' ; i++)
				bufferUpdate[filled++] = buffer[i];
			bufferUpdate[filled++] = ':';
			bufferUpdate[filled++] = '\t';
			for(int i=colon ; buffer[i]!='\0' ; i++)
				bufferUpdate[filled++] = buffer[i];
			bufferUpdate[filled] = '\0';

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
	printf("An existing user, #%d, has left!\n" , n);
	for(i = 0; i < n; i++)
	{
		if(allClients[i].socketfd == newClient.socketNum)
		{
			j = i;
			while(j < n-1)
			{
				allClients[j].socketfd = allClients[j+1].socketfd;
				j++;
			}
		}
	}
	n--;
}

int main(int argc , char* argv[])
{
	struct sockaddr_in serverAddress , clientAddress;
	struct clients newClient;
	socklen_t cliLen;
	int oldsockfd , newsockfd , port , len , check;
	char buffer[1000];
	
	pthread_t receivedThread;

	port = atoi(argv[1]);
	oldsockfd = socket(AF_INET,SOCK_STREAM,0);
	bzero((char *) &serverAddress , sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	
	cliLen = sizeof(clientAddress);
	check = bind(oldsockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
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
		
		printf("A new user, #%d, has joined!\n" , n+1);

		newClient.socketNum = newsockfd;
		allClients[n++].socketfd = newsockfd;

		pthread_create(&receivedThread , NULL , receiveMsg , &newClient);
		sleep(1);
	}
	return 0;
}