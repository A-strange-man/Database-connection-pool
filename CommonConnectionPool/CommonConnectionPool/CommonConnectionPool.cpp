#include "CommonConnectionPool.h"
#include "public.h"

//�̰߳�ȫ���������������ӿ�
ConnectionPoll * ConnectionPoll::getConnectionPoll()
{
	static ConnectionPoll pool;
	return &pool;
}

//�������ļ��м���������
bool ConnectionPoll::loadConfigFile()
{
	FILE *fp = fopen("mysql.ini", "r");
	if (fp == nullptr)
	{
		LOG("mysql.ini file is not exist");
		return false;
	}

	while (!feof(fp))
	{
		char line[1024] = { 0 };
		fgets(line, 1024, fp);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1)	//��Ч��������
		{
			continue;
		}
		int endidx = str.find('\n', idx);
		
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);
		
		if (key == "ip")
		{
			_ip = value;
		}
		else if (key == "port")
		{
			_port = atoi(value.c_str());
		}
		else if (key == "username")
		{
			_username = value;
		}
		else if (key == "password")
		{
			_password = value;
		}
		else if (key == "dbname")
		{
			_dbname = value;
		}
		else if (key == "initSize")
		{
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize")
		{
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdelTime")
		{
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeOut")
		{
			_connectionTimeOut = atoi(value.c_str());
		}
	}
	return true;
}

//���ӳصĹ��캯��
ConnectionPoll::ConnectionPoll()
{
	//����������
	if (!loadConfigFile())
		return;

	//������ʼ����������
	for (int i = 0; i < _initSize; ++i)
	{
		Connection *p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();		//ˢ��һ�¿������ӵ���ʼʱ��
		_connectionQueue.push(p);
		_connectionCount++;
	}

	//����һ���������̣߳���Ϊ���ӵ������ߡ�
	thread produce(std::bind(&ConnectionPoll::produceConnectionTask, this));
	produce.detach();

	//����һ����ʱ�̣߳�ɨ�賬�� _maxIdleTimeʱ��Ŀ������ӣ����ж�������ӻ���
	thread scanner(std::bind(&ConnectionPoll::scannerConnectionTask, this));
	scanner.detach();
}

//�̺߳����������ڶ������߳��У�ר�������µ�����
void ConnectionPoll::produceConnectionTask()
{
	while (1)
	{
		unique_lock<mutex> lock(_queueMutex);	//condition_variable ֻ�ܵȴ�unique_lock<mutex>�ϵ���������
		while (!_connectionQueue.empty())
		{
			cv.wait(lock);	//���в��գ��������߳̽���ȴ�״̬������ȴ�״̬ʱ�Ὣ���ͷš�
		}

		if (_connectionCount < _maxSize)
		{
			Connection *p = new Connection();
			if (!(p->connect(_ip, _port, _username, _password, _dbname)))
			{
				LOG("failed to create thread");
			}
			p->refreshAliveTime();		//ˢ��һ�¿������ӵ���ʼʱ��
			_connectionQueue.push(p);
			_connectionCount++;
		}

		//֪ͨ�������̣߳�����ʹ��������
		cv.notify_all();	

	}	//������Զ���mutex���ͷ�
}

//���ⲿ�ṩ�ӿڣ���ȡһ�����õĿ�������
shared_ptr<Connection> ConnectionPoll::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);	//��ǰ�̴߳����Ӷ��л�ȡ����ʱӦ�ü���

	while (_connectionQueue.empty())		
	{
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeOut)))	//��ʱ����
		{
			if (_connectionQueue.empty())
			{
				LOG("��ȡ�������ӳ�ʱ...");
				return nullptr;
			}
		}		//������Ϊ��ʱ�������������Ӷ����Ƿ��п�������
	}

	/*
	shared_ptr����ָ������ʱ�����ӻᱻ�ص�
	��Ҫ�Զ���shared_ptr���ͷ���Դ�ķ�ʽ (�Զ�������ָ���ɾ����)�������ӹ黹�����Ӷ�����
	*/
	shared_ptr<Connection> p(_connectionQueue.front(), 
		[&](Connection *p) {
			unique_lock<mutex> lock(_queueMutex);	//�����Ӷ��еĲ���һ��Ҫ��������֤�̰߳�ȫ
			p->refreshAliveTime();		//ˢ��һ�¿������ӵ���ʼʱ��
			_connectionQueue.push(p); 
		}
	);
	_connectionQueue.pop();
	if (_connectionQueue.empty())
	{
		cv.notify_all();	//���Ӷ��п��ˣ�֪ͨ��������������
	}

	return p;
}

//ɨ�賬�� _maxIdleTimeʱ��Ŀ������ӣ����ж�������ӻ���
void ConnectionPoll::scannerConnectionTask()
{
	while (1)
	{
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		//ɨ����У��ͷŶ������ӡ�ֻͨ����ͷ�����жϾ���
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCount > _initSize)
		{
			Connection *p = _connectionQueue.front();
			if (p->getAliveTime() >= _maxIdleTime * 1000)
			{
				_connectionQueue.pop();
				_connectionCount--;
				delete p;	//����~Connection()
			}
			else
			{
				break;
			}
		}
	}
}