#pragma once
#include <iostream>

//��־��ӡ
#define LOG(str) \
	std::cout << __FILE__ << " : " << __LINE__ << \
	__TIMESTAMP__ << " : " << str << std::endl;