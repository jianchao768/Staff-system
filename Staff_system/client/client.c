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


//��¼��ǰ�����˻�����Ϣ
int * login_id;
unsigned char login_name[20];

//��������
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
* ���� : main
* ���� : ������
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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

	//�½�ipv4�����׽���
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        perror("fail to socket\n");
		return -1;
	}

	//�½�������id��Ϣ�����
	struct sockaddr_in serveraddr;
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));

    //����
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
			case HOME_LOGIN:   //���do_login����ֵΪ1������root�û�����������ͨ�û�,������Ϊ-1��Ϊע��
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
* ���� : do_register
* ���� : register procedure
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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
	
	//����������ж�ע��ɹ�:
	if(recv(sockfd,msg->msg_ret,sizeof(unsigned char[128]),0) < 0){
        printf("fail to recv\n");
		return -1;
	}

	printf("%s\n",msg->msg_ret);

	return 0;
	
}


/******************************** 
* ���� : do_login
* ���� : ���뺯��
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0 ��ͨ�û�
             : 1 root�û�
             : -1 �޴��û�
* ����       : Null
*********************************/


int do_login(int sockfd,MSG *msg)
{
    int user_type = -1;

    msg->ope = LOGIN;
	
    printf("input name:");
	scanf("%s",msg->staff.name);
	getchar();

	//�ж��û�����
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

    //��д��������ֺ�id
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
	
    //��ӡ���ص���Ϣ
    printf("%s\n",msg->msg_ret);
    if(strncmp(msg->msg_ret,"Error",5) == 0){
		user_type = -1;
    }
	
	return user_type;
}


/******************************** 
* ����        : do_login_select
* ���� : root�û�����ѡ���
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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
				root_add_user(sockfd,msg); //����û�
				break;
			case ROOT_DEL:
				root_del_user(sockfd,msg); //ɾ���û�
				break;
			case ROOT_QUE_ONE:
				root_query_user_data(sockfd,msg);  //��ѯ�û���Ϣ
				break; 
			case ROOT_QUE_ALL:
				root_query_all_user_data(sockfd,msg);  //��ѯ�����û���Ϣ
				break;
			case ROOT_HIS_ONE:
				root_history(sockfd,msg);  //��ʷ��¼
				break;
			case ROOT_HIS_ALL:
				root_history_all(sockfd,msg); //������ʷ��¼
				break;
			case ROOT_CHANGE:
				root_change_data(sockfd,msg); //��������
				break;
			case ROOT_QUT:
				return 1;    //�˳�
				break;
			default :
				printf("***input error\n");

		  }
		}		
	return 0;
	

}

/******************************** 
* ����        : root_add_user
* ���� : root����û�
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
*********************************/

int root_add_user(int sockfd,MSG *msg)
{
    do_register(sockfd,msg);
	return 0;
}

/******************************** 
* ����        : root_del_user
* ���� : rootɾ���û�
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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
 
    //��ӡ���ص���Ϣ
    printf("%s\n",msg->msg_ret);

	return 0;
 	
}


/******************************** 
* ���� : root_query_user_data
* ���� : root��ѯ�û���Ϣ
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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
* ���� : root_query_all_user_data
* ���� : root��ѯ�����û���Ϣ
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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
* ���� : root_history
* ���� : root�û���ʷ��¼��ѯ
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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


	//���շ���������������ʷ��¼��Ϣ
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

	//���շ���������������ʷ��¼��Ϣ
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
    //�Ȳ�ѯ����Ҫ�޸ĵ����ݽṹ��
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

    //�����µ�����	
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
	
	//����������ж�д��ɹ�:
	if(recv(sockfd,msg->msg_ret,sizeof(unsigned char[128]),0) < 0){
        printf("fail to recv\n");
		return -1;
	}

	printf("%s\n",msg->msg_ret);

	return 0;
	
}


/******************************** 
* ���� : user_login_select
* ���� : ��ͨ�û�����ѡ��
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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
* ���� : do_query
* ���� : ��ͨ�û���ѯ����
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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
* ���� : do_change_data
* ���� : ��ͨ�û���������
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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
		    	return 0;            //�˳�
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

    //��ӡ���ص���Ϣ
    printf("%s\n",msg->msg_ret);
    return 0;

}


/******************************** 
* ���� : do_history
* ���� : ��ͨ�û���ʷ��¼��ѯ
* ����:       
*      a : sockfd 
*      b : msg
* ����ֵ     : 0
* ����       : Null
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


	//���շ���������������ʷ��¼��Ϣ
	while(1){
        recv(sockfd,msg,sizeof(MSG),0);
		
		if(strncmp(msg->msg_ret,"###",3)==0){
            break;
		}

		printf("id:%d name:%s%s\n",msg->staff.id,msg->staff.name,msg->msg_ret);
	}
	
	return 0;
	
}





