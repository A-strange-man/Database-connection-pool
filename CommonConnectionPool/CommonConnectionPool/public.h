#pragma once
#include <iostream>

//»’÷æ¥Ú”°
#define LOG(str) \
	std::cout << __FILE__ << " : " << __LINE__ << \
	__TIMESTAMP__ << " : " << str << std::endl;