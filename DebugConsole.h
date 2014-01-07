#ifndef INCLUDED_DEBUGCONSOLE_H
#define INCLUDED_DEBUGCONSOLE_H

/*
DebugConsole ver 2.5
2012/3/27
*/

#include<windows.h>
#include<iostream>
#include<stdarg.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "kernel32.lib")

class DCStreambuf_d : public std::streambuf
{
public:
	DCStreambuf_d();
	~DCStreambuf_d();
	
	virtual int underflow();
	virtual int overflow(int_type iChar = EOF);
	virtual int sync();
	
	void ClearScreen();
	void OutputStr(const char* str);
protected:
	HANDLE mhStdout;
	HANDLE mhStdin;
	
	static const int mGBUFSIZE = 1024;
	static const int mPBUFSIZE = 1024;
	
	char mGBuf[mGBUFSIZE + 1];
	char mPBuf[mPBUFSIZE + 1];
};

class DebugConsole_d : public std::iostream
{
private:
	DCStreambuf_d* mDCStreambuf;
public:
	DebugConsole_d() : std::iostream(mDCStreambuf = new DCStreambuf_d)
	{
	}
	~DebugConsole_d()
	{
		delete mDCStreambuf;
	}

	void ClearScreen()
	{
		mDCStreambuf->ClearScreen();
	}
	
	void printf(const char *format, ...);
};

//É_É~Å[
class DCStreambuf_r : public std::streambuf
{
};

class DebugConsole_r : public std::iostream
{
private:
	DCStreambuf_r* mDCStreambuf;
public:
	DebugConsole_r() : std::iostream(mDCStreambuf = new DCStreambuf_r)
	{
	}
	~DebugConsole_r()
	{
		delete mDCStreambuf;
	}

	void ClearScreen()
	{
	}
	
	void printf(const char *format, ...)
	{
	}
};

#endif //INCLUDED_DEBUGCONSOLE_H
