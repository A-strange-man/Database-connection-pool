#include <iostream>
#include "connection.h"
#include "public.h"
#include "CommonConnectionPool.h"

int main()
{
	/*Connection conn;
	char sql[1024] = { 0 };
	sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
		"zhangsan", 20, "male");
	conn.connect("127.0.0.1", 3306, "root", "yijia", "chat");
	conn.update(sql);*/

	ConnectionPoll *cp = ConnectionPoll::getConnectionPoll();


	return 0;
}
