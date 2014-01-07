#pragma once

#include<map>
#include<string>
#include<vector>
#include<functional>

class ArgumentParser
{
public:
	ArgumentParser(char switchChar, int argc, char* argv[]);
	ArgumentParser(char switchChar, char* lpCmdLine);
	~ArgumentParser();
	
	void foreach(std::function<void(std::string, std::string)> func);
	
	bool getParam(std::string name, std::string *buf);
private:
	void setting(char switchChar, std::vector<std::string> &args);
	
	std::map<std::string, std::string> mArgMap;
};
