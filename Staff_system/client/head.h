#ifndef __HEAD_H__
#define __HEAD_H__

#define ROOTNAME "root"

//������Ϣ�ṹ��
typedef	struct
{
	int	id;	//����
	unsigned	char	name[20];	//����
	int	age;	//����
	unsigned	char	sex;	//�Ա�('F':Ů,'M':��)
	unsigned	char	pwd[20];	//����
	unsigned	char	phone_num[11];	//�绰����(11λ)	
	float	salary;	//����
	unsigned	char	department[20];	//����
}__attribute__((packed))	STAFF,*PSTAFF;	//Ա����Ϣ�ṹ��


typedef	enum	
{
	REGISTER=0,	//ע��	
	LOGIN,	//��¼
	INSERT,	//��
	DELETE,	//ɾ
	MODIFY_PWD,	//�޸�����
	MODIFY_PHONE_NUM,	//�޸��ֻ���
	MODIFY_AGE,	//�޸�����
	MODIFY_SALARY,	//�޸�н��
	MODIFY_DEPARTMENT,	//�޸Ĳ���
	MODIFY_ALL_EXCEPT_ID,	//�޸ĳ��������������Ϣ
	SEARCH_ONESTAFF,	//��ѯĳ��Ա������Ϣ
	SEARCH_ALL,	//��ѯ����Ա������Ϣ
	HISTORY_THISSTAFF,	//��ѯĳ��Ա������ʷ��¼
	HISTORY_ALL	//��ѯ����Ա������ʷ��¼
}OPERATE;	//����


typedef	struct
{
	OPERATE	ope;	//��������
	STAFF	staff;	//Ա����Ϣ�ṹ�����
	unsigned	char	msg_ret[128];	//������Ϣ
}__attribute__((packed))	MSG,*PMSG;	//��Ϣ�ṹ��


//ѡ��궨��
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


