#ifndef __HEAD_H__
#define __HEAD_H__

#define ROOTNAME "root"

//基础信息结构体
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


//选项宏定义
#define HOME_REG    1
#define HOME_LOGIN  2
#define HOME_QUT    3

#define ROOT_ADD 1
#define ROOT_DEL 2
#define ROOT_QUE_ONE 3
#define ROOT_QUE_ALL 4
#define ROOT_HIS_ONE 5
#define ROOT_HIS_ALL 6
#define ROOT_CHANGE 7
#define ROOT_QUT 8

#define USER_QUE 1
#define USER_CHA 2
#define USER_HIS 3
#define USER_QUT 4

#define USER_MOD_PWD 1
#define USER_MOD_PHONE 2
#define USER_MOD_AGE 3
#define USER_MOD_SALARY 4
#define USER_MOD_DEPART 5
#define USER_MOD_QUIT 6


#endif


