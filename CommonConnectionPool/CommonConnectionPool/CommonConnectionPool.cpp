#include "CommonConnectionPool.h"
#include "public.h"

//线程安全的懒汉单例函数接口
ConnectionPoll * ConnectionPoll::getConnectionPoll()
{
	static ConnectionPoll pool;
	return &pool;
}

//从配置文件中加载配置项
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
		if (idx == -1)	//无效的配置项
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

//连接池的构造函数
ConnectionPoll::ConnectionPoll()
{
	//加载配置项
	if (!loadConfigFile())
		return;

	//创建初始数量的连接
	for (int i = 0; i < _initSize; ++i)
	{
		Connection *p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();		//刷新一下空闲连接的起始时间
		_connectionQueue.push(p);
		_connectionCount++;
	}

	//启动一个生产者线程，作为连接的生产者。
	thread produce(std::bind(&ConnectionPoll::produceConnectionTask, this));
	produce.detach();

	//启动一个定时线程，扫描超过 _maxIdleTime时间的空闲连接，进行多余的连接回收
	thread scanner(std::bind(&ConnectionPoll::scannerConnectionTask, this));
	scanner.detach();
}

//线程函数，运行在独立的线程中，专门生产新的连接
void ConnectionPoll::produceConnectionTask()
{
	while (1)
	{
		unique_lock<mutex> lock(_queueMutex);	//condition_variable 只能等待unique_lock<mutex>上的条件变量
		while (!_connectionQueue.empty())
		{
			cv.wait(lock);	//队列不空，生产者线程进入等待状态。进入等待状态时会将锁释放。
		}

		if (_connectionCount < _maxSize)
		{
			Connection *p = new Connection();
			if (!(p->connect(_ip, _port, _username, _password, _dbname)))
			{
				LOG("failed to create thread");
			}
			p->refreshAliveTime();		//刷新一下空闲连接的起始时间
			_connectionQueue.push(p);
			_connectionCount++;
		}

		//通知消费者线程，可以使用连接了
		cv.notify_all();	

	}	//这里会自动将mutex锁释放
}

//给外部提供接口，获取一个可用的空闲连接
shared_ptr<Connection> ConnectionPoll::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);	//当前线程从连接队列获取连接时应该枷锁

	while (_connectionQueue.empty())		
	{
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeOut)))	//因超时唤醒
		{
			if (_connectionQueue.empty())
			{
				LOG("获取空闲连接超时...");
				return nullptr;
			}
		}		//不是因为超时被唤醒则检查连接队列是否有空闲连接
	}

	/*
	shared_ptr智能指针析构时，连接会被关掉
	需要自定义shared_ptr的释放资源的方式 (自定义智能指针的删除器)，把连接归还到连接队列中
	*/
	shared_ptr<Connection> p(_connectionQueue.front(), 
		[&](Connection *p) {
			unique_lock<mutex> lock(_queueMutex);	//对连接队列的操作一定要加锁，保证线程安全
			p->refreshAliveTime();		//刷新一下空闲连接的起始时间
			_connectionQueue.push(p); 
		}
	);
	_connectionQueue.pop();
	if (_connectionQueue.empty())
	{
		cv.notify_all();	//连接队列空了，通知生产者生产连接
	}

	return p;
}

//扫描超过 _maxIdleTime时间的空闲连接，进行多余的连接回收
void ConnectionPoll::scannerConnectionTask()
{
	while (1)
	{
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		//扫描队列，释放多余连接。只通过队头连接判断就行
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCount > _initSize)
		{
			Connection *p = _connectionQueue.front();
			if (p->getAliveTime() >= _maxIdleTime * 1000)
			{
				_connectionQueue.pop();
				_connectionCount--;
				delete p;	//调用~Connection()
			}
			else
			{
				break;
			}
		}
	}
}