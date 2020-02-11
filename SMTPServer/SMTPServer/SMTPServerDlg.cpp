
// SMTPServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SMTPServer.h"
#include "SMTPServerDlg.h"
#include "afxdialogex.h"
#include <regex>
#include <string>
#include <fstream>
#include <unordered_map>
#include "utf-8.h"
#include "file.h"
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_CLEARINFO WM_USER+1

void findall(CString src, CString dst, vector<int> &index)
{
	int dstcount = dst.GetLength();
	int occur = 0;
	while ((occur = src.Find(dst, occur)) >= 0)
	{
		index.push_back(occur);
		occur += dstcount;
	}
}

CString char2CString(char *str)
{
	//计算char *数组大小，以字节为单位，一个汉字占两个字节
	int charLen = strlen(str);
	//计算多字节字符的大小，按字符计算。
	int len = MultiByteToWideChar(CP_ACP, 0, str, charLen, NULL, 0);
	//为宽字节字符数组申请空间，数组大小为按字节计算的多字节字符大小
	TCHAR *buf = new TCHAR[len + 1];
	//多字节编码转换成宽字节编码
	MultiByteToWideChar(CP_ACP, 0, str, charLen, buf, len);
	buf[len] = '\0';  //添加字符串结尾，注意不是len+1
	//将TCHAR数组转换为CString
	CString pWideChar;
	pWideChar.Append(buf);
	//删除缓冲区
	delete[]buf;
	return pWideChar;
}

char *CString2char(CString str)
{
	//注意：以下n和len的值大小不同,n是按字符计算的，len是按字节计算的
	int n = str.GetLength();
	//获取宽字节字符的大小，大小是按字节计算的
	int len = WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), NULL, 0, NULL, NULL);
	//为多字节字符数组申请空间，数组大小为按字节计算的宽字节字节大小
	char * p = new char[len + 1];  //以字节为单位
	//宽字节编码转换成多字节编码
	WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), p, len, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str, str.GetLength() + 1, p, len + 1, NULL, NULL);
	p[len + 1] = '/0';  //多字节字符以'/0'结束
	return p;
}

LRESULT CSMTPServerDlg::OnClearInfo(WPARAM wParam, LPARAM lParam)
{
	stock();
	clearInfo();
	return 0;
}

char *CSMTPServerDlg::base64_decode(char *src)
{
	char base64[] =
	{
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
		'O','P','Q','R','S','T','U','V','W','X','Y','Z',
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
		'o','p','q','r','s','t','u','v','w','x','y','z',
		'0','1','2','3','4','5','6','7','8','9','+','/'
	};
	unordered_map<char, int> base64map;	//unordered_map的底层是哈希表实现的，查询速度快
	int len = strlen(src), i, equalcount = 0;
	for (i = 0; i < 64; i++)
	{
		base64map[base64[i]] = i; //制作base64字符到值的映射，方便查找
	}
	string binstr = "";
	//将字符转为对应的6位二进制字符串
	for (i = 0; i < len; i++)
	{
		if (src[i] != '=')
		{
			//生成base64字符对应的值的6位二进制字符串
			char c = base64map[src[i]];
			char bin[9] = { 0 };
			itoa(c, bin, 2);
			char tarbin[7] = { 0 };
			sprintf(tarbin, "%06s", bin);
			binstr.append(tarbin);
		}
		else
		{
			binstr.append("000000");  //如果遇到结尾的等号则直接在二进制字符串后面补上0
			equalcount++; //等号个数加1
		}
	}
	picbuflen = binstr.length() / 8; //图片的字节大小应是二进制字符串长度的1/8
	char *stream = new char[picbuflen + 1];
	int count = 0;
	for (i = 0; i < binstr.length(); i += 8) //由于要生成1个字节的字符，所以循环的步长为8
	{
		int j;
		char c = 0;
		for (j = 0; j < 8; j++)
		{
			char temp;
			temp = (binstr[i + j] - 48);
			temp <<= (8 - 1 - j);
			temp = temp & 0xff;
			c += temp;
			c = c & 0xff;
		}
		stream[count++] = c;
	}
	stream[count] = '\0';
	picbuflen -= equalcount; //用统计的等号数目修正图片大小
							//不修正也不影响显示，但是最后可能会有冗余0字节
	return stream;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSMTPServerDlg 对话框



CSMTPServerDlg::CSMTPServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SMTPSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SMILE);
	memset(sendBuffer, 0, sizeof(sendBuffer));
	memset(recvBuffer, 0, sizeof(recvBuffer));
	m_pSocket = new CServerSocket;
	isinputdata = false;
	start = false;
	picfilename = NULL;
	dealimage = false;
	from = NULL;
	to = NULL;
	date = NULL;
	subject = NULL;
	txtfilename = NULL;
	attachname = NULL;
	base64src = NULL;
	picbuf = NULL;
	content = NULL;
	mailstr = NULL;
	picbuflen = 0;
	islookfile = false;
	delethis = false;
	txtcontent = NULL;
	txtfilename = NULL;
	hasimage = false;
	quit = false;
	firstpicfile = NULL;
}

CSMTPServerDlg::~CSMTPServerDlg()
{
	delete m_pSocket;
}

void CSMTPServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOGLIST, m_loglist);
	DDX_Control(pDX, IDC_MAILRECV, m_mailrecv);
	DDX_Control(pDX, IDC_MAILTEXT, m_mailtext);
	DDX_Control(pDX, IDC_PICTURE, m_picture);
	DDX_Control(pDX, IDC_LOOKMAIL, m_lookmail);
	DDX_Control(pDX, IDC_DELEMAIL, m_delemail);
	DDX_Control(pDX, IDC_EMAILCONTENTS, m_queue);
}

BEGIN_MESSAGE_MAP(CSMTPServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_LOOKMAIL, &CSMTPServerDlg::OnBnClickedLookmail)
	ON_BN_CLICKED(IDC_DELEMAIL, &CSMTPServerDlg::OnBnClickedDelemail)
	ON_CBN_SELCHANGE(IDC_EMAILCONTENTS, &CSMTPServerDlg::OnCbnSelchangeEmailcontents)
END_MESSAGE_MAP()


// CSMTPServerDlg 消息处理程序

void CSMTPServerDlg::clearInfo()
{
	//清空此次接收或查看邮件的环境，以便下次接收邮件或查看邮件
	memset(sendBuffer, 0, sizeof(sendBuffer));
	memset(recvBuffer, 0, sizeof(recvBuffer));
	isinputdata = false;
	start = false;
	dealimage = false;
	from = NULL;
	to = NULL;
	date = NULL;
	subject = NULL;
	base64src = NULL;
	picbuf = NULL;
	content = NULL;
	mailstr = NULL;
	picbuflen = 0;
	if (m_queue.GetCount())
	{
		m_lookmail.EnableWindow(TRUE);
		m_delemail.EnableWindow(TRUE);
	}
	else
	{
		m_lookmail.EnableWindow(FALSE);
		m_delemail.EnableWindow(FALSE);
	}
	m_queue.EnableWindow(TRUE);
	CString temp;
	temp.Format(L"（发送队列共有%d封邮件）", m_queue.GetCount());
	((CStatic*)GetDlgItem(IDC_TIPS))->SetWindowTextW(temp);
}

void CSMTPServerDlg::stock()
{
	//将邮件信息存入发送队列
	//由vector维护
	MailInfo *mailinfo=new MailInfo;
	mailinfo->date = date;
	mailinfo->from = from;
	mailinfo->mailstr = mailstr;
	mailinfo->subject = subject;
	mailinfo->to = to;
	info.push_back(*mailinfo);
	char *f=new char[50];
	string d = date;
	int t;
	while ((t=d.find(" ")) >= 0)
	{
		d.replace(t, 1, "_");
	}
	d.erase(d.find(","), 1);
	d.erase(d.find("+"), 1);
	d.replace(d.find(":"), 1, "_");
	d.replace(d.find(":"), 1, "_");
	sprintf(f, "%s.bin", d.c_str());
	m_queue.InsertString(m_queue.GetCount(), char2CString(date));
	fileindex = m_queue.GetCount() - 1;
	m_queue.SetCurSel(m_queue.GetCount() - 1);
	string fn = f;
	fn = "queue\\" + fn;
	//将必要的邮件信息存入本地文件，这样下次打开程序也可以加载这些邮件
	//其他不必要的信息在打开邮件的时候再解析
	ofstream outfile(fn.c_str(), ios::out | ios::binary);
	int len = strlen(date);
	outfile.write((char *)&len, sizeof(int)); //存入字符串长度是为了方便从哪个位置读取
	outfile.write(date, strlen(date));
	len = strlen(from);
	outfile.write((char *)&len, sizeof(int));
	outfile.write(from, strlen(from));
	len = strlen(to);
	outfile.write((char *)&len, sizeof(int));
	outfile.write(to, strlen(to));
	len = strlen(mailstr);
	outfile.write((char *)&len, sizeof(int));
	outfile.write(mailstr, strlen(mailstr));
	outfile.close();
	m_queue.SetCurSel(m_queue.GetCount());
}

void CSMTPServerDlg::greetClient(CClientSocket *pSocket)
{
	//此函数由CClientSocket::OnSend()调用
	if (!start)	//如果还没有进入到对话状态，就向客户端发出问候消息
	{
		sprintf(sendBuffer, "220 127.0.0.1 SMTP Mail Server\r\n");
		start = true;
	}
	else
		return;
	int ret = pSocket->Send(sendBuffer, strlen(sendBuffer));
	if (ret == SOCKET_ERROR)
	{
		//WSAEWOULDBLOCK：非阻塞模式下，请求的操作被阻塞，需等待再次调用
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			CString error;
			int errorcode = GetLastError();
			error.Format(L"Socket failed to send: %d", errorcode);
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				errorcode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //Default language
				(LPTSTR)&lpMsgBuf,
				0,
				NULL
				);
			error = error + L"\n" + (LPCTSTR)lpMsgBuf;
			AfxMessageBox(error);
			pSocket->Close();
		}
	}
	else
	{
		CString temp = char2CString(sendBuffer);
		temp = L"S: " + temp;
		m_loglist.InsertString(m_loglist.GetCount(), temp);
		pSocket->AsyncSelect(FD_READ); //设置接收消息类型为receive，准备与客户端会话或接收邮件数据
	}
}

//处理邮件信息
UINT processMail(LPVOID hWnd)
{
	CSMTPServerDlg *pDlg = (CSMTPServerDlg*)AfxGetApp()->GetMainWnd(); //获取主窗口句柄
	pDlg->hasimage = false;
	pDlg->firstpicfile = NULL;
	pDlg->Invalidate(true);
	pDlg->m_lookmail.EnableWindow(FALSE);
	pDlg->m_delemail.EnableWindow(FALSE);
	pDlg->m_queue.EnableWindow(FALSE);
	CString mail;
	if (pDlg->islookfile) //如果正在查看队列中的邮件，就从队列中获取待解析的邮件内容
	{
		mail = char2CString(pDlg->mailstr);
		pDlg->m_mailrecv.SetWindowTextW(mail);
	}
	else  //如果是正在接收邮件，则从窗口中获取待解析的邮件内容
	{
		pDlg->m_mailrecv.GetWindowTextW(mail);
		pDlg->mailstr = CString2char(mail);
	}

	//如果可能有图片（有可能是其他附件），则在图片显示框中显示Loading......
	if (mail.Find(L"Content-Type: image") >= 0 || mail.Find(L"Content-Type: application/octet-stream") >= 0)
	{
		pDlg->dealimage = true;
		pDlg->Invalidate(true);
		pDlg->m_picture.UpdateWindow();
	}

	pDlg->m_mailtext.SetWindowTextW(L"From: "+char2CString(pDlg->from)+
		L"\r\nTo: "+char2CString(pDlg->to)+L"\r\n"); //在邮件文字信息框显示发件方和收件方

	if (mail.Find(L"Date: ") >= 0) //在邮件文字信息框显示发件时间
	{
		regex datereg("Date: ([a-zA-Z]+, \\d+ [a-zA-Z]+ \\d+ \\d+:\\d+:\\d+ [\\+\\-]\\d+)");
		smatch result;
		string test(pDlg->mailstr);
		regex_search(test, result, datereg);
		for (int i = 0; i < result.size(); i++)
		{
			regex reg("[a-zA-Z]+, \\d+ [a-zA-Z]+ \\d+ \\d+:\\d+:\\d+ [\\+\\-]\\d+");
			string a = result[i].str();
			if (a.find("Date: ") == 0)
			{
				continue;
			}
			if (regex_match(a, reg))
			{
				pDlg->date = new char[a.length() + 1];
				::strcpy(pDlg->date, (char*)a.c_str());
			}
		}
		CString t;
		pDlg->m_mailtext.GetWindowTextW(t);
		pDlg->m_mailtext.SetWindowTextW(t + L"Date: " + char2CString(pDlg->date)
			+ L"\r\n");
	}

	if (mail.Find(L"Subject: ") >= 0) //在邮件文字信息框显示邮件标题
	{
		regex titleenreg("Subject: (.*)"), titlechreg("Subject: =\\?([\\w\\-\\.]*)\\?[\\w]\\?([a-zA-Z0-9\\+\\/\\=]*)\\?=");
		smatch result1, result2;
		string test(pDlg->mailstr);
		regex_search(test, result1, titlechreg);
		regex_search(test, result2, titleenreg);
		bool isgb = false, isutf8 = false;
		for (int i = 1; i < result1.size(); i++)
		{
			string a = result1[i].str();
			if (result1.size() == 3)
			{
				//判断标题编码
				if (!a.compare("gb2312") || !a.compare("GB2312"))
				{
					isgb = true;
					continue;
				}
				else if (!a.compare("utf-8") || !a.compare("UTF-8"))
				{
					isutf8 = true;
					continue;
				}
			}
			pDlg->subject = new char[a.length() + 1];
			::strcpy(pDlg->subject, (char*)a.c_str());
		}
		if (!isgb && !isutf8) //如果没有任何这里给的编码信息，则直接显示标题内容
		{
			for (int i = 1; i < result2.size(); i++)
			{
				string a = result2[i].str();
				if (result2.size() == 2)
				{
					pDlg->subject = new char[a.length() + 1];
					::strcpy(pDlg->subject, (char*)a.c_str());
				}
			}
		}
		else  //base64
		{
			string base64str = pDlg->subject;
			int pos;
			int count = 0;
			//去除base64编码中的回车换行
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->subject;
			pDlg->subject = new char[base64str.length() + 1];
			::strcpy(pDlg->subject, (char*)base64str.c_str());
			pDlg->subject = pDlg->base64_decode(pDlg->subject);	 //base64解码
			if (isutf8)
			{
				pDlg->subject = utf8togb(pDlg->subject);
			}
		}
		CString t;
		pDlg->m_mailtext.GetWindowTextW(t);
		pDlg->m_mailtext.SetWindowTextW(t + L"Subject: " + char2CString(pDlg->subject)
			+ L"\r\n");
	}

	vector<int> txt, txt2, img, attach, attach2;
	findall(mail, L"Content-Type: text/plain", txt);  //找出相应的所有子串的索引
	findall(mail, L"Content-Type: text/enriched", txt2);
	findall(mail, L"Content-Type: image", img);
	findall(mail, L"Content-Type: application/octet-stream", attach);
	findall(mail, L"Content-Type: application/x-msdownload", attach2);

	//在邮件文字信息框显示邮件文本及文本附件
	for (int j = 0; j < txt.size(); j++)
	{
		CString judge;
		if (j == txt.size() - 1)
		{
			judge = mail.Mid(txt[j]);
		}
		else
		{
			judge = mail.Mid(txt[j], txt[j + 1] - txt[j]);
		}
		//可以适应outlook和foxmail两种客户端
		regex regchtext("Content-Type: text/(?:plain|enriched);\\s*charset=\"(\\S*)\"\\s*Content-Transfer-Encoding: (\\S*)\\r\\n\\r\\n([a-zA-Z0-9\\/\\+=\\r\\n]*)------");
		regex regentext("Content-Type: text/(?:plain|enriched);\\s*charset=\"(?:\\S*)\"\\s*Content-Transfer-Encoding: (?:7bit|quoted-printable)\\r\\n\\r\\n([\\S\\s]*)\\r\\n\\r\\n\\r\\n------");
		regex regfiletxt("Content-Type: text/(?:plain|enriched);\\s*name=\"(\\S*\\.\\S*)\"\\s*Content-Transfer-Encoding: (\\S*)\\s*Content-Disposition: attachment;\\s*filename=\"(?:\\S*\\.\\S*)\"\\r\\n\\r\\n([\\S\\s]*)------");
		smatch result1, result2, result3;
		string test(CString2char(judge));
		regex_search(test, result1, regchtext);
		regex_search(test, result2, regentext);
		regex_search(test, result3, regfiletxt);
		bool match1 = false, isbase64 = false, isutf8 = false, match3 = false;
		if (result3.size())
		{
			match3 = true;
			for (int i = 1; i < result3.size(); i++)
			{
				regex filen("\\S*\\.\\S*");
				string a = result3[i].str();
				if (!a.compare("utf-8") || !a.compare("UTF-8"))
				{
					isutf8 = true;
					continue;
				}
				if (a.compare("base64") == 0 && !isbase64)
				{
					isbase64 = true;
					continue;
				}
				if (regex_match(a, filen))
				{
					pDlg->txtfilename = new char[a.length() + 1];
					::strcpy(pDlg->txtfilename, (char*)a.c_str());
				}
				else
				{
					pDlg->txtcontent = new char[a.length() + 1];
					::strcpy(pDlg->txtcontent, (char*)a.c_str());
				}
			}
			if (isbase64)	 //base64
			{
				string base64str = pDlg->txtcontent;
				CString tempp = char2CString(pDlg->txtcontent);
				if (tempp.Find(L"------") >= 0)
				{
					tempp = tempp.Left(tempp.Find(L"------"));
				}
				base64str = CString2char(tempp);
				int pos;
				int count = 0;
				//去除base64编码中的回车换行
				while ((pos = base64str.find("\r\n")) >= 0)
				{
					base64str = base64str.erase(pos, 2);
					count++;
				}
				delete[]pDlg->txtcontent;
				pDlg->txtcontent = new char[base64str.length() + 1];
				::strcpy(pDlg->txtcontent, (char*)base64str.c_str());
				pDlg->txtcontent = pDlg->base64_decode(pDlg->txtcontent);	//base64解码
				if (isutf8)
				{
					pDlg->txtcontent = utf8togb(pDlg->txtcontent);
				}
				ofstream outFile(pDlg->txtfilename, ios::out | ios::binary);
				outFile.write(pDlg->txtcontent, pDlg->picbuflen);
				outFile.close();
			}
			else
			{
				CString temp;
				temp = char2CString(pDlg->txtcontent);
				temp = temp.Left(temp.GetLength() - 2);
				pDlg->txtcontent = CString2char(temp);
				ofstream outFile(pDlg->txtfilename, ios::out | ios::binary);
				outFile.write(pDlg->txtcontent, strlen(pDlg->txtcontent));
				outFile.close();
			}
			CString t;
			pDlg->m_mailtext.GetWindowTextW(t);
			pDlg->m_mailtext.SetWindowTextW(t + L"\r\nText Filename: \r\n" + char2CString(pDlg->txtfilename) + L"\r\nText File Content: \r\n" + char2CString(pDlg->txtcontent)
				+ L"\r\n");
			continue;
		}

		isbase64 = false;
		isutf8 = false;
		for (int i = 1; i < result1.size(); i++)
		{
			string a = result1[i].str();
			if (!a.compare("utf-8") || !a.compare("UTF-8"))
			{
				isutf8 = true;
				continue;
			}
			if (a.compare("base64") == 0 && !isbase64)
			{
				isbase64 = true;
				continue;
			}
			else if (isbase64)
			{
				pDlg->content = new char[a.length() + 1];
				::strcpy(pDlg->content, (char*)a.c_str());
				match1 = true;
				break;
			}
		}
		if (!match1)  //正文内容无特殊编码
		{
			for (int i = 1; i < result2.size(); i++)
			{
				regex contentreg("[\\S\\s]*");
				string a = result2[i].str();
				if (regex_match(a, contentreg))
				{
					pDlg->content = new char[a.length() + 1];
					::strcpy(pDlg->content, (char*)a.c_str());
					break;
				}
			}
			/*if (pDlg->txtcontent&&pDlg->txtfilename)
			{
				ofstream outFile(pDlg->txtfilename, ios::out | ios::binary);
				outFile.write(pDlg->txtcontent, strlen(pDlg->txtcontent)); //将图片保存到本地，这样就从本地读取图片并加载
				outFile.close();
			}  */
		}
		if (isbase64)	 //base64
		{
			string base64str = pDlg->content;
			int pos;
			int count = 0;
			//去除base64编码中的回车换行
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->content;
			pDlg->content = new char[base64str.length() + 1];
			::strcpy(pDlg->content, (char*)base64str.c_str());
			pDlg->content = pDlg->base64_decode(pDlg->content);	//base64解码
			if (isutf8)
			{
				pDlg->content = utf8togb(pDlg->content);
			}
			if (pDlg->txtcontent&&pDlg->txtfilename)
			{
				ofstream outFile(pDlg->txtfilename, ios::out | ios::binary);
				outFile.write(pDlg->txtcontent, pDlg->picbuflen); 
				outFile.close();
			}
		}
		
		CString t;
		pDlg->m_mailtext.GetWindowTextW(t);
		pDlg->m_mailtext.SetWindowTextW(t + L"\r\nContent: \r\n" + char2CString(pDlg->content)
			+ L"\r\n");
	}

	//在邮件文字信息框显示邮件文本及文本附件
	for (int j = 0; j < txt2.size(); j++)
	{
		CString judge;
		if (j == txt2.size() - 1)
		{
			judge = mail.Mid(txt2[j]);
		}
		else
		{
			judge = mail.Mid(txt2[j], txt2[j + 1] - txt2[j]);
		}
		//可以适应outlook和foxmail两种客户端
		regex regchtext("Content-Type: text/(?:plain|enriched);\\s*charset=\"(\\S*)\"\\s*Content-Transfer-Encoding: (\\S*)\\r\\n\\r\\n([a-zA-Z0-9\\/\\+=\\r\\n]*)------");
		regex regentext("Content-Type: text/(?:plain|enriched);\\s*charset=\"us-ascii\"\\s*Content-Transfer-Encoding: (?:7bit|quoted-printable)\\r\\n\\r\\n([\\S\\s]*)\\r\\n\\r\\n\\r\\n------");
		regex regfiletxt("Content-Type: text/(?:plain|enriched);\\s*name=\"(\\S*\\.\\S*)\"\\s*Content-Transfer-Encoding: (\\S*)\\s*Content-Disposition: attachment;\\s*filename=\"(?:\\S*\\.\\S*)\"\\r\\n\\r\\n([\\S\\s]*)------");
		smatch result1, result2, result3;
		string test(CString2char(judge));
		regex_search(test, result1, regchtext);
		regex_search(test, result2, regentext);
		regex_search(test, result3, regfiletxt);
		bool match1 = false, isbase64 = false, isutf8 = false, match3 = false;
		if (result3.size())
		{
			match3 = true;
			for (int i = 1; i < result3.size(); i++)
			{
				regex filen("\\S*\\.\\S*");
				string a = result3[i].str();
				if (!a.compare("utf-8") || !a.compare("UTF-8"))
				{
					isutf8 = true;
					continue;
				}
				if (a.compare("base64") == 0 && !isbase64)
				{
					isbase64 = true;
					continue;
				}
				if (regex_match(a, filen))
				{
					pDlg->txtfilename = new char[a.length() + 1];
					::strcpy(pDlg->txtfilename, (char*)a.c_str());
				}
				else
				{
					pDlg->txtcontent = new char[a.length() + 1];
					::strcpy(pDlg->txtcontent, (char*)a.c_str());
				}
			}
			if (isbase64)	 //base64
			{
				string base64str = pDlg->txtcontent;
				CString tempp = char2CString(pDlg->txtcontent);
				if (tempp.Find(L"------") >= 0)
				{
					tempp = tempp.Left(tempp.Find(L"------"));
				}
				base64str = CString2char(tempp);
				int pos;
				int count = 0;
				//去除base64编码中的回车换行
				while ((pos = base64str.find("\r\n")) >= 0)
				{
					base64str = base64str.erase(pos, 2);
					count++;
				}
				delete[]pDlg->txtcontent;
				pDlg->txtcontent = new char[base64str.length() + 1];
				::strcpy(pDlg->txtcontent, (char*)base64str.c_str());
				pDlg->txtcontent = pDlg->base64_decode(pDlg->txtcontent);	//base64解码
				if (isutf8)
				{
					pDlg->txtcontent = utf8togb(pDlg->txtcontent);
				}
				ofstream outFile(pDlg->txtfilename, ios::out | ios::binary);
				outFile.write(pDlg->txtcontent, pDlg->picbuflen);
				outFile.close();
			}
			else
			{
				CString temp;
				temp = char2CString(pDlg->txtcontent);
				temp = temp.Left(temp.GetLength() - 2);
				pDlg->txtcontent = CString2char(temp);
				ofstream outFile(pDlg->txtfilename, ios::out | ios::binary);
				outFile.write(pDlg->txtcontent, strlen(pDlg->txtcontent));
				outFile.close();
			}
			CString t;
			pDlg->m_mailtext.GetWindowTextW(t);
			pDlg->m_mailtext.SetWindowTextW(t + L"\r\nText Filename: \r\n" + char2CString(pDlg->txtfilename) + L"\r\nText File Content: \r\n" + char2CString(pDlg->txtcontent)
				+ L"\r\n");
			continue;
		}

		isbase64 = false;
		isutf8 = false;
		for (int i = 1; i < result1.size(); i++)
		{
			string a = result1[i].str();
			if (!a.compare("utf-8") || !a.compare("UTF-8"))
			{
				isutf8 = true;
				continue;
			}
			if (a.compare("base64") == 0 && !isbase64)
			{
				isbase64 = true;
				continue;
			}
			else if (isbase64)
			{
				pDlg->content = new char[a.length() + 1];
				::strcpy(pDlg->content, (char*)a.c_str());
				match1 = true;
				break;
			}
		}
		if (!match1)  //正文内容无特殊编码
		{
			for (int i = 1; i < result2.size(); i++)
			{
				regex contentreg("[\\S\\s]*");
				string a = result2[i].str();
				if (regex_match(a, contentreg))
				{
					pDlg->content = new char[a.length() + 1];
					::strcpy(pDlg->content, (char*)a.c_str());
					break;
				}
			}
			/*if (pDlg->txtcontent&&pDlg->txtfilename)
			{
			ofstream outFile(pDlg->txtfilename, ios::out | ios::binary);
			outFile.write(pDlg->txtcontent, strlen(pDlg->txtcontent)); //将图片保存到本地，这样就从本地读取图片并加载
			outFile.close();
			}  */
		}
		if (isbase64)	 //base64
		{
			string base64str = pDlg->content;
			int pos;
			int count = 0;
			//去除base64编码中的回车换行
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->content;
			pDlg->content = new char[base64str.length() + 1];
			::strcpy(pDlg->content, (char*)base64str.c_str());
			pDlg->content = pDlg->base64_decode(pDlg->content);	//base64解码
			if (isutf8)
			{
				pDlg->content = utf8togb(pDlg->content);
			}
			if (pDlg->txtcontent&&pDlg->txtfilename)
			{
				ofstream outFile(pDlg->txtfilename, ios::out | ios::binary);
				outFile.write(pDlg->txtcontent, pDlg->picbuflen);
				outFile.close();
			}
		}

		CString t;
		pDlg->m_mailtext.GetWindowTextW(t);
		pDlg->m_mailtext.SetWindowTextW(t + L"\r\nContent: \r\n" + char2CString(pDlg->content)
			+ L"\r\n");
	}

	//如果可能有图片，如果有多张图片，则显示第一张图片
	for (int j = 0; j < img.size(); j++)
	{
		CString judge;
		if (j == img.size() - 1)
		{
			judge = mail.Mid(img[j]);
		}
		else
		{
			judge = mail.Mid(img[j], img[j + 1] - img[j]);
		}
		regex regpic("Content-Type: [\\w\\-\\/]*;\\s*name=\"(\\S*\\.\\S*)\"\\s*Content-Transfer-Encoding: base64\\r\\n(?:Content-ID: <.+>|Content-Disposition: attachment;\\r\\n\\tfilename=\".*\")\\r\\n\\r\\n([a-zA-Z0-9/=\\+\\r\\n]+)");
		smatch result;
		string test(CString2char(judge));
		regex_search(test, result, regpic);
		bool matchfilename = false, matchpic = false;
		for (int i = 0; i < result.size(); i++)
		{
			if (matchfilename && matchpic)//确保只匹配第一个出现的图片
				break;
			regex filenamereg("\\S*\\.\\S*"), picdetailreg("[a-zA-Z0-9/=\\+\\r\\n]+");
			string a = result[i].str();
			if (regex_match(a, filenamereg))
			{
				pDlg->picfilename = new char[a.length() + 1];
				::strcpy(pDlg->picfilename, (char*)a.c_str());
				matchfilename = true;
			}
			else if (regex_match(a, picdetailreg))
			{
				pDlg->base64src = new char[a.length() + 1];
				::strcpy(pDlg->base64src, (char*)a.c_str());
				matchpic = true;
			}
		}
		CString t;
		pDlg->m_mailtext.GetWindowTextW(t);
		pDlg->m_mailtext.SetWindowTextW(t + L"\r\nPic Filename: \r\n" + char2CString(pDlg->picfilename));
		string base64str = pDlg->base64src;
		int pos;
		int count = 0;
		//去除base64编码中的回车换行
		while ((pos = base64str.find("\r\n")) >= 0)
		{
			base64str = base64str.erase(pos, 2);
			count++;
		}
		delete[]pDlg->base64src;
		pDlg->base64src = new char[base64str.length() + 1];
		::strcpy(pDlg->base64src, (char*)base64str.c_str());
		pDlg->picbuf = pDlg->base64_decode(pDlg->base64src);  //base64解码
		ofstream outFile(pDlg->picfilename, ios::out | ios::binary);
		outFile.write(pDlg->picbuf, pDlg->picbuflen); //将图片保存到本地，这样就从本地读取图片并加载
		outFile.close();
		if (!pDlg->hasimage)
		{
			pDlg->firstpicfile = pDlg->picfilename;
			pDlg->dealimage = false;
			pDlg->Invalidate(true);	//发送WM_PAINT消息加载图片
			pDlg->m_picture.UpdateWindow();//跳过消息队列
			pDlg->hasimage = true;
		}
	}

	CString t;
	pDlg->m_mailtext.GetWindowTextW(t);
	pDlg->m_mailtext.SetWindowTextW(t + L"\r\n");

	//如果是一个附件（也有可能是图片）
	for (int j = 0; j < attach.size(); j++)
	{
		CString judge;
		if (j == attach.size() - 1)
		{
			judge = mail.Mid(attach[j]);
		}
		else
		{
			judge = mail.Mid(attach[j], attach[j + 1] - attach[j]);
		}
		regex regpic("Content-Type: [\\w\\-\\/]*;\\s*name=\"(\\S*\\.\\S*)\"\\s*Content-Transfer-Encoding: base64\\r\\n(?:Content-ID: <.+>|Content-Disposition: attachment;\\r\\n\\tfilename=\".*\")\\r\\n\\r\\n([a-zA-Z0-9/=\\+\\r\\n]+)");
		regex regfiletxt("Content-Type: application/octet-stream;\\s*name=\"(\\S*\\.\\S*)\"\\s*Content-Transfer-Encoding: (?:7bit|quoted-printable)\\s*Content-Disposition: attachment;\\s*filename=\"(?:\\S*\\.\\S*)\"\\r\\n\\r\\n([\\S\\s]*)------");
		smatch result1, result2;
		string test(CString2char(judge));
		regex_search(test, result1, regpic);
		regex_search(test, result2, regfiletxt);
		bool matchfilename = false, matchpic = false, picmatch = false;
		for (int i = 0; i < result1.size(); i++)
		{
			if (matchfilename && matchpic)//确保只匹配第一个出现的图片
			{
				picmatch = true;
				break;
			}
			regex filenamereg("\\S*\\.\\S*"), picdetailreg("[a-zA-Z0-9/=\\+\\r\\n]+");
			string a = result1[i].str();
			if (regex_match(a, filenamereg))
			{
				pDlg->attachname = new char[a.length() + 1];
				::strcpy(pDlg->attachname, (char*)a.c_str());
				matchfilename = true;
			}
			else if (regex_match(a, picdetailreg))
			{
				pDlg->base64src = new char[a.length() + 1];
				::strcpy(pDlg->base64src, (char*)a.c_str());
				matchpic = true;
			}
		}
		if (matchpic)
		{
			CString t;
			pDlg->m_mailtext.GetWindowTextW(t);
			pDlg->m_mailtext.SetWindowTextW(t + L"\r\nAttachment Filename: \r\n" + char2CString(pDlg->attachname));
			string base64str = pDlg->base64src;
			int pos;
			int count = 0;
			//去除base64编码中的回车换行
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->base64src;
			pDlg->base64src = new char[base64str.length() + 1];
			::strcpy(pDlg->base64src, (char*)base64str.c_str());
			pDlg->picbuf = pDlg->base64_decode(pDlg->base64src);  //base64解码
			ofstream outFile(pDlg->attachname, ios::out | ios::binary);
			outFile.write(pDlg->picbuf, pDlg->picbuflen); //将附件保存到本地
			outFile.close();
			CImage ima;
			ima.Load(char2CString(pDlg->attachname));
			if (!ima.IsNull())	//尝试当做图片加载，如果不能加载，则不是图片
			{
				if (!pDlg->hasimage)  //如果可以加载，就看有没有显示过图片，没有就显示
				{
					pDlg->firstpicfile = pDlg->attachname;
					pDlg->dealimage = false;
					pDlg->Invalidate(true);	//发送WM_PAINT消息加载图片
					pDlg->m_picture.UpdateWindow();//跳过消息队列
					pDlg->hasimage = true;
				}
			}
			else
			{
				CString t;
				pDlg->m_mailtext.GetWindowTextW(t);
				pDlg->m_mailtext.SetWindowTextW(t + L"\r\nContent: \r\n" + char2CString(pDlg->picbuf));
			}
		}
		else
		{
			for (int i = 1; i < result2.size(); i++)
			{
				regex filen("\\S*\\.\\S*");
				string a = result2[i].str();
				if (regex_match(a, filen))
				{
					pDlg->attachname = new char[a.length() + 1];
					::strcpy(pDlg->attachname, (char*)a.c_str());
				}
				else
				{
					pDlg->base64src = new char[a.length() + 1];
					::strcpy(pDlg->base64src, (char*)a.c_str());
				}
			}
			ofstream outFile(pDlg->attachname, ios::out | ios::binary);
			outFile.write(pDlg->base64src, strlen(pDlg->base64src)); //将附件保存到本地
			outFile.close();
			CString temp;
			temp = char2CString(pDlg->base64src);
			temp = temp.Left(temp.GetLength() - 2);
			pDlg->base64src = CString2char(temp);
			CString t;
			pDlg->m_mailtext.GetWindowTextW(t);
			pDlg->m_mailtext.SetWindowTextW(t + L"\r\nAttachment Filename: \r\n" + char2CString(pDlg->attachname));
			pDlg->m_mailtext.GetWindowTextW(t);
			pDlg->m_mailtext.SetWindowTextW(t + L"\r\nContent: \r\n" + char2CString(pDlg->base64src));
		
		}
	}

	//如果是一个附件（也有可能是图片）
	for (int j = 0; j < attach2.size(); j++)
	{
		CString judge;
		if (j == attach2.size() - 1)
		{
			judge = mail.Mid(attach2[j]);
		}
		else
		{
			judge = mail.Mid(attach2[j], attach2[j + 1] - attach2[j]);
		}
		regex regpic("Content-Type: [\\w\\-\\/]*;\\s*name=\"(\\S*\\.\\S*)\"\\s*Content-Transfer-Encoding: base64\\r\\n(?:Content-ID: <.+>|Content-Disposition: attachment;\\r\\n\\tfilename=\".*\")\\r\\n\\r\\n([a-zA-Z0-9/=\\+\\r\\n]+)");
		regex regfiletxt("Content-Type: application/octet-stream;\\s*name=\"(\\S*\\.\\S*)\"\\s*Content-Transfer-Encoding: (?:7bit|quoted-printable)\\s*Content-Disposition: attachment;\\s*filename=\"(?:\\S*\\.\\S*)\"\\r\\n\\r\\n([\\S\\s]*)------");
		smatch result1, result2;
		string test(CString2char(judge));
		regex_search(test, result1, regpic);
		regex_search(test, result2, regfiletxt);
		bool matchfilename = false, matchpic = false, picmatch = false;
		for (int i = 0; i < result1.size(); i++)
		{
			if (matchfilename && matchpic)//确保只匹配第一个出现的图片
			{
				picmatch = true;
				break;
			}
			regex filenamereg("\\S*\\.\\S*"), picdetailreg("[a-zA-Z0-9/=\\+\\r\\n]+");
			string a = result1[i].str();
			if (regex_match(a, filenamereg))
			{
				pDlg->attachname = new char[a.length() + 1];
				::strcpy(pDlg->attachname, (char*)a.c_str());
				matchfilename = true;
			}
			else if (regex_match(a, picdetailreg))
			{
				pDlg->base64src = new char[a.length() + 1];
				::strcpy(pDlg->base64src, (char*)a.c_str());
				matchpic = true;
			}
		}
		if (matchpic)
		{
			CString t;
			pDlg->m_mailtext.GetWindowTextW(t);
			pDlg->m_mailtext.SetWindowTextW(t + L"\r\nAttachment Filename: \r\n" + char2CString(pDlg->attachname));
			string base64str = pDlg->base64src;
			int pos;
			int count = 0;
			//去除base64编码中的回车换行
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->base64src;
			pDlg->base64src = new char[base64str.length() + 1];
			::strcpy(pDlg->base64src, (char*)base64str.c_str());
			pDlg->picbuf = pDlg->base64_decode(pDlg->base64src);  //base64解码
			ofstream outFile(pDlg->attachname, ios::out | ios::binary);
			outFile.write(pDlg->picbuf, pDlg->picbuflen); //将附件保存到本地
			outFile.close();
			CImage ima;
			ima.Load(char2CString(pDlg->attachname));
			if (!ima.IsNull())
			{
				if (!pDlg->hasimage)
				{
					pDlg->firstpicfile = pDlg->attachname;
					pDlg->dealimage = false;
					pDlg->Invalidate(true);	//发送WM_PAINT消息加载图片
					pDlg->m_picture.UpdateWindow();//跳过消息队列
					pDlg->hasimage = true;
				}
			}
			else
			{
				CString t;
				pDlg->m_mailtext.GetWindowTextW(t);
				pDlg->m_mailtext.SetWindowTextW(t + L"\r\nContent: \r\n" + char2CString(pDlg->picbuf));
			}
		}
		else
		{
			for (int i = 1; i < result2.size(); i++)
			{
				regex filen("\\S*\\.\\S*");
				string a = result2[i].str();
				if (regex_match(a, filen))
				{
					pDlg->attachname = new char[a.length() + 1];
					::strcpy(pDlg->attachname, (char*)a.c_str());
				}
				else
				{
					pDlg->base64src = new char[a.length() + 1];
					::strcpy(pDlg->base64src, (char*)a.c_str());
				}
			}
			ofstream outFile(pDlg->attachname, ios::out | ios::binary);
			outFile.write(pDlg->base64src, strlen(pDlg->base64src)); //将附件保存到本地
			outFile.close();
			CString temp;
			temp = char2CString(pDlg->base64src);
			temp = temp.Left(temp.GetLength() - 2);
			pDlg->base64src = CString2char(temp);
			CString t;
			pDlg->m_mailtext.GetWindowTextW(t);
			pDlg->m_mailtext.SetWindowTextW(t + L"\r\nAttachment Filename: \r\n" + char2CString(pDlg->attachname));
			pDlg->m_mailtext.GetWindowTextW(t);
			pDlg->m_mailtext.SetWindowTextW(t + L"\r\nContent: \r\n" + char2CString(pDlg->base64src));

		}
	}
	if (!pDlg->hasimage)
	{
		pDlg->dealimage = false;
		pDlg->Invalidate(true);
	}

	if (!pDlg->islookfile) //如果是刚接收的邮件，则把它存在队列中
	{
		pDlg->stock();
	}
	else
		pDlg->islookfile = false;
	pDlg->clearInfo();	//清除一些变量信息，为下次做准备
	return 0;
}

void CSMTPServerDlg::recvClient(CClientSocket *pSocket)
{
	//接收客户端消息并与其进行交互
	quit = false;
	memset(recvBuffer, 0, sizeof(recvBuffer));
	int ret = pSocket->Receive(recvBuffer, sizeof(recvBuffer) - 1);
	if (ret == SOCKET_ERROR)
	{
		//WSAEWOULDBLOCK：非阻塞模式下，请求的操作被阻塞，需等待再次调用
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			CString error;
			int errorcode = GetLastError();
			error.Format(L"Socket failed to receive: %d", errorcode);
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				errorcode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //Default language
				(LPTSTR)&lpMsgBuf,
				0,
				NULL
				);
			error = error + L"\n" + (LPCTSTR)lpMsgBuf;
			AfxMessageBox(error);
			pSocket->Close();
		}
	}
	else
	{
		if (isinputdata) //如果客户端正在发送邮件数据
		{
			delethis = false;
			recvBuffer[ret] = '\0'; //terminate the string
			CString temp = char2CString(recvBuffer), temp2;
			m_mailrecv.GetWindowTextW(temp2);
			temp2 += temp;
			m_mailrecv.SetWindowTextW(temp2);
			if (temp.Find(L"\r\n.\r\n") >= 0) //如果遇到单独的.和回车换行则停止接收邮件数据，返回接收成功
			{
				isinputdata = false;
				memset(sendBuffer, 0, sizeof(sendBuffer));
				sprintf(sendBuffer, "250 Message acceptd for delivery\r\n");
				ret = pSocket->Send(sendBuffer, strlen(sendBuffer));
				if (ret == SOCKET_ERROR)
				{
					//WSAEWOULDBLOCK：非阻塞模式下，请求的操作被阻塞，需等待再次调用
					if (GetLastError() != WSAEWOULDBLOCK)
					{
						CString error;
						int errorcode = GetLastError();
						error.Format(L"Socket failed to send: %d", errorcode);
						LPVOID lpMsgBuf;
						FormatMessage(
							FORMAT_MESSAGE_ALLOCATE_BUFFER |
							FORMAT_MESSAGE_FROM_SYSTEM |
							FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							errorcode,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //Default language
							(LPTSTR)&lpMsgBuf,
							0,
							NULL
							);
						error = error + L"\n" + (LPCTSTR)lpMsgBuf;
						AfxMessageBox(error);
						pSocket->Close();
					}
				}
				else
				{
					CString temp = char2CString(sendBuffer);
					temp = L"S: " + temp;
					m_loglist.InsertString(m_loglist.GetCount(), temp);
					pSocket->AsyncSelect(FD_READ); //设置事件为接收（OnReceive）
					AfxBeginThread(processMail, NULL, THREAD_PRIORITY_NORMAL); //处理邮件内容
				}
			}
		}
		else  //进入命令处理
		{
			CString tempp;
			CString deal = char2CString(recvBuffer);
			tempp = L"R: " + deal;
			m_loglist.InsertString(m_loglist.GetCount(), tempp);
			CString deal2 = deal.MakeLower();  //把接收到的内容转为小写再比较
			deal = char2CString(recvBuffer);
			if (!deal2.Find(L"helo") || !deal2.Find(L"ehlo"))
			{
				regex reg("(helo|ehlo) [\\S]+\\r\\n");
				string test(CString2char(deal2));
				memset(sendBuffer, 0, sizeof(sendBuffer));
				if (!regex_match(test, reg)) //如果不匹配正则表达式，则命令的语法错误
				{
					sprintf(sendBuffer, "501 Syntax error\r\n");
				}
				else
				{
					sprintf(sendBuffer, "250 Hello, pleased to meet you\r\n");
				}
			}
			else if (!deal2.Find(L"mail from"))
			{
				regex reg("mail from:[ ]?<\\w+([\\-\\+\\.']\\w+)*@\\w+([\\-\\.]\\w+)*(\\.\\w+([\\-\\.]\\w+)*)?>\\r\\n");
				string test(CString2char(deal2));
				memset(sendBuffer, 0, sizeof(sendBuffer));
				if (!regex_match(test, reg))
				{
					sprintf(sendBuffer, "501 Syntax error\r\n");
				}
				else
				{
					CString temp = deal.Mid(deal.Find(L"<") + 1);
					temp = temp.Left(temp.Find(L">"));
					from = CString2char(temp);
					sprintf(sendBuffer, "250 %s... Sender ok\r\n", from);
				}
			}
			else if (!deal2.Find(L"rcpt to"))
			{
				regex reg("rcpt to:[ ]?<\\w+([\\-\\+\\.']\\w+)*@\\w+([\\-\\.]\\w+)*(\\.\\w+([\\-\\.]\\w+)*)?>\\r\\n");
				string test(CString2char(deal2));
				memset(sendBuffer, 0, sizeof(sendBuffer));
				if (!regex_match(test, reg))
				{
					sprintf(sendBuffer, "501 Syntax error\r\n");
				}
				else
				{
					CString temp = deal.Mid(deal.Find(L"<") + 1);
					temp = temp.Left(temp.Find(L">"));
					to = CString2char(temp);
					sprintf(sendBuffer, "250 %s... Recipient ok\r\n", to);
				}
			}
			else if (!deal2.Find(L"data"))
			{
				if (!from || !to) //如果还未指定发送方和接收方，则返回命令序列错误
				{
					memset(sendBuffer, 0, sizeof(sendBuffer));
					sprintf(sendBuffer, "503 Wrong command order\r\n");
				}
				else
				{
					memset(sendBuffer, 0, sizeof(sendBuffer));
					if (deal2.GetLength() == 6)
					{
						sprintf(sendBuffer, "354 Enter mail, end with \".\" on a line by itself\r\n");
						isinputdata = true;	//进入接收数据模式
						hasimage = false;
						m_mailrecv.SetWindowTextW(L"");	//先清空界面显示的上次接收的邮件内容
						m_mailtext.SetWindowTextW(L"");
						m_lookmail.EnableWindow(FALSE);	//禁用两个按钮
						m_delemail.EnableWindow(FALSE);
					}
					else
					{
						sprintf(sendBuffer, "501 Syntax error\r\n");
					}
				}
			}
			else if (!deal2.Find(L"quit"))
			{
				memset(sendBuffer, 0, sizeof(sendBuffer));
				memset(sendBuffer, 0, sizeof(sendBuffer));
				if (deal2.GetLength() == 6)
				{
					sprintf(sendBuffer, "221 Quit, goodbye\r\n");
					start = false;
					quit = true; //关闭负责通信的套接字
				}
				else
				{
					sprintf(sendBuffer, "501 Syntax error\r\n");
				}
			}	
			else if (!deal2.Find(L"rset"))
			{
				memset(sendBuffer, 0, sizeof(sendBuffer));
				memset(sendBuffer, 0, sizeof(sendBuffer));
				if (deal2.GetLength() == 6)
				{
					sprintf(sendBuffer, "250 OK\r\n");
				}
				else
				{
					sprintf(sendBuffer, "501 Syntax error\r\n");
				}
			}
			else if (!deal2.Compare(L"\r\n") == 0)
			{
				memset(sendBuffer, 0, sizeof(sendBuffer));
				sprintf(sendBuffer, "250 Choose a command\r\n");
			}
			else  //如果不能匹配上述的任何一个命令，则返回命令未定义
			{
				memset(sendBuffer, 0, sizeof(sendBuffer));
				sprintf(sendBuffer, "502 Bad command\r\n");
			}
			ret = pSocket->Send(sendBuffer, strlen(sendBuffer)); //发送邮件的缓冲区大小应和发送内容有效长度一致
			if (ret == SOCKET_ERROR)
			{
				//WSAEWOULDBLOCK：非阻塞模式下，请求的操作被阻塞，需等待再次调用
				if (GetLastError() != WSAEWOULDBLOCK)
				{
					CString error;
					int errorcode = GetLastError();
					error.Format(L"Socket failed to send: %d", errorcode);
					LPVOID lpMsgBuf;
					FormatMessage(
						FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						errorcode,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //Default language
						(LPTSTR)&lpMsgBuf,
						0,
						NULL
						);
					error = error + L"\n" + (LPCTSTR)lpMsgBuf;
					AfxMessageBox(error);
					pSocket->Close();
				}
			}
			else
			{
				CString temp = char2CString(sendBuffer);
				temp = L"S: " + temp;
				m_loglist.InsertString(m_loglist.GetCount(), temp);
				pSocket->AsyncSelect(FD_READ); //设置接收消息类型，以免之后不再调用OnReceive()了
			}
		}
	}
}

BOOL CSMTPServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_SHOW);

	// TODO: 在此添加额外的初始化代码
	m_picture.GetClientRect(&m_picrect); //获取图片区域大小
	//从本地读取发送队列
	string folderPath = "queue";
	if (GetFileAttributesA(folderPath.c_str())== INVALID_FILE_ATTRIBUTES)
	{
		CString dir = char2CString((char *)folderPath.c_str());
		bool flag = CreateDirectory(dir, NULL);
	}
	vector<string> files;
	getfilebytime(folderPath, files); //获取文件夹中所有文件

	for (int i = 0; i < files.size(); i++)
	{
		ifstream infile(files[i].c_str(), ios::in | ios::binary);
		MailInfo *pinfo = new MailInfo;
		string data;
		int len = 0;
		infile.read((char *)&len, sizeof(int));
		pinfo->date = new char[len + 1];
		infile.read(pinfo->date, len);
		pinfo->date[len] = '\0';  //从这里读出的字符串没有'\0'需手动添加
		infile.read((char *)&len, sizeof(int));
		pinfo->from = new char[len + 1];
		infile.read(pinfo->from, len);
		pinfo->from[len] = '\0';
		infile.read((char *)&len, sizeof(int));
		pinfo->to = new char[len + 1];
		infile.read(pinfo->to, len);
		pinfo->to[len] = '\0';
		infile.read((char *)&len, sizeof(int));
		pinfo->mailstr = new char[len + 1];
		infile.read(pinfo->mailstr, len);
		pinfo->mailstr[len] = '\0';
		info.push_back(*pinfo);
		infile.close();
		m_queue.InsertString(m_queue.GetCount(), char2CString(pinfo->date));
	}

	if (!files.size())
	{
		m_lookmail.EnableWindow(FALSE);
		m_delemail.EnableWindow(FALSE);
	}
	else
	{
		m_queue.SetCurSel(0);
	}

	CString temp1, temp2;
	temp2.Format(L"%d", files.size());
	((CStatic*)GetDlgItem(IDC_TIPS))->GetWindowTextW(temp1);
	((CStatic*)GetDlgItem(IDC_TIPS))->SetWindowTextW(temp1 + temp2 + L"封邮件）");
	

	CString ip = L"127.0.0.1";
	int port = 25;
	if (!m_pSocket->Create(port, SOCK_STREAM, FD_ACCEPT, ip)) //FD_ACCEPT为设置的事件为接收连接的事件（OnAccept）
	{
		MessageBox(L"Create socket error!", L"SMTPServer", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	m_pSocket->Bind(port, ip);
	//监听，等待连接请求
	//nConnectionBacklog：挂起连接的队列可以增长到的最大长度。有效范围是从1到5（默认）。
	if (!m_pSocket->Listen())
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			CString error;
			int errorcode = GetLastError();
			error.Format(L"Socket failed to listen: %d", errorcode);
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				errorcode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //Default language
				(LPTSTR)&lpMsgBuf,
				0,
				NULL
				);
			error = error + L"\n" + (LPCTSTR)lpMsgBuf;
			AfxMessageBox(error);
			//m_pSocket->Close();
			return FALSE;
		}
	}
	else
	{
		m_loglist.InsertString(m_loglist.GetCount(), L"*** SMTP服务器准备好");
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSMTPServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSMTPServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	//将对图片区域的操作放入OnPaint中是因为OnPaint会重绘图片区域导致看不到图片
	if (delethis) //如果从队列中删除的邮件是显示在界面中的，就清除图片显示区
	{
		clearImage();
		return;
	}
	if (firstpicfile || dealimage)
	{
		loadImage();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSMTPServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSMTPServerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pSocket->Close();
	CDialogEx::OnClose();
}

void CSMTPServerDlg::clearImage()
{
	CRect rect;
	CDC *pDC = m_picture.GetDC();
	//CPen pen(PS_SOLID, 1, RGB(105, 105, 105));
	CPen pen(PS_SOLID, 1, RGB(240, 240, 240)); //设置边框和对话框背景色同色，这样就看不到边框了
	CBrush brush;
	brush.CreateSolidBrush(RGB(240, 240, 240));
	pDC->SelectObject(&pen);
	pDC->Rectangle(m_picrect);
	pDC->SelectObject(&brush);
	pDC->Rectangle(m_picrect);
	ReleaseDC(pDC);
}

void CSMTPServerDlg::loadImage()
{
	m_picture.UnLoad();	 //释放加载过的图片资源
	m_picture.SetPaintRect(&m_picrect);
	clearImage();
	int height, width;
	CRect rect;//定义矩形类
	CRect rect1;
	CImage image; //创建图片类
	if (dealimage)	//如果正在处理图片，则在图片框中显示loading
	{
		CDC *pDC = m_picture.GetDC();//获得pictrue控件的DC

		CFont font;
		font.CreateFont(26,                                    //   字体的高度   
			0,                                          //   字体的宽度  
			0,                                          //  nEscapement 
			0,                                          //  nOrientation   
			20,                                  //   nWeight   
			FALSE,                                      //   bItalic   
			FALSE,                                      //   bUnderline   
			0,                                                   //   cStrikeOut   
			ANSI_CHARSET,                             //   nCharSet   
			OUT_DEFAULT_PRECIS,                 //   nOutPrecision   
			CLIP_DEFAULT_PRECIS,               //   nClipPrecision   
			DEFAULT_QUALITY,                       //   nQuality   
			DEFAULT_PITCH | FF_SWISS,     //   nPitchAndFamily     
			_T("Microsoft Sans Serif"));

		pDC->SelectObject(&font);
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0, 0, 0));
		pDC->TextOut(m_picrect.right / 2 - 50, m_picrect.bottom / 2 - 25, L"Loading......");
		return;
	}
	CDC *pDC = m_picture.GetDC();//获得m_pictrue控件的DC
	CString filen = char2CString(firstpicfile);
	if (filen.Mid(filen.Find(L".") + 1).CompareNoCase(L"gif") == 0)	//如果是gif就使用CPictureEx类型的m_picture控件加载gif，因为CImage是看不到动图在动的
	{
		m_picture.GetClientRect(&rect); //获得m_pictrue控件所在的矩形区域					
		m_picture.Load(char2CString(firstpicfile));//加载GIF路径
		//m_picture.SetBgMode(CPictureEx::BackgroundMode::TransparentBg, RGB(0, 0, 0));
		pDC->SetBkColor(RGB(240, 240, 240));
		m_picture.SetPaintRect(&rect);
		m_picture.Draw();
	}
	else
	{
		image.Load(char2CString(firstpicfile));//加载图片
		if (image.IsNull())	 //因为有可能打开了非图片类型的附件，所以要先判断一下是不是图片
			return;
		height = image.GetHeight();//得到图片高度
		width = image.GetWidth();//得到图片宽度
		SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);
		//m_picrect也是m_picture的矩形区域，由于m_picture的加载会改变这个矩形区域的大小，所以要单独保存一个成员变量
		//这里可以对大图片实现缩放，这样显示效果就不会失真
		if (width <= m_picrect.Width() && height <= m_picrect.Width()) //如果图片的尺寸小于图片控件尺寸，则不缩放显示
		{
			rect1 = CRect(m_picrect.TopLeft(), CSize(width, height));
			image.StretchBlt(pDC->m_hDC, rect1, SRCCOPY); //将图片画到Picture控件表示的矩形区域
		}
		else//如果图片的尺寸大于图片控件的尺寸
		{
			float xScale = (float)m_picrect.Width() / (float)width;//求X方向的缩放因子
			float yScale = (float)m_picrect.Height() / (float)height;//求Y方向的缩放因子
			//为了图片在图片控件上显示不失真，采用X和Y方向上较小的缩放因子作为缩放因子，此时图片不失真，但是可能不会铺满整个图片控件区域
			float ScaleIndex = (xScale <= yScale ? xScale : yScale);
			//rect1 = CRect(rect.TopLeft(), CSize((int)width*xScale,(int)height*xScale));//此时图片失真，但是会铺满整个图片控件区域
			rect1 = CRect(m_picrect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex));
			image.StretchBlt(pDC->m_hDC, rect1, SRCCOPY); //将图片画到Picture控件表示的矩形区域
		}
	}	   
	ReleaseDC(pDC);//释放picture控件的DC
	dealimage = false; //图片处理结束
}


HBRUSH CSMTPServerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (pWnd->GetDlgCtrlID() == IDC_MAILRECV|| pWnd->GetDlgCtrlID() == IDC_MAILTEXT)
	{  //将接收到的邮件内容框和邮件文字信息框设置成白色
		pDC->SetTextColor(RGB(0, 0, 0));//设置文本颜色
		pDC->SetBkColor(RGB(255, 255, 255));//设置文本背景
		hbr = CreateSolidBrush(RGB(255, 255, 255));//控件的背景色
	}
	else if (pWnd->GetDlgCtrlID() == IDC_PICTURE)
	{
		pDC->SetBkColor(RGB(240, 240, 240));
		hbr = CreateSolidBrush(RGB(240, 240, 240));
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CSMTPServerDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}



void CSMTPServerDlg::OnBnClickedLookmail()
{
	// TODO: 在此添加控件通知处理程序代码
	islookfile = true;	//进入查看邮件模式
	delethis = false;
	int i;
	i = fileindex = m_queue.GetCurSel();
	mailstr = info[i].mailstr;
	from = info[i].from;
	to = info[i].to;
	AfxBeginThread(processMail, NULL, THREAD_PRIORITY_NORMAL);
}


void CSMTPServerDlg::OnBnClickedDelemail()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = m_queue.GetCurSel();
	string temp = info[i].date;
	int t;
	while ((t = temp.find(" ")) >= 0)
	{
		temp.replace(t, 1, "_");
	}
	temp.erase(temp.find(","), 1);
	temp.erase(temp.find("+"), 1);
	temp.replace(temp.find(":"), 1, "_");
	temp.replace(temp.find(":"), 1, "_");
	char *tt = new char[temp.length() + 1];
	sprintf(tt, "%s.bin", temp.c_str());
	string ttt = tt;
	ttt = "queue\\" + ttt;
	remove(ttt.c_str());  //删除对应的本地文件
	m_queue.DeleteString(i);

	//删除队列中对应的项
	vector<MailInfo>::iterator it;
	int j;
	for (it = info.begin(), j = 0; it != info.end(); it++, j++)
	{
		if (j == i)
		{
			info.erase(it);
			break;
		}
	}


	if (!m_queue.GetCount())
	{
		m_lookmail.EnableWindow(FALSE);
		m_delemail.EnableWindow(FALSE);
	}
	if (i == fileindex)	//如果要删除的邮件显示在界面中，则清空界面
	{
		m_mailrecv.SetWindowTextW(L"");
		m_mailtext.SetWindowTextW(L"");
		delethis = true;
		clearImage();
	}
	CString temp2;
	temp2.Format(L"（发送队列共有%d封邮件）", m_queue.GetCount());
	((CStatic*)GetDlgItem(IDC_TIPS))->SetWindowTextW(temp2);

	if (i >= m_queue.GetCount())
	{
		m_queue.SetCurSel(m_queue.GetCount() - 1);
	}
	else
	{
		m_queue.SetCurSel(i);
	}
}


void CSMTPServerDlg::OnCbnSelchangeEmailcontents()
{
	// TODO: 在此添加控件通知处理程序代码
	m_lookmail.EnableWindow(TRUE);
	m_delemail.EnableWindow(TRUE);
}
