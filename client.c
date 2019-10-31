#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>

void* receiveMsg(void* curSocket)
{
	char buffer[1000];
	memset(buffer , '\0' , sizeof(buffer));
	int newsocketfd = *((int *)curSocket);
	int len = recv(newsocketfd , buffer , 1000 , 0);
	while(len > 0)
	{
		len = recv(newsocketfd , buffer , 1000 , 0);
		buffer[len] = '\0';
		fputs(buffer,stdout);
		bzero(buffer , sizeof(buffer));
	}
}

int main(int argc , char* argv[])
{
	struct sockaddr_in clientAddress;
	int oldsocketfd , newsocketfd , clieSize , port , len , check;
	char buffer[1000] , username[50] , curMsg[1000];

	pthread_t receivedThread;

	strcpy(username,argv[1]);
	port = atoi(argv[2]);
	oldsocketfd = socket(AF_INET , SOCK_STREAM , 0);
	bzero(clientAddress.sin_zero , sizeof(clientAddress.sin_zero));
	clientAddress.sin_port = htons(port);
	clientAddress.sin_family = AF_INET;

	check = connect(oldsocketfd , (struct sockaddr *)&clientAddress , sizeof(clientAddress));
	if(check < 0)
	{
		perror("error while connecting");
		exit(1);
	}

	printf("You are now connected, %s, say Hello!\n" , username);
	pthread_create(&receivedThread , NULL , receiveMsg , &oldsocketfd);

	len = write(oldsocketfd , username , strlen(username));

	while(fgets(buffer , 1000 , stdin) > 0)
	{
		strcpy(curMsg , username);
		strcat(curMsg , ":\t");
		strcat(curMsg , buffer);

		len = write(oldsocketfd , curMsg , strlen(curMsg));
		if(len < 0)
		{
			perror("error while writing");
			exit(1);
		}

		bzero(buffer , sizeof(buffer));
		bzero(curMsg , sizeof(curMsg));
	}
	pthread_join(receivedThread , NULL);
	
	close(oldsocketfd);

	return 0;
}