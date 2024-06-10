/*
 *       Name: My HttServer.c
 *       Date: 2024-6-9
 */
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<string.h>
#include<ctype.h>
#include<errno.h>
#include<arpa/inet.h>
#include<pthread.h> 

#define serverPort 80
#define maxSize 1024
#define debug 1

struct stat myStat;

void *dealHttpRequest(void *);
void dealHttpResponse(int , const char *);
void notFound(int );//404
void innerError(int );//500
void unimplemented(int );//501
void badRequest(int );// 400
void sendHead(int , FILE *);
void sendBody(int , FILE *);
int getLineDetail(char *, char *, int , int );
int getLineData(int , char *, int );



int main(void)
{
	int socketId;
	struct sockaddr_in server_addr;

	// 1. Create socket id
	socketId = socket(AF_INET,SOCK_STREAM,0);
	if (socketId < 0)
	{
		perror("socket");
		exit(-1);
	}
	printf("Socket success.......\n");
	
	//Clear lable, write id and port
	bzero(&server_addr,sizeof(server_addr));
	
	// 2. Bind ip and port
	server_addr.sin_family = AF_INET; // select IPv4
	server_addr.sin_port = htons(serverPort); // bind port
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // lesten any Addr
	int bindId = bind(socketId, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(bindId < 0)
	{
		perror("bind");
		exit(-1);
	}
	printf("Bind success.....\n");

	// 3. Listen socketID
	int listenId = listen(socketId,128);
	if(listenId < 0)
	{
		perror("listen");
		exit(-1);
	}
	printf("wait client link\n");
	
	while(1)
	{
		struct sockaddr_in client;
		socklen_t client_addr_len;
		client_addr_len = sizeof(client);
		int client_sockId;
		client_sockId = accept(socketId, (struct sockaddr *)&client, &client_addr_len);
		if(client_sockId < 0)
		{
			printf("Client connect error......\n");
			continue;
		}
		
		// print client ip and port
		char client_ip[64];
		printf("client ip: %s\nclient port: %d\n",
				inet_ntop(AF_INET, &client.sin_addr.s_addr,client_ip,sizeof(client_ip)),
				ntohs(client.sin_port));
		
		// deal http request and read client send data
		//dealHttpRequest(client_sockId);
		
		// creat pthread to deal with http request
		pthread_t pid;
		int *pClientSockId = NULL;
		pClientSockId = (int *)malloc(sizeof(int));
		*pClientSockId = client_sockId;
		pthread_create(&pid,NULL, dealHttpRequest, (void *)pClientSockId);

	}
	return 0;

}

// HTTP head deal
void *dealHttpRequest(void *pClientSockId)
{
	// read http request
	// read a line data
	int client_sockId = *(int *)pClientSockId;
	int dataLength = 0;
	char buf[maxSize];
	dataLength = getLineData(client_sockId, buf, sizeof(buf)-1);
	// get request line
	char method[64];
	char url[256];
	char path[512];
	if(dataLength > 0)
	{
		int j = 0;
		j = getLineDetail(method, buf, sizeof(method)-1, j);
		if(debug) printf("Request method: %s\n", method);
		// only deal with GET
		if(!strncasecmp(method, "GET",3))
		{
			while(isspace(buf[j++]));
			j = getLineDetail(url, buf, sizeof(url)-1, j);
			if(debug) printf("Request url: %s\n\n",url);
			
			do
			{
				dataLength =  getLineData(client_sockId, buf, sizeof(buf)-1);
				if(debug) printf("read: %s\n",buf);
			}while(dataLength > 0);
			
			// index server local html
			
			// deal ? in url
			char *pos = strchr(url,'?');
			if(pos)
			{
				*pos = '\0';
				printf("real url: %s\n",url);
			}
			sprintf(path, "./HTML/%s", url);
			if(debug) printf("path: %s\n", path);
			
			// execute http answer
			// if file exist, response 200 OK, send html file
			// if file not exist, response 404 NOT FOUND
			if(!stat(path, &myStat))
			{
				if(S_ISDIR(myStat.st_mode))
				{
					strcat(path, "/index.html");
				}
				dealHttpResponse(client_sockId, path);
			}
			else
			{
				fprintf(stderr, "stat %s failed. Reason: %s\n", path, strerror(errno));
				notFound(client_sockId);
			}
		}
		else
		{
			// not get request, read http head and response client 501
			// Method Not Implemented
			fprintf(stderr, "warning other request [%s]\n", method);
			
			do
			{
				dataLength =  getLineData(client_sockId, buf, sizeof(buf)-1);
				if(debug) printf("read: %s\n",buf);
			}while(dataLength > 0);
			
			unimplemented(client_sockId);// Not GET request
		}
		
	}
	else
	{
		// format question
		printf("request question\n");
		badRequest(client_sockId);
		
	}
	
	close(client_sockId);
	if(pClientSockId) free(pClientSockId);
	
	return NULL;
}

// HTTP Response
void dealHttpResponse(int client_sockId, const char *path)
{
	FILE *resource = NULL;
	resource = fopen(path, "r");
	if(!resource)
	{
		notFound(client_sockId);
		return;
	}

	// 1. send mainHeader
	sendHead(client_sockId, resource);
	// 2. create Content-Length and send
	sendBody(client_sockId, resource);
	fclose(resource);
	// 3. send html document
	
}

void sendHead(int client_sockId, FILE *resource)
{
	char buf[1024] = "HTTP/1.0 200 OK \r\nServer: MyServer\r\nContent-Type: text/html\r\nConnection: Close\r\n";
	
	struct stat inMyStat;
	int fileId = 0;
	fileId = fileno(resource);
	
	if(fstat(fileId,&inMyStat) == -1)
	{
		innerError(client_sockId);
	}
	char tmp[64];
	snprintf(tmp, 64, "Content-Length: %ld\r\n\r\n",inMyStat.st_size);
	strcat(buf,tmp);
	
	if(debug) fprintf(stdout, "header: %s\n", buf);
	
	int sendRes = send(client_sockId, buf, strlen(buf), 0);
	if(sendRes < 0)
	{
		fprintf(stderr, "send failed. data: %s\n, reason: %s\n", buf, strerror(errno));
	}
}

// send html file to client
void sendBody(int client_sockId, FILE *resource)
{
	char buf[1024];
	fgets(buf, sizeof(buf), resource);
	while(!feof(resource))
	{
		int len = write(client_sockId, buf,strlen(buf));
		
		// send failed
		if(len < 0)
		{
			fprintf(stderr, "send body error. reason: %s\n",strerror(errno));
			break;
		}
		if(debug) fprintf(stdout,"%s",buf);
		fgets(buf, sizeof(buf), resource);
	}
}

int getLineDetail(char *Result, char *buf, int size, int j)
{
	int i = 0;
	while((i < size) && !isspace(buf[j]))
	{
		Result[i] = buf[j];
		i++,j++;
	}
	Result[i] = '\0';
	return j;
}

//return -1: read error, >0: read line success, =0: NULL
int getLineData(int socketId, char *buf, int bufSize)
{
	int cnt = 0, len = 0;
	char ch = '\0';
	
	while((cnt < bufSize) && ch != '\n')
	{
		len = read(socketId, &ch, 1);
		if(len == 1)
		{
			if(ch == '\r') continue;
			else if(ch == '\n') break;
			else
			{
				buf[cnt] = ch;
				cnt++;
			}
			
		}
		else if(len == -1)
		{
			perror("Read failed:");
			return -1;
		}
		else
		{
			fprintf(stderr,"Client close.\n");
			return -1;
		}
	}
	buf[cnt] = '\0';
	return cnt;
}

// 404
void notFound(int client_sockId)
{
	FILE *resource = NULL;
	char *path = "./HTML/404.html";
	resource = fopen(path, "r");
	if(!resource)
	{
		printf("open file error\n");
		return;
	}
	sendHead(client_sockId, resource);
	sendBody(client_sockId, resource);
	fclose(resource);
}

// 500
void innerError(int client_sockId)
{
	FILE *resource = NULL;
	char *path = "./HTML/500.html";
	resource = fopen(path, "r");
	if(!resource)
	{
		printf("open file error\n");
		return;
	}
	sendHead(client_sockId, resource);
	sendBody(client_sockId, resource);
	fclose(resource);
}

// 501
void unimplemented(int client_sockId)
{
	FILE *resource = NULL;
	char *path = "./HTML/501.html";
	resource = fopen(path, "r");
	if(!resource)
	{
		printf("open file error\n");
		return;
	}
	sendHead(client_sockId, resource);
	sendBody(client_sockId, resource);
	fclose(resource);
}

// 400
void badRequest(int client_sockId)
{
	FILE *resource = NULL;
	char *path = "./HTML/400.html";
	resource = fopen(path, "r");
	if(!resource)
	{
		printf("open file error\n");
		return;
	}
	sendHead(client_sockId, resource);
	sendBody(client_sockId, resource);
	fclose(resource);
}
