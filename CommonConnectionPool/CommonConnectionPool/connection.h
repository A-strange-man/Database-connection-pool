#pragma once
/*
ʵ��MYSQL���ݵĲ���
*/

#include "mysql.h"
#include <string>
#include <ctime>

class Connection
{
public:
	//��ʼ�����ݿ�����
	Connection();
	
	//�ͷ����ݿ�������Դ
	~Connection();
	
	//�������ݿ�
	bool connect(std::string ip, unsigned short port, std::string user, std::string password, std::string dbname);
	
	//���²���  insert��update��delete
	bool update(std::string sql);
	
	//��ѯ���� select
	MYSQL_RES *querey(std::string sql);

	//ˢ��һ�����ӵ���ʼ����ʱ���
	void refreshAliveTime() { _aliveTime = clock(); }

	//���ش��ʱ��
	clock_t getAliveTime() const { return clock() - _aliveTime; }		//��ǰʱ�� - ��ʼ����ʱ���

private:
	MYSQL *_conn;		//��ʾ��MySQL Server��һ������
	clock_t _aliveTime;	//��¼�������״̬�����ʼʱ���
};