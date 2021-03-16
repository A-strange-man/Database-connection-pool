#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>	//条件变量用于线程间通信
#include "connection.h"
using namespace std;

/*
单例模式：
1.构造函数私有化
2.提供获取类对象的静态方法
*/

/*
连接池类
*/

class ConnectionPoll
{
public:
	//获取连接池对象实例
	static ConnectionPoll *getConnectionPoll();
	//给外部提供接口，获取一个可用的空闲连接
	shared_ptr<Connection> getConnection();
private:
	//构造函数私有化
	ConnectionPoll();	

	//从配置文件中加载配置项
	bool loadConfigFile();	

	/*
	运行在独立的线程中，专门负责生产新连接（当连接数不够的时候）
	将线程函数设置为成员方法可以很方便地访问成员变量
	*/
	void produceConnectionTask();

	//扫描超过 _maxIdleTime时间的空闲连接，进行多余的连接回收
	void scannerConnectionTask();

	string _ip;				//MySQL的IP地址
	unsigned short _port;	//端口号 默认3306
	string _username;		//MySQL用户名
	string _password;		//MySQL密码
	string _dbname;			//连接的数据库名称
	int _initSize;			//连接池的初始连接量
	int _maxSize;			//连接池的最大连接量
	int _maxIdleTime;		//连接池最大空闲时间
	int _connectionTimeOut;	//连接池获取连接的超时时间
	
	queue<Connection*> _connectionQueue;	//存储MySQL连接的队列。
	mutex _queueMutex;						//维护连接队列线程安全的互斥锁。
	atomic_int _connectionCount;			//记录连接所创建的connection连接的总数量，atomic_int类型可以保证线程安全
	condition_variable cv;					//设置条件变量，用于连接生产线程和连接消费线程的通信
};