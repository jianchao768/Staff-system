#ifndef	__STAFF_V0_H__
#define	__STAFF_V0_H__
#include	<sqlite3.h>
#define	STAFSYSTEM_DB	"StaffSystem.db"
#define	ERRNUM	50
typedef	struct
{
	int	id;	//工号
	unsigned	char	name[20];	//姓名
	int	age;	//年龄
	unsigned	char	sex;	//性别('F':女,'M':男)
	unsigned	char	pwd[20];	//密码
	unsigned	char	phone_num[11];	//电话号码(11位)	
	float	salary;	//工资
	unsigned	char	department[20];	//部门
}__attribute__((packed))	STAFF,*PSTAFF;	//员工信息结构体
typedef	enum	
{
	REGISTER=0,	//注册	
	LOGIN,	//登录
	INSERT,	//增
	DELETE,	//删
	MODIFY_PWD,	//修改密码
	MODIFY_PHONE_NUM,	//修改手机号
	MODIFY_AGE,	//修改年龄
	MODIFY_SALARY,	//修改薪资
	MODIFY_DEPARTMENT,	//修改部门
	MODIFY_ALL_EXCEPT_ID,	//修改除工号外的所有信息
	SEARCH_ONESTAFF,	//查询某个员工的信息
	SEARCH_ALL,	//查询所有员工的信息
	HISTORY_THISSTAFF,	//查询某个员工的历史记录
	HISTORY_ALL	//查询所有员工的历史记录
}OPERATE;	//操作
typedef	struct
{
	OPERATE	ope;	//操作类型
	STAFF	staff;	//员工信息结构体对象
	unsigned	char	msg_ret[128];	//返回消息
}__attribute__((packed))	MSG,*PMSG;	//消息结构体
typedef	struct
{
	int	fd_acce;	//已与客户端建立链接的套接字
	struct	sockaddr_in	addr_cli;	//客户端IP地址
	sqlite3	*pDb;	//sqlite3数据库的句柄
}__attribute__((packed))	PARAM_THREAD,*PPARAM_THREAD;	//给处理线程传递参数的结构体
#endif