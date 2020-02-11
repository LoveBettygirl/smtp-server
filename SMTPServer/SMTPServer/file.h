#pragma once
#include <io.h>
#include <cstring>
#include <string>
#include <vector>
using namespace std;
void getFiles1(string path, vector<string>& files)
{
	//文件句柄  
	//long hFile = 0;  //win7
	intptr_t hFile = 0;   //win10
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*.bin").c_str(), &fileinfo)) != -1)
		// "\\*"是指读取文件夹下的所有类型的文件，若想读取特定类型的文件，以png为例，则用“\\*.png”
	{
		do
		{
			//如果是目录，则递归遍历  
			//如果不是，加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				//判断是否为"."当前目录，".."上一层目录
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles1(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(path + "\\" + fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

void getFiles2(string path, vector<string>& files, vector<string> &ownname)
{
	/*files存储文件的路径及名称(eg.   C:\Users\WUQP\Desktop\test_devided\data1.txt)
	ownname只存储文件的名称(eg.     data1.txt)*/

	//文件句柄  
	intptr_t hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*.bin").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录，则递归遍历  
			//如果不是，加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				// 判断是否为"."当前目录，".."上一层目录
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles2(p.assign(path).append("\\").append(fileinfo.name), files, ownname);
			}
			else
			{
				files.push_back(path + "\\" + fileinfo.name);
				ownname.push_back(fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

bool fun(const string &p1, const string &p2)
{  
	//文件句柄  
	intptr_t hFile1 = 0, hFile2 = 0;
	//文件信息  
	struct _finddata_t fileinfo1, fileinfo2;
	hFile1 = _findfirst(p1.c_str(), &fileinfo1);
	hFile2 = _findfirst(p2.c_str(), &fileinfo2);
	return fileinfo1.time_write < fileinfo2.time_write;
}

void getfilebytime(string path, vector<string>& files)
{
	getFiles1(path, files);
	sort(files.begin(), files.end(), fun);
}