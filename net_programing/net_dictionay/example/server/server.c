#include "head.h"

extern int do_client(int connect_fd,sqlite3 *pdb);//处理业务逻辑,do_client.c

void signal_handler(int signum)
{
	waitpid(-1,NULL,WNOHANG);
	return;
}

int init_tcp(char *ip,char *port)
{/*{{{*/
	int sockfd;
	struct sockaddr_in server_addr;
	
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		perror("Fail to socket");
		exit(EXIT_FAILURE);
	}
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port   = htons(atoi(port));
	server_addr.sin_addr.s_addr = inet_addr(ip);

	if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
	{
		perror("Fail to bind");
		exit(EXIT_FAILURE);
	}
	
	listen(sockfd,5);
	printf("listen....\n");
	return sockfd;
}/*}}}*/

//.server ip port db
//数据库中已经手动创建了2个表：user_table,word_table
//注：由于我们后面函数要传承，故这里的const应该去掉
int main(int argc, char *argv[])
{
	int pid;	/*{{{*/
	sqlite3 *pdb;
	int listenfd,connect_fd;
	int addr_len = sizeof(struct sockaddr);
	struct sockaddr_in peer_addr;

	if(argc < 3)
	{
		fprintf(stderr,"Usage : %s ip port system.db.\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	//探测子进程的改变状态，回收僵尸态子进程
	if(signal(SIGCHLD,signal_handler) == SIG_ERR)
	{
		perror("Fail to signal");
		exit(EXIT_FAILURE);
	}

	if(sqlite3_open(argv[3],&pdb) != SQLITE_OK)
	{
		fprintf(stderr,"sqlite3 open %s : %s.\n",argv[3],sqlite3_errmsg(pdb));
		exit(EXIT_FAILURE);
	}

	//初始化tcp连接，得到监听套接字
	listenfd = init_tcp(argv[1],argv[2]);

	//提取客户段的链接请求，创建子进程和客户端交互}}}
	while(1)
	{
		if((connect_fd = accept(listenfd,(struct sockaddr *)&peer_addr,&addr_len)) < 0)
		{
			perror("Fail to accept");
			exit(EXIT_FAILURE);
		}
		
		if((pid = fork()) < 0)
		{
			perror("Fail to fork");
			exit(EXIT_FAILURE);
		}

		//创建子进程处理客户端的请求
		if(pid == 0){
			close(listenfd);
			do_client(connect_fd,pdb);//处理业务逻辑,do_client.c
		}

		close(connect_fd);
	}

	exit(EXIT_SUCCESS);
}
