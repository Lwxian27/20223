#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  N  16
#define  R  1          //register
#define  L   2         //login
#define  Q  3         //quit
#define INSERT  6    
#define DELETE 7
#define UPDATE 8
#define SELECT  9  
#define EXIT 5

#define DATABASE "employee.db" 
 
typedef struct {
	int type;
	char name[N];
	char data[256];   // password or word or remark 
	char uname[N];
	char sex;
	int age;
} MSG;

void do_register(int socketfd, MSG *msg);
int do_login(int socketfd, MSG *msg);
void do_manager(int socketfd, MSG *msg);
void manager_insert(int socketfd, MSG *msg);
void manager_delete(int socketfd, MSG *msg);
void manager_update(int socketfd, MSG *msg);
void manager_select(int socketfd, MSG *msg);

int main(int argc, char const *argv[])
{
	  int socketfd;
	 struct sockaddr_in server_addr;
	 MSG msg;
	 if (argc < 3)
	 {
		 	printf("Usage : %s <serv_ip> <serv_port>\n", argv[0]);
		    exit(-1);
	 }
	 
    if ((socketfd = socket(AF_INET,SOCK_STREAM,0) )== -1 )
	{
			perror("fail to socket");
		    exit(-1);
	}
	
	bzero(&server_addr, sizeof(server_addr));
  
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr =  inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	if (connect(socketfd,( const struct sockaddr *)&server_addr,sizeof(server_addr)) == -1 )
	{
		perror("connet error");
		exit(-1);
	}
	
	int n;
	while (1)
	{
		printf("************************************\n");
		printf("* 1: register   2: login   3: quit *\n");
		printf("************************************\n");
		printf("please choose : ");

		if(scanf("%d", &n) <= 0)
		{
			perror("scanf");
			exit(-1);
		} 
		switch(n)
		{
			case R: 
			    do_register(socketfd, &msg);
				break;
			case L: 
				if (do_login(socketfd, &msg)) 
					do_manager(socketfd,&msg); 
			  break;
			case Q:
				close(socketfd);
				exit(0);
		} 
	}  
	return 0;
}
 
void do_register(int socketfd, MSG *msg)
{
    //???????????????
	msg->type = R;
    //???????????????
	printf("input your name:");
	scanf("%s", msg->name);
	//????????????
    printf("input your password:");
	scanf("%s", msg->data);
	//????????????
    send(socketfd, msg, sizeof(MSG), 0);
	//?????????????????????
    recv(socketfd, msg, sizeof(MSG), 0);
	printf("register : %s\n", msg->data); 
    return;
}

int  do_login(int socketfd, MSG *msg){

	    int flag = 0;
             //???????????????
		msg->type = L ;
		//???????????????
		printf("input your name:");
		scanf("%s", msg->name);
		//????????????
		printf("input your password:");
		scanf("%s", msg->data);
		//????????????
		send(socketfd, msg, sizeof(MSG), 0);
		//?????????????????????
		recv(socketfd, msg, sizeof(MSG), 0);
		printf("login : %s\n", msg->data); 
		if (strcmp(msg->data,"OK") == 0)
		{
			 flag = 1;
		} 
		return  flag; 
}

void do_manager(int socketfd, MSG *msg)
{
       	int n;
	while (1)
	{
		printf("**********************************************************\n");
		printf("* 6: insert   7: delete   8: update   9.select   5.exit *\n");
		printf("**********************************************************\n");
		printf("please choose : ");

		if(scanf("%d", &n) <= 0)
		{
			perror("scanf");
			exit(-1);
		}

		switch(n)
		{
			case INSERT: 
			     manager_insert(socketfd, msg);
				break;
			case DELETE: 
			     manager_delete(socketfd, msg);
			  break;
		    case UPDATE: 
			  manager_update(socketfd, msg);
			  break;
			case SELECT: 
			  manager_select(socketfd, msg);
			  break;
			case EXIT:   
			  return;
		} 
	}  
}

void manager_insert(int socketfd, MSG *msg)
{
    //???????????????
	msg->type = INSERT; 
    //???????????????
	printf("input  name:");
	scanf("%s", msg->uname);  
	printf("input sex:");
	scanf("%s",&msg->sex); 
	printf("input age:");
	scanf("%d",&msg->age);  
	//????????????
    send(socketfd, msg, sizeof(MSG), 0);
	//?????????????????????
    recv(socketfd, msg, sizeof(MSG), 0);
	printf("insert : %s\n", msg->data); 
    return;
}

void manager_delete(int socketfd, MSG *msg)
{
    //???????????????
	msg->type = DELETE; 
    //?????????????????????
	printf("delete  name:");
	scanf("%s", msg->uname);   
	//????????????
    send(socketfd, msg, sizeof(MSG), 0);
	//?????????????????????
    recv(socketfd, msg, sizeof(MSG), 0);
	printf("delete : %s\n", msg->data); 
    return;
}

void manager_update(int socketfd, MSG *msg)
{
    //???????????????
	msg->type = UPDATE; 
    //?????????????????????
	printf("update  name:");
	scanf("%s", msg->uname);   
	printf("update  sex:");
	scanf("%s", &msg->sex);  
	printf("update  age:");
	scanf("%d", &msg->age);  
	//????????????
    send(socketfd, msg, sizeof(MSG), 0);
	//?????????????????????
    recv(socketfd, msg, sizeof(MSG), 0);
	printf("update : %s\n", msg->data); 
    return;
}

void manager_select(int socketfd, MSG *msg)
{
    //???????????????
	msg->type = SELECT; 
    //?????????????????????
	printf("select  name:");
	scanf("%s", msg->uname);    
	//????????????
    send(socketfd, msg, sizeof(MSG), 0);
	//?????????????????????
    recv(socketfd, msg, sizeof(MSG), 0);
	printf("select : %s\n", msg->data); 
	if (!strcmp(msg->data,"OK"))
	{ 
	   printf("uname:%s   sex:%c   age:%d\n",msg->uname,msg->sex,msg->age);
	} 
    return;
}


