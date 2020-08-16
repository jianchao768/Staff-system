#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include "head.h"


//记录当前登入账户的信息
int * login_id;
unsigned char login_name[20];

//声明函数
int do_register(int sockfd,MSG *msg);
int do_login(int sockfd,MSG *msg);

int root_login_select(int sockfd,MSG *msg);
int root_add_user(int sockfd,MSG *msg);
int root_del_user(int sockfd,MSG *msg);
int root_query_user_data(int sockfd,MSG *msg);
int root_query_all_user_data(int sockfd,MSG *msg);
int root_history(int sockfd,MSG *msg);
int root_history_all(int sockfd,MSG *msg);
int root_change_data(int sockfd,MSG *msg);

int user_login_select(int sockfd,MSG *msg);
int do_query(int sockfd,MSG *msg);
int do_change_data(int sockfd,MSG *msg);
int do_history(int sockfd,MSG *msg);


/******************************** 
* 名字 : main
* 功能 : 主函数
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int main(int argc,const char *argv[])
{
    int sockfd;
	int n,ret;
	MSG msg;
	
	if(argc != 3){
        printf("Usage:%s serverip port\n",argv[0]);
		return -1;
	}

	//新建ipv4网络套接字
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        perror("fail to socket\n");
		return -1;
	}

	//新建服务器id信息并填充
	struct sockaddr_in serveraddr;
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));

    //连接
	if(connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0){
        perror("fail to conect\n");
		return -1;
	}

	char buf[100] = {0};
	while(1){
		printf("************< home >************\n");		
		printf("* 1.register  2.login  3.quit  *\n");		
		printf("********************************\n");		
		printf("Please choose:");

        scanf("%d",&n);
		getchar();
		switch(n){
            case HOME_REG:
				do_register(sockfd,&msg);
				break;
			case HOME_LOGIN:   //如果do_login返回值为1，则是root用户，否则是普通用户,若返回为-1则为注销
			    ret = do_login(sockfd,&msg);
				if(ret == 1){
                    root_login_select(sockfd,&msg);
				}
				else if(ret == 0){
					
                    user_login_select(sockfd,&msg);
				}
				else if(ret < 0){
                    puts("There is no such user");
				}
                break;
			case HOME_QUT:
				close(sockfd);
				return 0;
				break;

			default:
				printf("input error\n");

		}

	}

	return 0;
}


/******************************** 
* 名字 : do_register
* 功能 : register procedure
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int do_register(int sockfd,MSG *msg)
{
    msg->ope = REGISTER;

	printf("Input id:");
	scanf("%d",&(msg->staff.id) );
	getchar();
	
    printf("Input name:");
	scanf("%s",(msg->staff).name);
	getchar();

	printf("Input age:");
	scanf("%d",&(msg->staff.age) );
	getchar();

	printf("Input sex(F/M):");
	scanf("%c",&(msg->staff.sex) );
	getchar();

	printf("Input pwd:");
	scanf("%s",msg->staff.pwd);
	getchar();

	printf("Input phone_num(11):");
	scanf("%s",msg->staff.phone_num );
	getchar();

    printf("Input salary:");
	scanf("%f",&(msg->staff.salary) );
	getchar();

	printf("Input depart:");
	scanf("%s",msg->staff.department);
	getchar();

	if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
		return -1;
	}
	
	//用来输出和判断注册成功:
	if(recv(sockfd,msg->msg_ret,sizeof(unsigned char[128]),0) < 0){
        printf("fail to recv\n");
		return -1;
	}

	printf("%s\n",msg->msg_ret);

	return 0;
	
}


/******************************** 
* 名字 : do_login
* 功能 : 登入函数
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0 普通用户
             : 1 root用户
             : -1 无此用户
* 其他       : Null
*********************************/


int do_login(int sockfd,MSG *msg)
{
    int user_type = -1;

    msg->ope = LOGIN;
	
    printf("input name:");
	scanf("%s",msg->staff.name);
	getchar();

	//判断用户类型
	if(strcmp(msg->staff.name,ROOTNAME) == 0){
        user_type = 1;
	}else{
        user_type = 0;
	}

	printf("input passwd:");
	scanf("%s",msg->staff.pwd);
	getchar();

	printf("Input id:");
	scanf("%d",&(msg->staff.id) );
	getchar();

    //填写登入的名字和id
    login_id = &(msg->staff.id);
	strcpy(login_name,msg->staff.name);

	printf("login_id : %d, login_name : %s \n",*login_id,login_name);
	
	if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
		return -1;
	}

	if(recv(sockfd,msg->msg_ret,sizeof(unsigned char[128]),0) < 0){
        printf("fail to recv\n");
		return -1;
	}    
	
    //打印返回的消息
    printf("%s\n",msg->msg_ret);
    if(strncmp(msg->msg_ret,"Error",5) == 0){
		user_type = -1;
    }
	
	return user_type;
}


/******************************** 
* 名字        : do_login_select
* 功能 : root用户登入选项函数
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int root_login_select(int sockfd,MSG *msg)
{
    int n=-1;
	while(1){
        printf("*********************************< root >*************************************\n");		
		printf("* 1.add_user       2.del_user      3.query_user         4.query_all_user     *\n");
		printf("* 5.user_history   6.all_history   7.change_user_data   8.quit               *\n");
		printf("******************************************************************************\n");		
		printf("Please choose:");   

		scanf("%d",&n);
		getchar();

		switch(n){
            case ROOT_ADD:
				root_add_user(sockfd,msg); //添加用户
				break;
			case ROOT_DEL:
				root_del_user(sockfd,msg); //删除用户
				break;
			case ROOT_QUE_ONE:
				root_query_user_data(sockfd,msg);  //查询用户信息
				break; 
			case ROOT_QUE_ALL:
				root_query_all_user_data(sockfd,msg);  //查询所有用户信息
				break;
			case ROOT_HIS_ONE:
				root_history(sockfd,msg);  //历史记录
				break;
			case ROOT_HIS_ALL:
				root_history_all(sockfd,msg); //所有历史记录
				break;
			case ROOT_CHANGE:
				root_change_data(sockfd,msg); //更改数据
				break;
			case ROOT_QUT:
				return 1;    //退出
				break;
			default :
				printf("***input error\n");

		  }
		}		
	return 0;
	

}

/******************************** 
* 名字        : root_add_user
* 功能 : root添加用户
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int root_add_user(int sockfd,MSG *msg)
{
    do_register(sockfd,msg);
	return 0;
}

/******************************** 
* 名字        : root_del_user
* 功能 : root删除用户
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int root_del_user(int sockfd,MSG *msg)
{
    msg->ope = DELETE;
    
	printf("input id:");
		scanf("%d",&((msg->staff).id) );
		getchar();
		
    if(send(sockfd,msg,sizeof(MSG),0) < 0){
		printf("fail to send\n");
		return -1;
	}

	if(recv(sockfd,msg->msg_ret,sizeof(unsigned char[128]),0) < 0){
        printf("fail to recv\n");
		return -1;
	}	
 
    //打印返回的消息
    printf("%s\n",msg->msg_ret);

	return 0;
 	
}


/******************************** 
* 名字 : root_query_user_data
* 功能 : root查询用户信息
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int root_query_user_data(int sockfd,MSG *msg)
{
    msg->ope = SEARCH_ONESTAFF;

	printf("input id:");
	scanf("%d",&(msg->staff.id));
	getchar();
	
    printf("input name:");
	scanf("%s",msg->staff.name);
	getchar();

    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
		return -1;
	}

	if(recv(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to recv\n");
		return -1;
	}
		
    printf("id:%d name:%s age:%d sex:%c pwd:%s phone_num:%s salary:%f depart:%s\n",
			msg->staff.id,msg->staff.name,
	         msg->staff.age,msg->staff.sex,
	          msg->staff.pwd,msg->staff.phone_num,
	           msg->staff.salary,msg->staff.department);
	putchar(10);

    return 0;
}


/******************************** 
* 名字 : root_query_all_user_data
* 功能 : root查询所有用户信息
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int root_query_all_user_data(int sockfd,MSG *msg)
{
    msg->ope = SEARCH_ALL;

    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
		return -1;
	}

    while(1){
	    if(recv(sockfd,msg,sizeof(MSG),0) < 0){
            printf("fail to recv\n");
		    return -1;
	    }
		
		if(strncmp(msg->msg_ret,"###",3)==0){
            break;
		}
			
        printf("id:%d name:%s age:%d sex:%c pwd:%s phone_num:%s salary:%f depart:%s\n",
			msg->staff.id,msg->staff.name,
	         msg->staff.age,msg->staff.sex,
	          msg->staff.pwd,msg->staff.phone_num,
	           msg->staff.salary,msg->staff.department);
    }

    return 0;
}


/******************************** 
* 名字 : root_history
* 功能 : root用户历史记录查询
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int root_history(int sockfd,MSG *msg)
{
    msg->ope = HISTORY_THISSTAFF;

	printf("input id:");
	scanf("%d",&(msg->staff.id) );
	getchar();
	
    printf("input name:");
	scanf("%s",msg->staff.name);
	getchar();

	msg->staff.id = *login_id;
    strcpy(msg->staff.name,login_name);
	
    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
		return -1;
	}


	//接收服务器穿过来的历史记录消息
	while(1){
        recv(sockfd,msg,sizeof(MSG),0);
		
		if(strncmp(msg->msg_ret,"###",3)==0){
            break;
		}

		printf("id:%d name:%s%s\n",msg->staff.id,msg->staff.name,msg->msg_ret);
	}
	
	return 0;
	
}


int root_history_all(int sockfd,MSG *msg)
{
    msg->ope = HISTORY_ALL;

    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
		return -1;
	}

	//接收服务器穿过来的历史记录消息
	while(1){
        recv(sockfd,msg,sizeof(MSG),0);
		
		if(strncmp(msg->msg_ret,"###",3)==0){
            break;
		}

		printf("id:%d name:%s%s \n",msg->staff.id,msg->staff.name,msg->msg_ret);
	}
	
	return 0;    

}

int root_change_data(int sockfd,MSG *msg)
{
    //先查询到想要修改的数据结构体
	msg->ope = SEARCH_ONESTAFF;
	
	printf("input name:");
	scanf("%s",msg->staff.name);
	getchar();
	
	printf("input id:");
	scanf("%d",&(msg->staff.id));
	getchar();
	
	if(send(sockfd,msg,sizeof(MSG),0) < 0){
		printf("fail to send\n");
		return -1;
	}
	
	if(recv(sockfd,msg,sizeof(MSG),0) < 0){
		 printf("fail to recv\n");
		 return -1;
	}

    //输入新的数据	
    msg->ope = MODIFY_ALL_EXCEPT_ID;
    printf("Input name:");
	scanf("%s",(msg->staff).name);
	getchar();

	printf("Input age:");
	scanf("%d",&(msg->staff.age) );
	getchar();

	printf("Input sex(F/M):");
	scanf("%c",&(msg->staff.sex) );
	getchar();

	printf("Input pwd:");
	scanf("%s",msg->staff.pwd);
	getchar();

	printf("Input phone_num(11):");
	scanf("%s",msg->staff.phone_num );
	getchar();

    printf("Input salary:");
	scanf("%f",&(msg->staff.salary) );
	getchar();

	printf("Input depart:");
	scanf("%s",msg->staff.department);
	getchar();

	if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
		return -1;
	}
	
	//用来输出和判断写入成功:
	if(recv(sockfd,msg->msg_ret,sizeof(unsigned char[128]),0) < 0){
        printf("fail to recv\n");
		return -1;
	}

	printf("%s\n",msg->msg_ret);

	return 0;
	
}


/******************************** 
* 名字 : user_login_select
* 功能 : 普通用户功能选项
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/
int user_login_select(int sockfd,MSG *msg)
{
    int n=-1;
 	while(1){
        printf("***********************< user >*********************\n");		
		printf("* 1.query_data  2.change_data  3.history  4.quit   *\n");		
		printf("****************************************************\n");		
		printf("Please choose:");   

		scanf("%d",&n);
		getchar();

		switch(n){
            case USER_QUE:
				do_query(sockfd,msg);
				break;
			case USER_CHA:
				do_change_data(sockfd,msg);
				break;
			case USER_HIS:
				do_history(sockfd,msg);
				break;
			case USER_QUT:
				return 1;
				break;
			default :
				printf("***input error\n");

		}
	}
	
	return 0;
}


/******************************** 
* 名字 : do_query
* 功能 : 普通用户查询功能
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/


int do_query(int sockfd,MSG *msg)
{
	msg->ope = SEARCH_ONESTAFF;

    msg->staff.id = *login_id;
    strcpy(msg->staff.name,login_name);

	printf("msg->staff.id : %d ,msg->staff.name : %s\n",msg->staff.id,msg->staff.name);

    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
	    return -1;
	}

	if(recv(sockfd,msg,sizeof(MSG),0) < 0){
         printf("fail to recv\n");
		 return -1;
	}
		
    printf("id:%d name:%s age:%d sex:%c pwd:%s phone_num:%s salary:%f depart:%s\n",
			msg->staff.id,msg->staff.name,
	         msg->staff.age,msg->staff.sex,
	          msg->staff.pwd,msg->staff.phone_num,
	           msg->staff.salary,msg->staff.department);

    return 0;
}


/******************************** 
* 名字 : do_change_data
* 功能 : 普通用户更改数据
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int do_change_data(int sockfd,MSG *msg)
{
    int n=-1;

        printf("***************/change select/***********************\n");		
        printf("* 1.pwd  2.phone_num 3.age 4.salary 5.depart 6.quit *\n");		
	    printf("*****************************************************\n");			
	    printf("Please choose:");   

	    scanf("%d",&n);
		getchar();

	    switch(n){
            case USER_MOD_PWD:
				msg->ope = MODIFY_PWD;
			    printf("input new pwd:");
			    scanf("%s",msg->staff.pwd);
			    getchar();
			    break;
		    case USER_MOD_PHONE:
				msg->ope = MODIFY_PHONE_NUM;
		   	    printf("input new phone number:");
			    scanf("%s",msg->staff.phone_num);
			    getchar();
			    break;
		    case USER_MOD_AGE:
				msg->ope = MODIFY_AGE;
			    printf("input new age:");
			    scanf("%d",&(msg->staff.age));
			    getchar();
			    break; 
		    case USER_MOD_SALARY:
				msg->ope = MODIFY_SALARY;
			    printf("input new salary:");
			    scanf("%f",&(msg->staff.salary));
		    	getchar();
			    break;
		    case USER_MOD_DEPART:
				msg->ope = MODIFY_DEPARTMENT;
		    	printf("input new depart:");
    	    	scanf("%s",msg->staff.department);
		    	getchar();
			    break;
		    case USER_MOD_QUIT:
		    	return 0;            //退出
		    	break;
		    default :
			    printf("***error\n");

	    }
		
    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
		return -1;
	}

    if(recv(sockfd,msg->msg_ret,sizeof(unsigned char[128]),0) < 0){
        printf("fail to recv\n");
        return -1;
	}

    //打印返回的消息
    printf("%s\n",msg->msg_ret);
    return 0;

}


/******************************** 
* 名字 : do_history
* 功能 : 普通用户历史记录查询
* 参数:       
*      a : sockfd 
*      b : msg
* 返回值     : 0
* 其他       : Null
*********************************/

int do_history(int sockfd,MSG *msg)
{
    msg->ope = HISTORY_THISSTAFF;

	msg->staff.id = *login_id;
    strcpy(msg->staff.name,login_name);
	
    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        printf("fail to send\n");
		return -1;
	}


	//接收服务器穿过来的历史记录消息
	while(1){
        recv(sockfd,msg,sizeof(MSG),0);
		
		if(strncmp(msg->msg_ret,"###",3)==0){
            break;
		}

		printf("id:%d name:%s%s\n",msg->staff.id,msg->staff.name,msg->msg_ret);
	}
	
	return 0;
	
}





