#include "connection.h"
#include "public.h"

Connection::Connection()
{
	_conn = mysql_init(nullptr);	//��ʼ�����ݿ�����
}

Connection::~Connection()
{
	if (_conn != nullptr)
		mysql_close(_conn);		//�ͷ����ݿ�������Դ
}

bool Connection::connect(std::string ip, unsigned short port, std::string user, std::string password, std::string dbname)
{
	//�������ݿ�
	MYSQL *p = mysql_real_connect(_conn, ip.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
	return p != nullptr;
}

bool Connection::update(std::string sql)
{
	if (mysql_query(_conn, sql.c_str()))	//���ݿ���²�����insert��delete��update
	{
		LOG("����ʧ�ܣ�" + sql);
		return false;
	}
	return true;
}

MYSQL_RES * Connection::querey(std::string sql)
{
	if (mysql_query(_conn, sql.c_str()))	//���ݿ��ѯ������insert
	{
		LOG("��ѯʧ��" +sql);
		return nullptr;
	}
	return mysql_use_result(_conn);
}




