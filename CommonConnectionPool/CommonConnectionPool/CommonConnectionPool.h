#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>	//�������������̼߳�ͨ��
#include "connection.h"
using namespace std;

/*
����ģʽ��
1.���캯��˽�л�
2.�ṩ��ȡ�����ľ�̬����
*/

/*
���ӳ���
*/

class ConnectionPoll
{
public:
	//��ȡ���ӳض���ʵ��
	static ConnectionPoll *getConnectionPoll();
	//���ⲿ�ṩ�ӿڣ���ȡһ�����õĿ�������
	shared_ptr<Connection> getConnection();
private:
	//���캯��˽�л�
	ConnectionPoll();	

	//�������ļ��м���������
	bool loadConfigFile();	

	/*
	�����ڶ������߳��У�ר�Ÿ������������ӣ���������������ʱ��
	���̺߳�������Ϊ��Ա�������Ժܷ���ط��ʳ�Ա����
	*/
	void produceConnectionTask();

	//ɨ�賬�� _maxIdleTimeʱ��Ŀ������ӣ����ж�������ӻ���
	void scannerConnectionTask();

	string _ip;				//MySQL��IP��ַ
	unsigned short _port;	//�˿ں� Ĭ��3306
	string _username;		//MySQL�û���
	string _password;		//MySQL����
	string _dbname;			//���ӵ����ݿ�����
	int _initSize;			//���ӳصĳ�ʼ������
	int _maxSize;			//���ӳص����������
	int _maxIdleTime;		//���ӳ�������ʱ��
	int _connectionTimeOut;	//���ӳػ�ȡ���ӵĳ�ʱʱ��
	
	queue<Connection*> _connectionQueue;	//�洢MySQL���ӵĶ��С�
	mutex _queueMutex;						//ά�����Ӷ����̰߳�ȫ�Ļ�������
	atomic_int _connectionCount;			//��¼������������connection���ӵ���������atomic_int���Ϳ��Ա�֤�̰߳�ȫ
	condition_variable cv;					//���������������������������̺߳����������̵߳�ͨ��
};