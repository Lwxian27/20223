#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sqlite3.h> 
#include <unistd.h>
#include <string.h>

#define  N  16
#define  R  1          //register
#define  L   2         //login
#define  Q  3         //quit
#define INSERT 6
#define DELETE 7
#define UPDATE 8
#define SELECT  9

#define DATABASE  "employee.db" 

typedef struct {
	int type;
	char name[N];
	char data[256];   // password or word or remark 
	char uname[N];
	char sex;
	int age;
} MSG;

void do_client(int connectfd, sqlite3 *db);
void do_register(int connectfd, MSG *msg, sqlite3 *db);
void do_login(int connectfd, MSG *msg, sqlite3 *db); 
void do_insert(int connectfd, MSG *msg, sqlite3 *db);
void do_delete(int connectfd, MSG *msg, sqlite3 *db);
void do_update(int connectfd, MSG *msg, sqlite3 *db);
void do_select(int connectfd, MSG *msg, sqlite3 *db);

int main(int argc, char const *argv[])
{
	int sockfd, connectfd;
	struct sockaddr_in server_addr;
	pid_t pid;
	sqlite3 *db;
	
    if (argc < 3)
	{
		printf("Usage : %s <ip> <port>\n", argv[0]);
		exit(-1);
	}
	
	if (sqlite3_open(DATABASE, &db) != SQLITE_OK)
	{
		printf("error : %s\n", sqlite3_errmsg(db));
		exit(-1);
	}
	sqlite3_exec(db, "create table usr(name text primary key, pass text)", NULL, NULL, NULL);
	sqlite3_exec(db, "create table info (name text  primary key, sex text, age int)", NULL, NULL, NULL);

	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket error");
		exit(-1);
	}
	
 bzero(&server_addr, sizeof(server_addr));
 server_addr.sin_family = AF_INET;
 server_addr.sin_addr.s_addr = inet_addr(argv[1]);
 server_addr.sin_port = htons(atoi(argv[2]));

 if (bind(sockfd,(const struct sockaddr *)&server_addr,sizeof(server_addr)) == -1 )
 {
	  perror("bind error");
	  exit(-1);
 }
 
   if (listen(sockfd, 5) < 0)
	{
		perror("fail to listen");
		exit(-1);
	}

	while ( 1 )
	{
		if ((connectfd = accept(sockfd, NULL, NULL)) < 0)
		{
			perror("fail to accept");
			exit(-1);
		}
		
        if ((pid = fork()) < 0)
		{
			perror("fail to fork");
			exit(-1);
		}
		else if(pid == 0) //子进程执行处理代码
		{
			do_client(connectfd, db);
		}
        else  //父进程负责连接
        {
		    close(connectfd);
        }
	}
	
	return 0;
} 

void do_client(int connectfd, sqlite3 *db)
{
	MSG msg;
	while (recv(connectfd, &msg, sizeof(MSG), 0) > 0)  // receive request
	{
		printf("type = %d\n", msg.type);
		printf("data= %s\n", msg.data);
		switch ( msg.type )
		{
		case R :
			do_register(connectfd, &msg, db);
			break;
		case L :
			do_login(connectfd, &msg, db);
			break;
		case Q : 
			exit(0);
			break;
		case  INSERT:
		    do_insert(connectfd, &msg, db);
			break; 
		case  DELETE:
		    do_delete(connectfd, &msg, db);
			break;
		 case  UPDATE:
		    do_update(connectfd, &msg, db);
			break; 
		case  SELECT:
		    do_select(connectfd, &msg, db);
			break; 
		}
	}
	printf("client quit\n");
	exit(0);
	return;
}

void do_register(int connectfd, MSG *msg, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char *errmsg;
    
    //使用sqlite3_exec函数调用插入函数判断是否能够插入成功
    //由于用户名设置为主键，所以如果用户名已经存在就会报错
	sprintf(sqlstr, "insert into usr values('%s', '%s')", msg->name, msg->data);
	if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		sprintf(msg->data, "user %s already exist!!!", msg->name);
	}
	else
	{
		strcpy(msg->data, "OK");
	} 
	send(connectfd, msg, sizeof(MSG), 0); 
	return;
}

void do_login(int connectfd, MSG *msg, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char *errmsg, **result;
	int nrow, ncolumn;

    //通过sqlite3_get_table函数查询记录是否存在
	sprintf(sqlstr, "select * from usr where name = '%s' and pass = '%s'", msg->name, msg->data);
	if(sqlite3_get_table(db, sqlstr, &result, &nrow, &ncolumn, &errmsg) != SQLITE_OK)
	{
		printf("error : %s\n", errmsg);
	}
    //通过nrow参数判断是否能够查询到疾记录，如果值为0，则查询不到，如果值为非0，则查询到
	if(nrow == 0)
	{
		strcpy(msg->data, "name or password is wrony!!!");
	}
	else
	{
		strncpy(msg->data, "OK", 256);
	} 
	send(connectfd, msg, sizeof(MSG), 0); 
	return;
}

void do_insert(int connectfd, MSG *msg, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char *errmsg;
    
    //使用sqlite3_exec函数调用插入函数判断是否能够插入成功
    //由于用户名设置为主键，所以如果用户名已经存在就会报错
	sprintf(sqlstr, "insert into info values('%s', '%c','%d')", msg->uname, msg->sex,msg->age);
	if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		sprintf(msg->data, "info %s already exist!!!", msg->uname);
	}
	else
	{
		strcpy(msg->data, "OK");
	} 
	send(connectfd, msg, sizeof(MSG), 0); 
	return;
}
void do_delete(int connectfd, MSG *msg, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char *errmsg; 
	printf("name = %s \n",msg->name);
	if (!strcmp(msg->name,"admin"))
	{
		    printf("管理员删除功能\n");
		   	sprintf(sqlstr, "delete from info where name = '%s'", msg->uname); 
			if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
			{
				  printf("管理员删除功能，删除失败\n");
				sprintf(msg->data, "info %s already not exist!!!", msg->uname);
			}
			else
			{
				printf("管理员删除功能，删除成功\n");
				strcpy(msg->data, "OK");
			} 
	} else
	{
		  strcpy(msg->data,"该员工无删除权限");
	}  
	send(connectfd, msg, sizeof(MSG), 0); 
	return;
}

void do_update(int connectfd, MSG *msg, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char *errmsg; 
	printf("name = %s \n",msg->name);
	printf("uname = %s \n",msg->uname);
	if (!strcmp(msg->name,"admin"))
	{
		    printf("管理员修改功能\n");
		   	sprintf(sqlstr, "update info  set  sex = '%c',age = '%d' where name = '%s'", msg->sex,msg->age,msg->uname); 
			if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
			{
				  printf("管理员修改功能，修改失败\n");
				sprintf(msg->data, "info %s already not exist!!!", msg->uname);
			}
			else
			{
				printf("管理员修改功能，修改成功\n");
				strcpy(msg->data, "OK");
			} 
	} else
	{
		  strcpy(msg->data,"该员工无修改权限");
	}  
	send(connectfd, msg, sizeof(MSG), 0); 
	return;
}

void do_select(int connectfd, MSG *msg, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char *errmsg;  
	char **dbResult;
	int nRow=0, nColumn=0;  
	printf("name = %s \n",msg->name);
	if (!strcmp(msg->name,"admin"))
	{
		    printf("管理员查询功能\n");
		   	sprintf(sqlstr, "select * from info ");  
			if(sqlite3_get_table(db, sqlstr, &dbResult, &nRow,&nColumn,&errmsg) != 0)
			{
				  printf("管理员查询功能，查询失败\n");
				sprintf(msg->data, "info %s already not exist!!!", msg->uname);
			}
			else
			{
				printf("管理员查询功能，查询成功\n"); 
				msg->sex = atoi(dbResult[1]);
				msg->age = atoi(dbResult[2]);
				strcpy(msg->data, "OK");
			} 
	} else
	{
		  strcpy(msg->data,"该员工无查询权限");
	}  
	send(connectfd, msg, sizeof(MSG), 0); 
	return;
}

