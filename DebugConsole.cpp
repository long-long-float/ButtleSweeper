#include"DebugConsole.h"

DCStreambuf_d::DCStreambuf_d()
{
	AllocConsole();
	mhStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	mhStdin = GetStdHandle(STD_INPUT_HANDLE);
	
	setg(mGBuf, mGBuf, mGBuf);
	setp(mPBuf, mPBuf + mPBUFSIZE);
}

DCStreambuf_d::~DCStreambuf_d()
{
	FreeConsole();
}

int DCStreambuf_d::underflow()
{
	int c;
	if((gptr() >= mGBuf) && (gptr() < egptr()))
	{
		c = (int)*gptr();
		c &= 255;
		return c;
	}
	
	DWORD size;
	if(ReadConsole(mhStdin, mGBuf, mGBUFSIZE, &size, 0) == 0)
	{
		return std::char_traits<char>::eof();
	}
	setg(mGBuf, mGBuf, mGBuf + size);
	c = (int)mGBuf[0];
	c &= 255;
	return c;
}

int DCStreambuf_d::overflow(int_type iChar)
{
	int npend = (int)(pptr() - mPBuf);
	if(npend < 0)
	{
		return std::char_traits<char>::eof();
	}
	else if(npend > 0)
	{
		DWORD size;
		if(WriteConsole(mhStdout, mPBuf, npend, &size, 0) == 0)
		{
			return std::char_traits<char>::eof();
		}
	}
	setp(mPBuf + npend, mPBuf + mPBUFSIZE);
	if(iChar == std::char_traits<char>::eof())
	{
		return 0;
	}
	*pptr() = (char)(iChar & 255);
	pbump(1);
	return iChar;
}

int DCStreambuf_d::sync()
{
	int nleft = (int)(pptr() - mPBuf);
	DWORD size;
	
	char *p = mPBuf;
	while(nleft > 0)
	{
		if(WriteConsole(mhStdout, mPBuf, nleft, &size, 0) == 0)
		{
			return -1;
		}
		nleft -= (int)size;
		p += (int)size;
	}
	setp(mPBuf, mPBuf + mPBUFSIZE);
	return 0;
}

void DCStreambuf_d::ClearScreen()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD coord;
	DWORD dwWrite;
	
	GetConsoleScreenBufferInfo(mhStdout, &csbi );
	dwWrite = (csbi.dwSize.X * csbi.dwSize.Y);
	coord.X = 0;
	coord.Y = 0;
	SetConsoleCursorPosition(mhStdout, coord );
	SetConsoleTextAttribute(mhStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
	FillConsoleOutputCharacter(mhStdout, 0x20, dwWrite, coord, NULL );
	FillConsoleOutputAttribute(mhStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, dwWrite, coord, NULL );

}

void DCStreambuf_d::OutputStr(const char* str)
{
	WriteConsole(mhStdout, str, strlen(str), NULL, 0);
}

void DebugConsole_d::printf(const char *format, ...)
{
	va_list argp;
	va_start(argp, format);
	
	int l = _vscprintf(format, argp);
	char *str = new char[l + 1];
	vsprintf(str, format, argp);
	
	mDCStreambuf->OutputStr(str);
	
	delete [] str;
	
	va_end(argp);
}
