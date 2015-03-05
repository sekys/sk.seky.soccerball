#ifndef BALL_log4cpp_H
#define BALL_log4cpp_H

#include <typeinfo>
#include <log4cpp/Category.hh>

#pragma comment(lib, "ws2_32.lib") 

#ifdef _DEBUG
	#define CREATE_LOG4CPP() &log4cpp::Category::getInstance( typeid(this).name() )
	#pragma comment(lib, "log4cppD.lib")
#else
	//#define CREATE_LOG4CPP() null
	#define CREATE_LOG4CPP() &log4cpp::Category::getInstance( typeid(this).name() )
	#pragma comment(lib, "log4cppLIB.lib")
#endif


#endif