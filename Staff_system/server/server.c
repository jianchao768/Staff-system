#include	<stdio.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<string.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<netinet/in.h>
#include	<time.h>
#include	"staff.h"
/*
功能:获得当前时间
参数:tm_GW:指向存放当前时间的字符串的指针
返回值:无
*/
void	get_time(char	*tm_GW)
{
	time_t	t;
	time(&t);
	struct	tm	*tp=localtime(&t);
	sprintf(tm_GW,"%d-%d-%d	%d:%d:%d",tp->tm_year+1900,tp->tm_mon+1,tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec);
}
/*
功能:以阻塞方式发送信息
参数:fd:已与客户端建立链接的文件描述符;buf:指向要发送的内容的指针;len:要发送的内容的长度(单位:字节);p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	send_info(int	fd,const	void	*buf,size_t	len,int	*p_cnt_err)
{
	int	ret_send;
	do
	{
		ret_send=send(fd,buf,len,0);
	}while((ret_send<0)&&(errno==EINTR));
	if(ret_send<0)
	{
		perror("send error");
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else	if(!ret_send)	printf("Client %d is closed.\n",fd);
}
/*
功能:向员工历史记录表添加历史记录
参数:p_sql3:sqlite3数据库的句柄;p_staff:指向进行操作的员工信息结构体的指针;p_cnt_err:指向代表发生错误次数的指针;p_str_opera:指向代表操作信息内容的字符串的指针
返回值:无
*/
void	add_history(sqlite3	*p_sql3,PSTAFF	p_staff,int	*p_cnt_err,char	*p_str_opera)
{
	char	time_finish[30];
	get_time(time_finish);
	char	str_cmd[128];
	sprintf(str_cmd,"Insert into staff_history values(\"%s\",%d,\"%s\",\"%s\")",time_finish,p_staff->id,p_staff->name,p_str_opera);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_sql3,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else	sqlite3_free_table(result);
}
/*
功能:注册用户
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_register(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Select * from staff_info where id=%d;",(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		if((!row)&&(!column))
		{
			sprintf(str_cmd,"Insert into staff_info values(%d,\"%s\",%d,\"%c\",\"%s\",\"%s\",%lf,\"%s\");",(p_msg->staff).id,(p_msg->staff).name,(p_msg->staff).age,(p_msg->staff).sex,(p_msg->staff).pwd,(p_msg->staff).phone_num,(p_msg->staff).salary,(p_msg->staff).department);
			if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
			{
				printf("%s\n",msg_err);
				if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
			}
			else	
			{
				sqlite3_free_table(result);
				sprintf(p_msg->msg_ret,"Register this staff succeed.\n");
				send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
			}
			add_history(p_arg->pDb,&(p_msg->staff),p_cnt_err,"Register");
		}
		else
		{
			sprintf(p_msg->msg_ret,"This staff already exists.\n");
			send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		}
	}
}
/*
功能:登录
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_login(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Select * from staff_info where name=\"%s\" and pwd=\"%s\" and id=\"%d\";",(p_msg->staff).name,(p_msg->staff).pwd,(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		sprintf(p_msg->msg_ret,(!row)?"Error\n":"Welcome\n");
		send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		add_history(p_arg->pDb,&(p_msg->staff),p_cnt_err,"Login");
	}
}
/*
功能:添加用户
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_insert(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Select * from staff_info where id=%d;",(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		if((!row)&&(!column))
		{
			sprintf(str_cmd,"Insert into staff_info values(%d,\"%s\",%d,\"%c\",\"%s\",\"%s\",%lf,\"%s\");",(p_msg->staff).id,(p_msg->staff).name,(p_msg->staff).age,(p_msg->staff).sex,(p_msg->staff).pwd,(p_msg->staff).phone_num,(p_msg->staff).salary,(p_msg->staff).department);
			if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
			{
				printf("%s\n",msg_err);
				if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
			}
			else	
			{
				sqlite3_free_table(result);
				sprintf(p_msg->msg_ret,"Insert this staff succeed.\n");
				send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
				add_history(p_arg->pDb,&(p_msg->staff),p_cnt_err,"Insert");
			}
		}
		else
		{
			sprintf(p_msg->msg_ret,"This staff already exists.\n");
			send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		}
	}
}
/*
功能:删除用户
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_delete(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Select * from staff_info where id=%d;",(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		if((!row)&&(!column))	
		{
			sprintf(p_msg->msg_ret,"This staff doesn\'t exist.\n");
			send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		}
		else
		{
			sprintf(str_cmd,"Delete from staff_info where id=%d;",(p_msg->staff).id);
			if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
			{
				printf("%s\n",msg_err);
				if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
			}
			else	
			{
				sqlite3_free_table(result);
				sprintf(p_msg->msg_ret,"This staff has been deleted succeed.\n");
				send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
				add_history(p_arg->pDb,&(p_msg->staff),p_cnt_err,"Delete");
			}
		}
	}
}
/*
功能:修改用户密码
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_modify_pwd(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Update staff_info set pwd=\"%s\" where name=\"%s\" and id=%d;",(p_msg->staff).pwd,(p_msg->staff).name,(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		sprintf(p_msg->msg_ret,"This staff\'s password has been modified succeed.\n");
		send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		add_history(p_arg->pDb,&(p_msg->staff),p_cnt_err,"Modify_Pwd");
	}
}
/*
功能:修改用户手机号
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_modify_phone_num(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Update staff_info set phone_num=\"%s\" where name=\"%s\" and id=%d;",(p_msg->staff).phone_num,(p_msg->staff).name,(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		sprintf(p_msg->msg_ret,"This staff\'s phone num has been modified succeed.\n");
		send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		add_history(p_arg->pDb,&(p_msg->staff),p_cnt_err,"Modify_PhoneNum");
	}
}
/*
功能:修改用户年龄
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_modify_age(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Update staff_info set age=%d where name=\"%s\" and id=%d;",(p_msg->staff).age,(p_msg->staff).name,(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		sprintf(p_msg->msg_ret,"This staff\'s age has been modified succeed.\n");
		send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		add_history(p_arg->pDb,&(p_msg->staff),p_cnt_err,"Modify_Age");
	}
}
/*
功能:修改用户薪资
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_modify_salary(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Update staff_info set salary=%d where name=\"%s\" and id=%d;",(p_msg->staff).salary,(p_msg->staff).name,(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		sprintf(p_msg->msg_ret,"This staff\'s salary has been modified succeed.\n");
		send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		add_history(p_arg->pDb,&(p_msg->staff),p_cnt_err,"Modify_Salary");
	}
}
/*
功能:修改用户部门
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_modify_department(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Update staff_info set department=\"%s\" where name=\"%s\" and id=%d;",(p_msg->staff).department,(p_msg->staff).name,(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		sprintf(p_msg->msg_ret,"This staff\'s salary has been modified succeed.\n");
		send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		add_history(p_arg->pDb,&(p_msg->staff),p_cnt_err,"Modify_Department");
	}
}
/*
功能:
参数:
返回值:
*/
void	set_root_info(PSTAFF	p_staff)
{
	p_staff->id=0;
	strcpy(p_staff->name,"root");
}
/*
功能:修改用户除工号外的所有信息
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_modify_all_except_id(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Update staff_info set name=\"%s\",age=%d,sex=\"%c\",pwd=\"%s\",phone_num=\"%s\",salary=%lf,department=\"%s\" where id=%d;",(p_msg->staff).name,(p_msg->staff).age,(p_msg->staff).sex,(p_msg->staff).pwd,(p_msg->staff).phone_num,(p_msg->staff).salary,(p_msg->staff).department,(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		sqlite3_free_table(result);
		sprintf(p_msg->msg_ret,"This staff information has been modified succeed.\n");
		send_info(p_arg->fd_acce,p_msg->msg_ret,sizeof(unsigned	char[128]),p_cnt_err);
		STAFF	root;
		set_root_info(&root);
		add_history(p_arg->pDb,&root,p_cnt_err,"Modify_All_Except_Id");
	}
}
/*
功能:查询某个员工的信息
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_search_onestaff(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Select * from staff_info where name=\"%s\" and id=%d;",(p_msg->staff).name,(p_msg->staff).id);
	char	**result;
	int	row,column;
	char	*msg_err;
	if(sqlite3_get_table(p_arg->pDb,str_cmd,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("%s\n",msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	else
	{
		if((!row)&&(!column))	sprintf(p_msg->msg_ret,"This staff doesn\'t exist.\n");
		else
		{
			(p_msg->staff).age=atoi(result[column+2]);
			(p_msg->staff).sex=result[column+3][0];
			strcpy((p_msg->staff).pwd,result[column+4]);
			strcpy((p_msg->staff).phone_num,result[column+5]);
			(p_msg->staff).salary=atof(result[column+6]);
			strcpy((p_msg->staff).department,result[column+7]);
			sprintf(p_msg->msg_ret,"This staff has been found.\n");
		}
		sqlite3_free_table(result);
		send_info(p_arg->fd_acce,p_msg,sizeof(MSG),p_cnt_err);
		STAFF	root;
		set_root_info(&root);
		add_history(p_arg->pDb,&root,p_cnt_err,"Search_OneStaff");
	}
}
typedef	struct
{
	int	fd_acce;	//已与客户端建立链接的套接字
	int	*p_cnt_err;	//指向代表发生错误次数的指针
}TMP_CALLBACK,*P_TMP_CALLBACK;	//回调函数所用的临时结构体
/*
功能:函数do_search_all所使用的回调函数
参数:para:传递给回调函数的参数;f_num:表的列数;f_value:查询结果的指针数组;f_name:指向字段名的指针数组
返回值:0
*/
int	callback_func_for_search(void	*para,int	f_num,char	**f_value,char	**f_name)
{
	int	fd_acce=((P_TMP_CALLBACK)para)->fd_acce;
	MSG	msg_snd;
	(msg_snd.staff).id=atoi(f_value[0]);
	strcpy((msg_snd.staff).name,f_value[1]);
	(msg_snd.staff).age=atoi(f_value[2]);
	(msg_snd.staff).sex=f_value[3][0];
	strcpy((msg_snd.staff).pwd,f_value[4]);
	strcpy((msg_snd.staff).phone_num,f_value[5]);
	(msg_snd.staff).salary=atof(f_value[6]);
	strcpy((msg_snd.staff).department,f_value[7]);
	strcpy(msg_snd.msg_ret,"***");
	send_info(fd_acce,&msg_snd,sizeof(MSG),((P_TMP_CALLBACK)para)->p_cnt_err);
	return	0;
}
/*
功能:查询所有员工的信息
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_search_all(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	strcpy(str_cmd,"Select * from staff_info;");
	TMP_CALLBACK	v_tmpCB;
	v_tmpCB.fd_acce=(p_arg->fd_acce);
	v_tmpCB.p_cnt_err=p_cnt_err;
	char	*p_msg_err;
	if(sqlite3_exec(p_arg->pDb,str_cmd,callback_func_for_search,(void	*)&v_tmpCB,&p_msg_err)!=SQLITE_OK)
	{
		printf("sqlite3_exec error:%s\n",p_msg_err);
		sqlite3_free(p_msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	MSG	msg_tmp;
	strcpy(msg_tmp.msg_ret,"###");
	send_info(p_arg->fd_acce,&msg_tmp,sizeof(MSG),p_cnt_err);
	STAFF	root;
	set_root_info(&root);
	add_history(p_arg->pDb,&root,p_cnt_err,"Search_All");
}
/*
功能:函数do_history_thisstaff和do_history_all所使用的回调函数
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:0
*/
int	callback_func_for_history(void	*para,int	f_num,char	**f_value,char	**f_name)
{
	int	fd_acce=((P_TMP_CALLBACK)para)->fd_acce;
	MSG	msg_snd;
	(msg_snd.staff).id=atoi(f_value[1]);
	strcpy((msg_snd.staff).name,f_value[2]);
	sprintf(msg_snd.msg_ret,"%s	%s",f_value[0],f_value[3]);
	send_info(fd_acce,&msg_snd,sizeof(MSG),((P_TMP_CALLBACK)para)->p_cnt_err);
	return	0;
}
/*
功能:查询某个员工的历史记录
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_history_thisstaff(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	sprintf(str_cmd,"Select * from staff_history where id=%d and name=\"%s\";",(p_msg->staff).id,(p_msg->staff).name);
	TMP_CALLBACK	v_tmpCB;
	v_tmpCB.fd_acce=(p_arg->fd_acce);
	v_tmpCB.p_cnt_err=p_cnt_err;
	char	*p_msg_err;
	if(sqlite3_exec(p_arg->pDb,str_cmd,callback_func_for_history,(void	*)&v_tmpCB,&p_msg_err)!=SQLITE_OK)
	{
		printf("sqlite3_exec error:%s\n",p_msg_err);
		sqlite3_free(p_msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	MSG	msg_tmp;
	strcpy(msg_tmp.msg_ret,"###");
	send_info(p_arg->fd_acce,&msg_tmp,sizeof(MSG),p_cnt_err);
	STAFF	root;
	set_root_info(&root);
	add_history(p_arg->pDb,&root,p_cnt_err,"History_ThisStaff");
}
/*
功能:查询所有员工的历史记录
参数:p_arg:指向给处理线程传递参数的结构体的指针;p_msg:指向消息结构体的指针;p_cnt_err:指向代表发生错误次数的指针
返回值:无
*/
void	do_history_all(PPARAM_THREAD	p_arg,PMSG	p_msg,int	*p_cnt_err)
{
	char	str_cmd[128];
	strcpy(str_cmd,"Select * from staff_history;");
	TMP_CALLBACK	v_tmpCB;
	v_tmpCB.fd_acce=(p_arg->fd_acce);
	v_tmpCB.p_cnt_err=p_cnt_err;
	char	*p_msg_err;
	if(sqlite3_exec(p_arg->pDb,str_cmd,callback_func_for_history,(void	*)&v_tmpCB,&p_msg_err)!=SQLITE_OK)
	{
		printf("sqlite3_exec error:%s\n",p_msg_err);
		sqlite3_free(p_msg_err);
		if((*p_cnt_err)<ERRNUM)	(*p_cnt_err)++;
	}
	MSG	msg_tmp;
	strcpy(msg_tmp.msg_ret,"###");
	send_info(p_arg->fd_acce,&msg_tmp,sizeof(MSG),p_cnt_err);
	STAFF	root;
	set_root_info(&root);
	add_history(p_arg->pDb,&root,p_cnt_err,"History_All");
}
/*
功能:处理线程的线程入口函数
参数:arg:指向给处理线程传递参数的结构体的指针
返回值:NULL
*/
void	*thread_routine(void	*arg)
{
	if(pthread_detach(pthread_self()))
	{
		perror("pthread_detach error");
		return	NULL;
	}
	MSG	msg;
	PPARAM_THREAD	p_arg=(PPARAM_THREAD)arg;
	int	byte_recv;
	while(1)
	{
		static	int	cnt_err=0;
		memset(&msg,0,sizeof(MSG));
		byte_recv=recv(p_arg->fd_acce,&msg,sizeof(MSG),0);
		if(byte_recv<=0)
		{
			if(byte_recv<0)
			{
				perror("recv error");
				if(cnt_err<ERRNUM)	cnt_err++;
			}
			else	
			{
				printf("客户端%d已关闭!\n",p_arg->fd_acce);
				break;
			}
		}
		switch(msg.ope)
		{
			case	REGISTER:do_register(p_arg,&msg,&cnt_err);break;
			case	LOGIN:do_login(p_arg,&msg,&cnt_err);break;
			case	INSERT:do_insert(p_arg,&msg,&cnt_err);break;
			case	DELETE:do_delete(p_arg,&msg,&cnt_err);break;
			case	MODIFY_PWD:do_modify_pwd(p_arg,&msg,&cnt_err);break;
			case	MODIFY_PHONE_NUM:do_modify_phone_num(p_arg,&msg,&cnt_err);break;
			case	MODIFY_AGE:do_modify_age(p_arg,&msg,&cnt_err);break;
			case	MODIFY_SALARY:do_modify_salary(p_arg,&msg,&cnt_err);break;
			case	MODIFY_DEPARTMENT:do_modify_department(p_arg,&msg,&cnt_err);break;
			case	MODIFY_ALL_EXCEPT_ID:do_modify_all_except_id(p_arg,&msg,&cnt_err);break;
			case	SEARCH_ONESTAFF:do_search_onestaff(p_arg,&msg,&cnt_err);break;
			case	SEARCH_ALL:do_search_all(p_arg,&msg,&cnt_err);break;
			case	HISTORY_THISSTAFF:do_history_thisstaff(p_arg,&msg,&cnt_err);break;
			case	HISTORY_ALL:do_history_all(p_arg,&msg,&cnt_err);break;
		}
		if(cnt_err==ERRNUM)
		{
			cnt_err=0;
			break;
		}
	}
	if(shutdown(p_arg->fd_acce,SHUT_RDWR)==-1)	perror("shutdown error");
	return	NULL;
}
/*
功能:主函数
参数:argv[1]:IP地址,argv[2]:端口号,argv[3]:要监听的套接字的个数
返回值:成功:0,失败:-1
*/
int	main(int	argc,const	char	*argv[])
{
	int	ret_m;
	//校验输入
	if(argc!=4)
	{
		ret_m=-1;
		goto	ERR_STP0;
	}
	int	port=atoi(argv[2]);
	if((port<1024)||(port>49151))
	{
		ret_m=-1;
		goto	ERR_STP0;
	}
	int	num=atoi(argv[3]);
	if((num<=0)||(num>1000))
	{
		ret_m=-1;
		goto	ERR_STP0;
	}
	//数据库相关初始化
	sqlite3	*pSqlite3;
	if(sqlite3_open(STAFSYSTEM_DB,&pSqlite3)!=SQLITE_OK)
	{
		char	*str_err=sqlite3_errmsg(pSqlite3);
		printf("sqlite3_open	error:%s\n",str_err);
		sqlite3_free(str_err);
		ret_m=-1;
		goto	ERR_STP0;
	}
	char	*msg_err;
	if(sqlite3_exec(pSqlite3,"create table if not exists staff_history(time TEXT,id INTEGER,name TEXT,operate TEXT)",NULL,NULL,&msg_err)!=SQLITE_OK)
	{
		printf("sqlite3_exec for create table staff_history error:%s\n",msg_err);
		ret_m=-1;
		goto	ERR_STP1;
	}
	if(sqlite3_exec(pSqlite3,"create table if not exists staff_info(id INTEGER Primary Key,name TEXT,age INTEGER,sex TEXT,pwd TEXT,phone_num TEXT,salary REAL,department TEXT)",NULL,NULL,&msg_err)!=SQLITE_OK)
	{
		printf("sqlite3_exec for create table staff_info error:%s\n",msg_err);
		ret_m=-1;
		goto	ERR_STP1;
	}
	//root用户的增加
	char	str_root[128];
	sprintf(str_root,"select * from staff_info where name=\"root\";");
	char	**result;
	int	row,column;
	if(sqlite3_get_table(pSqlite3,str_root,&result,&row,&column,&msg_err)!=SQLITE_OK)
	{
		printf("sqlite3_get_table error:%s\n",msg_err);
		ret_m=-1;
		goto	ERR_STP1;
	}
	else
	{
		if((!row)&&(!column))
		{
			sprintf(str_root,"insert into staff_info values(0,\"root\",99,\"M\",\"123456\",\"99999999999\",9999,\"department\");");
			if(sqlite3_get_table(pSqlite3,str_root,&result,&row,&column,&msg_err)!=SQLITE_OK)
			{
				printf("sqlite3_get_table error:%s\n",msg_err);
				ret_m=-1;
				goto	ERR_STP1;
			}
		}
	}
	//创建套接字
	int	fd_ser_sock=socket(AF_INET,SOCK_STREAM,0);
	if(fd_ser_sock==-1)
	{
		perror("socket error");
		ret_m=-1;
		goto	ERR_STP1;
	}
	int	b_reuse;
	//设置地址可重用
	if(setsockopt(fd_ser_sock,SOL_SOCKET,SO_REUSEADDR,&b_reuse,sizeof(b_reuse))==-1)
	{
		perror("setsockopt error");
		ret_m=-1;
		goto	ERR_STP2;
	}
	//服务器地址及端口号设置
	struct	sockaddr_in	addr_ser;
	bzero(&addr_ser,sizeof(addr_ser));
	addr_ser.sin_family=AF_INET;
	addr_ser.sin_addr.s_addr=inet_addr(argv[1]);
	addr_ser.sin_port=htons(port);
	//绑定
	if(bind(fd_ser_sock,(struct	sockaddr	*)&addr_ser,sizeof(addr_ser))==-1)
	{
		perror("bind error");
		ret_m=-1;
		goto	ERR_STP2;
	}
	//监听
	if(listen(fd_ser_sock,num)==-1)
	{
		perror("listen error");
		ret_m=-1;
		goto	ERR_STP2;
	}
	int	len_addr_cli=sizeof(struct	sockaddr_in);
	PARAM_THREAD	param_thread;
	param_thread.pDb=pSqlite3;
	pthread_t	tid;
	int	fd_tmp;
	while(1)
	{
		static	int	cnt_err=0;
		//建立连接
		fd_tmp=accept(fd_ser_sock,(struct	sockaddr	*)&(param_thread.addr_cli),&len_addr_cli);
		if(fd_tmp==-1)
		{
			perror("accept error");
			if(cnt_err<ERRNUM)	cnt_err++;
		}
		else
		{
			char	str_addr_cli[16];
			if(!inet_ntop(AF_INET,&(param_thread.addr_cli.sin_addr.s_addr),str_addr_cli,16))
			{
				perror("inet_ntop error");
				if(cnt_err<ERRNUM)	cnt_err++;
			}
			else
			{
				param_thread.fd_acce=fd_tmp;
				printf("客户端%d(%s:%d)已连接!\n",param_thread.fd_acce,str_addr_cli,ntohs(param_thread.addr_cli.sin_port));
				//创建服务子线程
				if(pthread_create(&tid,NULL,thread_routine,&param_thread))
				{
					perror("pthread_create error");
					if(cnt_err<ERRNUM)	cnt_err++;
				}
			}
		}
		if(cnt_err==ERRNUM)
		{
			cnt_err=0;
			break;
		}
	}
	if(close(fd_ser_sock)==-1)	perror("close error");
	if(sqlite3_close(pSqlite3)!=SQLITE_OK)
	{
		char	*str_err=sqlite3_errmsg(pSqlite3);
		printf("sqlite3_close error:%s\n",str_err);
		sqlite3_free(str_err);
	}
	return	0;
ERR_STP2:
	if(close(fd_ser_sock)==-1)	perror("close error");
ERR_STP1:
	if(sqlite3_close(pSqlite3)!=SQLITE_OK)
	{
		char	*str_err=sqlite3_errmsg(pSqlite3);
		printf("sqlite3_close error:%s\n",str_err);
		sqlite3_free(str_err);
	}
ERR_STP0:
	return	ret_m;
}