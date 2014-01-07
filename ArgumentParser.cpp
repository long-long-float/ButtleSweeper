#include"ArgumentParser.h"
using namespace std;

typedef pair<string, string> Argument;

void splitString(const char* src, char delimiter, vector<string> *argsBuf)
{
	int begin = 0;
	for(int idx = 0;src[idx] != '\0';idx++)
	{
		if(src[begin] == delimiter) begin++;
		if(src[idx] != delimiter && (src[idx + 1] == '\0' || src[idx + 1] == delimiter))
		{
			int len = idx - begin + 1;
			char* buf = new char[len + 1];
			//ÉRÉsÅ[
			for(int i = 0;i < len;i++) buf[i] = src[begin + i];
			buf[len] = '\0';
			argsBuf->push_back(string(buf));
			delete [] buf;
			
			begin = idx + 1;
		}
	}
}

void toLower(string *str)
{
	for(int i = 0;i < str->size();i++)
	{
		if('A' <= (*str)[i] && (*str)[i] < 'Z') (*str)[i] = 'a' + (*str)[i] - 'A';
	}
}

ArgumentParser::ArgumentParser(char switchChar, int argc, char* argv[])
{
	vector<string> args;
	for(int i = 1;i < argc;i++)
	{
		args.push_back(string(argv[i]));
	}
	setting(switchChar, args);
}

ArgumentParser::ArgumentParser(char switchChar, char* lpCmdLine)
{
	vector<string> args;
	splitString(lpCmdLine, ' ', &args);
	
	setting(switchChar, args);
}

ArgumentParser::~ArgumentParser()
{
}

void ArgumentParser::foreach(std::function<void(string, string)> func)
{
	for(map<string, string>::iterator itr = mArgMap.begin();itr != mArgMap.end();itr++)
	{
		func(itr->first, itr->second);
	}
}

bool ArgumentParser::getParam(string name, string *buf)
{
	toLower(&name);
	map<string, string>::iterator itr = mArgMap.find(name);
	if(itr != mArgMap.end())
	{
		*buf = itr->second;
		return true;
	}
	else return false;
}

void ArgumentParser::setting(char switchChar, vector<string> &args)
{
	int begin = 0;
	if(args.size() >= 1 && args[0][0] != switchChar)
	{
		mArgMap.insert(Argument("", args[0]));
		begin++;
	}
	for(int i = begin;i < args.size();i += 2)
	{
		if(args[i][0] != switchChar) continue;
		string second = ((i + 1) < args.size())? args[i + 1] : "";
		toLower(&args[i]);
		mArgMap.insert(Argument(args[i], second));
	}
}
