#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define _SIZE_ 1024
int main()
{
	char method[_SIZE_];
	char content_len[_SIZE_];
	char content_data[_SIZE_];

	if(getenv("METHOD"))
	{
		strcpy(method,getenv("METHOD"));
	}
	else
	{

		return 1;
	}

	if(strcasecmp(method,"GET")==0)
	{
		if(getenv("QUERY_STRING"))
		{
			strcpy(content_data,getenv("QUERY_STRING"));
		}
		else
		{

			return 1;
		}
	}
	else if(strcasecmp(method,"POST")==0)
	{
		if(getenv("CONTENT_LEN"))
		{
			strcpy(content_len,getenv("CONTENT_LEN"));
		}
		else
		{

			return 1;
		}

		int i=0;
		char ch='\0';
		for(i=0;i<atoi(content_len);i++)
		{
			read(0,&ch,1);
			content_data[i]=ch;
		}
		content_data[i]='\0';
	}

	int data1=0;
	int data2=0;
	char *arr[3];
	int i=0;
	char* start=content_data;
	while(*start)
	{
		if(*start=='=')
		{
			arr[i++]=start+1;
		}
		else if(*start=='&')
		{
			*start='\0';
		}
		start++;
	}
	data1=atoi(arr[0]);
	data2=atoi(arr[1]);


	printf("<html>");
	printf("<h2>");
	printf("<tr>%d+%d=%d</tr><br/>",data1,data2,data1+data2);
	printf("<tr>%d-%d=%d</tr><br/>",data1,data2,data1-data2);
	printf("<tr>%d*%d=%d</tr><br/>",data1,data2,data1*data2);
	printf("<tr>%d/%d=%d</tr><br/>",data1,data2,data1/data2);
	printf("<tr>%d%%%d=%d</tr><br/>",data1,data2,data1%data2);
	printf("</h2>");
	printf("</html>");

	return 0;
}
