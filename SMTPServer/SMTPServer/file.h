#pragma once
#include <io.h>
#include <cstring>
#include <string>
#include <vector>
using namespace std;
void getFiles1(string path, vector<string>& files)
{
	//�ļ����  
	//long hFile = 0;  //win7
	intptr_t hFile = 0;   //win10
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*.bin").c_str(), &fileinfo)) != -1)
		// "\\*"��ָ��ȡ�ļ����µ��������͵��ļ��������ȡ�ض����͵��ļ�����pngΪ�������á�\\*.png��
	{
		do
		{
			//�����Ŀ¼����ݹ����  
			//������ǣ������б�  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				//�ж��Ƿ�Ϊ"."��ǰĿ¼��".."��һ��Ŀ¼
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
	/*files�洢�ļ���·��������(eg.   C:\Users\WUQP\Desktop\test_devided\data1.txt)
	ownnameֻ�洢�ļ�������(eg.     data1.txt)*/

	//�ļ����  
	intptr_t hFile = 0;
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*.bin").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//�����Ŀ¼����ݹ����  
			//������ǣ������б�  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				// �ж��Ƿ�Ϊ"."��ǰĿ¼��".."��һ��Ŀ¼
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
	//�ļ����  
	intptr_t hFile1 = 0, hFile2 = 0;
	//�ļ���Ϣ  
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