#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>


#define PORT 8080
#define MAXCONN 10
#define MSGSIZE 90000
#define HDRSIZE 512
#define MAXFNAME 32
#define FOUND404 "HTTP/1.1 404 Not Found\r\n"
#define HTTPOK	"HTTP/1.1 200 OK\r\n"

extern int errno;

char *get_Extension(char *filename){
	return strrchr(filename,'.');
}

char *get_ContentType(char *filename){
	char *cType=(char *)malloc(MAXFNAME*2);
	strcpy(cType,"Content-Type: ");
	char *fileExtension=get_Extension(filename);
	if(strcmp(fileExtension,".html")==0||strcmp(fileExtension,".hml")==0||strcmp(fileExtension,".htm")==0){
		strcpy(cType,"text/html\r\n");
	}
	else if(strcmp(fileExtension,".txt")==0||strcmp(fileExtension,".c")==0||strcmp(filename,".tex")==0){
		strcpy(cType,"text/plain\r\n");
	}
	else if(strcmp(fileExtension,".c")==0){
		strcpy(cType,"text/plain\r\n");
	}
	else if(strcmp(filename,".pdf")==0){
		strcpy(cType,"application/pdf\r\n");
	}
	else if(strcmp(filename,".tex")==0){
		strcpy(cType,"text/plain\r\n");
	}
	else{
		strcpy(cType,"application/octet-stream\r\n");
	}
	return cType;
}

int get_FileNr(char* ommit){
	DIR *dirp;
	struct dirent *ent;
	int noFiles=0;
	dirp=opendir("./");
	if (!dirp){
		perror("error @open");
	}

	while((ent=readdir(dirp))!=NULL)
	{
		if (ent->d_type==4){continue;}
		else{
			if (strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0||strcmp(ent->d_name,ommit)==0)
				continue;
			else
				noFiles++;
			}
		}
	(void)closedir(dirp);
	return noFiles;
}


char **get_Files(char* ommit){
	int noFiles=get_FileNr(ommit);
	int i;
	struct dirent *ent;
	DIR *dirp;
	dirp=opendir("./");

   char **cwdFiles=malloc(sizeof(char)*MAXFNAME*(noFiles+1));
   
   dirp=opendir("./");
   if (!dirp){
   	perror("error @open");
   }
   i=0;
   while((ent=readdir(dirp))!=NULL)
   {
   	if (ent->d_type==4){continue;}
   	else{
   		if (strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0||strcmp(ent->d_name,ommit)==0)
   	   		continue;
   	   	else {
   	   		cwdFiles[i]=malloc(sizeof(char)*(sizeof(ent->d_name)+1));
   	   		strcpy(cwdFiles[i],ent->d_name);
   	   		i++;
   		}
   	}
   }

   return cwdFiles;
}

const char *createPayload(int noFiles, char** cwdFiles){
	char payloadStart[]="<html><head><meta charset=\"UTF-8\"><title>Simple Web Server</title></head><body><H1>File Index:</H1>\n";
	char payloadEnd[]="</body></html>\r\n";
	char *payload=malloc(sizeof(char)*noFiles*2*26+strlen(payloadEnd)+strlen(payloadStart));
	strcat(payload,payloadStart);
	int index=0;
	for (;index<noFiles;index++){
		strcat(payload,"<p><a href=\"./");
		// strcat(payload,"<p>");
		strcat(payload,cwdFiles[index]);
	 	strcat(payload,"\">");
	 	strcat(payload,cwdFiles[index]);
	 	strcat(payload,"</a></p>\n");
		// strcat(payload,"</p>\n");
	}
	strcat(payload,payloadEnd);
	printf("%s\n",payload);

	return payload;
}

const char *createHeader(char *status, int cLength, char* cType,int keepAlive){
	char *header=malloc(sizeof(char) * HDRSIZE);
	memset(header,0,HDRSIZE);
	keepAlive=1;
	// bzero(header,HDRSIZE);
	strcat(header,status);
	// strcat(header,"Content-length: 47\n");
	strcat(header,"Content-length: ");
	char str[12];
	sprintf(str,"%d",cLength);
	strcat(header,str);
	strcat(header,"\r\nContent-Type: ");
	strcat(header,cType);
	if (keepAlive==0){
		strcat(header,"Connection: close\r\n");
	}
	else{
		strcat(header,"Connection: keep-alive\r\n");
	}
	strcat(header,"Date: ");
	time_t t=time(NULL);
	struct tm *tm = localtime(&t);
	char date[64];
	strftime(date,sizeof(date),"%c",tm);
	strcat(header,date);
	strcat(header," EET\nServer: Simple Web Server\r\n\n");
	return header;
}

int sendFile(int connection,char *filename){
	char *fileBuffer=(char *)malloc(sizeof(char)*MSGSIZE);
	char *header=(char *)createHeader(FOUND404,0,"text/html\r\n",0);
	char *header404=(char *)malloc(sizeof(char)*strlen(header));
	strcpy(header404,header);
	free(header);
	FILE *fp;
	int fd;
	struct stat filestat;

	if (((fd=open(filename,O_RDONLY))<=-1)||(fstat(fd,&filestat)<0)){
		printf("Could not get the size of the file.\n");
		write(connection,header404,strlen(header404));
		printf("%s\n",header404);
		free(header404);
		return 1;
	}
	fp=fopen(filename,"r");
	if (fp==NULL){
		write(connection,header404,strlen(header404));
		fclose(fp);
		printf("%s\n",header404);
		free(header404);
		return 1;
	}
	else if (fp!=NULL){
		if(strcmp(get_ContentType(filename),"application/octet-stream\r\n")==0){
			printf("Will not send because of not known file type.\n");
			write(connection,header404,strlen(header404));
			fclose(fp);
			printf("%s\n",header404);
			free(header404);
			return 1;
		}
		if(filestat.st_size>MSGSIZE){
			printf("Will not send because file is too big.\n");
			write(connection,header404,strlen(header404));
			fclose(fp);
			printf("%s\n",header404);
			free(header404);
			return 1;

		}
		header=(char *)createHeader(HTTPOK,filestat.st_size,get_ContentType(filename),1);
		char *okHeader=(char *)malloc(sizeof(char)*strlen(header));
		strcpy(okHeader,header);
		free(header);
		write(connection,okHeader,strlen(okHeader));
		printf("%s\n",okHeader);
		free(okHeader);
		fread(fileBuffer,sizeof(char),filestat.st_size+1,fp);
		fclose(fp);
		write(connection,fileBuffer,filestat.st_size);
		return 0;
	}
	free(header404);
	return 1;
}

int verifyRequest(char *buffer){
	char *verify;
	if ((verify=strstr(buffer,"OK"))==NULL){
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[]){

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int fd_server;
	int on=1;
	int nr_client=0;

	//socket creation
	fd_server=socket(AF_INET, SOCK_STREAM, 0);
	if (fd_server<0){
		perror("Socket error.\n");
		return errno;
	}

	//reuse address;
	if (setsockopt(fd_server,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int))<0){
		perror("setsockopt(SO_REUSEADDR) error..\n");
		return errno;
	}

	//where the data structures will be saved;
	bzero(&server_addr,sizeof(server_addr));
	bzero(&client_addr,sizeof(client_addr));


	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(PORT);

	if (bind(fd_server,(struct sockaddr *) &server_addr, sizeof(struct sockaddr))==-1){
		perror("Bind error.\n");
		close(fd_server);
		return errno;
	}

	if (listen(fd_server,MAXCONN)==-1){
		perror("Listen error.\n");
		close(fd_server);
		return errno;
	}

	int noFiles=get_FileNr(argv[0]+2); //+2 because argv[0] includes "./" and we don't want that;
	char **cwdFiles=get_Files(argv[0]+2);
	const char *payload=createPayload(noFiles,cwdFiles);
	free(cwdFiles);
	char *header=(char *)createHeader(HTTPOK,strlen(payload),"text/html\r\n",0);
	char *okHeader=(char *)malloc(sizeof(char)*strlen(header));
	strcpy(okHeader,header);
	free(header);

	while(1){
		socklen_t length = sizeof(client_addr);
		int fd_client;
		printf("%s waiting on port %d..\n",argv[0],PORT);
		fd_client=accept(fd_server,(struct sockaddr *) &client_addr, &length);
		if (fd_client==-1){
			perror("Connection failed.\n");
			continue;
		}
		printf("1 client connected.\n");
		nr_client++;
		printf("There is a total of %d clients connected.\n",nr_client);

		if (!fork()){	//child process
			char clientREQ[HDRSIZE];
			char serverRSP[MSGSIZE];
			char get[3],reqContent[HDRSIZE-MAXFNAME-3];
			char *filename=(char *)malloc(sizeof(char)*MAXFNAME+1);
			char *newfilename=(char *)malloc(sizeof(char)*MAXFNAME);
			memset(&clientREQ,0,sizeof(clientREQ));
			memset(&serverRSP,0,sizeof(serverRSP));

			// read(fd_client,clientREQ,HDRSIZE-1);
			read(fd_client,clientREQ,HDRSIZE-1);
			sscanf(clientREQ,"%s %s %s",get,filename,reqContent);

			if ((filename!=NULL) && (strcmp(filename,"/")==0)){
				printf("%s\n",clientREQ);				
				printf("%s\n",okHeader);

				memset(&serverRSP,0,sizeof(serverRSP));
				strcpy(serverRSP,okHeader);

				write(fd_client,&serverRSP,strlen(okHeader));

				memset(&serverRSP,0,sizeof(serverRSP));
				strcpy(serverRSP,payload);
				write(fd_client, &serverRSP,strlen(payload));

				memset(&serverRSP,0,sizeof(serverRSP));
			}
			else{
				printf("%s\n",clientREQ);
				memset(newfilename,0,strlen(newfilename));
				newfilename=filename+1; // removes the '/' from the http request to get only the filename
				printf("%s\n",newfilename);
				printf("Sending %s.\n",newfilename);
				if (sendFile(fd_client,newfilename)!=0){
					printf("Could not send file.\n");
				}
				else printf("File sent.\n");

			}
		}

		close(fd_client);
		nr_client--;
		printf("1 client disconnected.\n");

	}
	free(okHeader);
	return 0;
}