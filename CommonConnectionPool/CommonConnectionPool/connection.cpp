#include "connection.h"
#include "public.h"

Connection::Connection()
{
	_conn = mysql_init(nullptr);	//初始化数据库连接
}

Connection::~Connection()
{
	if (_conn != nullptr)
		mysql_close(_conn);		//释放数据库连接资源
}

bool Connection::connect(std::string ip, unsigned short port, std::string user, std::string password, std::string dbname)
{
	//连接数据库
	MYSQL *p = mysql_real_connect(_conn, ip.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
	return p != nullptr;
}

bool Connection::update(std::string sql)
{
	if (mysql_query(_conn, sql.c_str()))	//数据库更新操作，insert，delete，update
	{
		LOG("更新失败：" + sql);
		return false;
	}
	return true;
}

MYSQL_RES * Connection::querey(std::string sql)
{
	if (mysql_query(_conn, sql.c_str()))	//数据库查询操作，insert
	{
		LOG("查询失败" +sql);
		return nullptr;
	}
	return mysql_use_result(_conn);
}




