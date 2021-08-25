#ifndef LINKEREXCEPTION_H
#define LINKEREXCEPTION_H

#include <exception>
#include <string>

using namespace std;

class LinkerException : public exception
{
public:
	LinkerException(string msg) : msg(msg) {};
	string getMsg() { return msg; }
private:
	string msg;
};

#endif