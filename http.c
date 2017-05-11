#include"http.h"
int startup(const char* _ip,int _port)
{
	assert(_ip);

	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		printf_log("socket failed",FATAL);
		exit(2);
	}

	int opt=1;                     
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_port=htons(_port);
	local.sin_addr.s_addr=inet_addr(_ip);

	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
	{
		printf_log("bind failed",FATAL);
		exit(3);
	}

	if(listen(sock,5)<0)
	{
		printf_log("listen failed",FATAL);
		exit(4);
	}

	return sock;
}

void printf_log(const char* msg,int level)
{

	openlog("http",LOG_PID|LOG_CONS|LOG_NDELAY,LOG_USER);
	switch(level)
	{
		case NORMAL:
			syslog(LOG_NOTICE,msg);
			break;
		case WRONING:
			syslog(LOG_WARNING,msg);
			break;
		case FATAL:
			syslog(LOG_ERR,msg);
			break;
	}
	closelog();
}


static int get_line(int sock,char* buf)
{
	char ch='\0';
	int i=0;
	ssize_t ret=0;
	while(i<SIZE && ch!='\n')
	{
		ret=recv(sock,&ch,1,0);
		if(ret>0&&ch=='\r')
		{
			ssize_t s=recv(sock,&ch,1,MSG_PEEK);
			if(s>0&&ch=='\n')
			{
				recv(sock,&ch,1,0);
			}
			else
			{
				ch='\n';
			}
		}
		buf[i++]=ch;
	}
	buf[i]='\0';
	return i;
}


static void clear_header(int sock)
{
	char buf[SIZE];
	int ret=0;
	do
	{
		ret=get_line(sock,buf);
	}while(ret!=1&&(strcmp(buf,"\n")==0));
}


static void show_404(int sock)
{
	clear_header(sock);
	char* msg="HTTP/1.0 404	Not Found\r\n";
	send(sock,msg,strlen(msg),0);
	send(sock,"\r\n",strlen("\r\n"),0);

	struct stat st;
	stat("wwwroot/404.html",&st);
	int fd=open("wwwroot/404.html",O_RDONLY);
	sendfile(sock,fd,NULL,st.st_size);
	close(fd);
}

void echo_error(int sock,int err_code)
{
	switch(err_code)
	{
		case 403:
			break;
		case 404:
			show_404(sock);
			break;
		case 405:
			break;
		case 500:
			break;
defaut:
			break;
	}
}


static int echo_www(int sock,const char * path,size_t s)
{
	int fd=open(path,O_RDONLY);
	if(fd<0)
	{
		echo_error(sock,403);
		return 7;
	}

	char* msg="HTTP/1.0 200 OK\r\n";
	send(sock,msg,strlen(msg),0); 
	send(sock,"\r\n",strlen("\r\n"),0);

	if(sendfile(sock,fd,NULL,s)<0)
	{
		echo_error(sock,500);
		return 8;	
	}	
	close(fd);
	return 0;
}


static int excu_cgi(int sock,const char* method,\
		const char* path,const char* query_string)
{
	char line[SIZE];
	int ret=0;
	int content_len=-1;
	if(strcasecmp(method,"GET")==0)
	{
		clear_header(sock);
	}
	else
	{
		do
		{
			ret=get_line(sock,line);
			if(strncmp(line,"Content-Length: ",16)==0)
			{
				content_len=atoi(line+16);
			}
		}while(ret!=1&&(strcmp(line,"\n")!=0));
	}


	char method_env[SIZE/8];
	char query_string_env[SIZE/8];
	char content_len_env[SIZE/8];

	sprintf(method_env,"METHOD=%s",method);
	putenv(method_env);

	sprintf(query_string_env,"QUERY_STRING=%s",query_string);
	putenv(query_string_env);

	sprintf(content_len_env,"CONTENT_LEN=%d",content_len);
	putenv(content_len_env);


	int input[2];
	int output[2];

	pipe(input);
	pipe(output);

	pid_t id=fork();

	if(id<0)
	{
		printf_log("fork failed",FATAL);
		echo_error(sock,500);
		return 9;
	}
	else if(id==0)
	{
		close(input[1]);    
		close(output[0]);


		dup2(input[0],0);
		dup2(output[1],1);

		execl(path,path,NULL);
		exit(1);
	}
	else 
	{
		close(input[0]); 
		close(output[1]);

		char ch='\0';
		if(strcasecmp(method,"POST")==0)
		{
			int i=0;
			for( i=0;i<content_len;i++)
			{
				recv(sock,&ch,1,0);
				write(input[1],&ch,1);
			}
		}

		char* msg="HTTP/1.0 200 OK\r\n\r\n";
		send(sock,msg,strlen(msg),0);


		while(read(output[0],&ch,1))
		{
			send(sock,&ch,1,0);
		}
		waitpid(id,NULL,0);
	}

	return 0;
}


int handler_msg(int sock)
{
	char buf[SIZE];
	int count=get_line(sock,buf);
	int ret=0;
	char method[32];
	char url[SIZE];
	char *query_string=NULL;
	int i=0;
	int j=0;
	int cgi=0;


	while( j < count )
	{
		if(isspace(buf[j]))
		{
			break;
		}
		method[i]=buf[j];	
		i++;
		j++;
	}
	method[i]='\0';
	printf("method:%s\n",method);

	while(isspace(buf[j])&&j<SIZE)
	{
		j++;
	}

	if(strcasecmp(method,"POST")&&strcasecmp(method,"GET"))
	{
		printf_log("method failed",FATAL);
		echo_error(sock,405);
		ret=5;
		goto end;
	}

	if(strcasecmp(method,"POST")==0)
	{
		cgi=1;
	}	

	i=0;
	while(j<count)
	{
		if(isspace(buf[j]))
		{
			break;
		}
		if(buf[j]=='?')
		{
			query_string=&url[i];
			query_string++;
			url[i]='\0';
		}
		else
			url[i]=buf[j];
		j++;
		i++;
	}
	url[i]='\0';
	printf("url:%s",url);

	if(strcasecmp(method,"GET")==0&&query_string!=NULL)
	{
		cgi=1;
	}


	char path[SIZE];
	sprintf(path,"wwwroot%s",url);       

	if(path[strlen(path)-1]=='/')
	{
		strcat(path,"index.html");
	}

	struct stat st;           
	if(stat(path,&st)<0)
	{
		printf_log("stat path faile\n",FATAL);
		echo_error(sock,404);
		ret=6;
		goto end;
	}
	else
	{
		if(S_ISDIR(st.st_mode))
		{
			strcat(path,"/index.html");
		}

		if(st.st_mode & (S_IXUSR | S_IXOTH | S_IXGRP))
		{
			cgi=1;
		}
	}

	if(cgi)
	{
		ret=excu_cgi(sock,method,path,query_string);
	}
	else
	{
		clear_header(sock);
		ret=echo_www(sock,path,st.st_size);
	}

end:
	close(sock);
	return ret;
}
