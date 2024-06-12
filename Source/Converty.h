#pragma once

#include <string>

class Converty
{
public:
	static std::string WideToANSI(const std::wstring& wstr) {
		int count = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
		std::string str(count, 0);
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
		return str;
	}

	static std::wstring AnsiToWide(const std::string& str)
	{
		int count = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), NULL, 0);
		std::wstring wstr(count, 0);
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), &wstr[0], count);
		return wstr;
	}

	static std::string WideToUtf8(const std::wstring& wstr)
	{
		int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
		std::string str(count, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
		return str;
	}

	static std::wstring Utf8ToWide(const std::string& str)
	{
		int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
		std::wstring wstr(count, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstr[0], count);
		return wstr;
	}
};

