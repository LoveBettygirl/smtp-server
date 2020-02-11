#pragma once
#include <Windows.h>
#include <string>
#include <wchar.h>
using namespace std;
char *utf8togb(const char* utf8)//UTF-8到GB2312的转换
{
	int length = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[length + 1];
	memset(wstr, 0, length + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, length);
	length = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[length + 1];
	memset(str, 0, length + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, length, NULL, NULL);
	if (wstr)
	{
		delete[] wstr;
	}
	return str;
}
char *gbtoutf8(const char* gb2312) //GB2312到UTF-8的转换
{
	int length = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[length + 1];
	memset(wstr, 0, length + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, length);
	length = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[length + 1];
	memset(str, 0, length + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, length, NULL, NULL);
	if (wstr)
	{
		delete[] wstr;
	}
	return str;
}