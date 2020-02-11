
// SMTPServerDlg.cpp : ʵ���ļ�
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
	//����char *�����С�����ֽ�Ϊ��λ��һ������ռ�����ֽ�
	int charLen = strlen(str);
	//������ֽ��ַ��Ĵ�С�����ַ����㡣
	int len = MultiByteToWideChar(CP_ACP, 0, str, charLen, NULL, 0);
	//Ϊ���ֽ��ַ���������ռ䣬�����СΪ���ֽڼ���Ķ��ֽ��ַ���С
	TCHAR *buf = new TCHAR[len + 1];
	//���ֽڱ���ת���ɿ��ֽڱ���
	MultiByteToWideChar(CP_ACP, 0, str, charLen, buf, len);
	buf[len] = '\0';  //����ַ�����β��ע�ⲻ��len+1
	//��TCHAR����ת��ΪCString
	CString pWideChar;
	pWideChar.Append(buf);
	//ɾ��������
	delete[]buf;
	return pWideChar;
}

char *CString2char(CString str)
{
	//ע�⣺����n��len��ֵ��С��ͬ,n�ǰ��ַ�����ģ�len�ǰ��ֽڼ����
	int n = str.GetLength();
	//��ȡ���ֽ��ַ��Ĵ�С����С�ǰ��ֽڼ����
	int len = WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), NULL, 0, NULL, NULL);
	//Ϊ���ֽ��ַ���������ռ䣬�����СΪ���ֽڼ���Ŀ��ֽ��ֽڴ�С
	char * p = new char[len + 1];  //���ֽ�Ϊ��λ
	//���ֽڱ���ת���ɶ��ֽڱ���
	WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), p, len, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str, str.GetLength() + 1, p, len + 1, NULL, NULL);
	p[len + 1] = '/0';  //���ֽ��ַ���'/0'����
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
	unordered_map<char, int> base64map;	//unordered_map�ĵײ��ǹ�ϣ��ʵ�ֵģ���ѯ�ٶȿ�
	int len = strlen(src), i, equalcount = 0;
	for (i = 0; i < 64; i++)
	{
		base64map[base64[i]] = i; //����base64�ַ���ֵ��ӳ�䣬�������
	}
	string binstr = "";
	//���ַ�תΪ��Ӧ��6λ�������ַ���
	for (i = 0; i < len; i++)
	{
		if (src[i] != '=')
		{
			//����base64�ַ���Ӧ��ֵ��6λ�������ַ���
			char c = base64map[src[i]];
			char bin[9] = { 0 };
			itoa(c, bin, 2);
			char tarbin[7] = { 0 };
			sprintf(tarbin, "%06s", bin);
			binstr.append(tarbin);
		}
		else
		{
			binstr.append("000000");  //���������β�ĵȺ���ֱ���ڶ������ַ������油��0
			equalcount++; //�ȺŸ�����1
		}
	}
	picbuflen = binstr.length() / 8; //ͼƬ���ֽڴ�СӦ�Ƕ������ַ������ȵ�1/8
	char *stream = new char[picbuflen + 1];
	int count = 0;
	for (i = 0; i < binstr.length(); i += 8) //����Ҫ����1���ֽڵ��ַ�������ѭ���Ĳ���Ϊ8
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
	picbuflen -= equalcount; //��ͳ�ƵĵȺ���Ŀ����ͼƬ��С
							//������Ҳ��Ӱ����ʾ�����������ܻ�������0�ֽ�
	return stream;
}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CSMTPServerDlg �Ի���



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


// CSMTPServerDlg ��Ϣ�������

void CSMTPServerDlg::clearInfo()
{
	//��մ˴ν��ջ�鿴�ʼ��Ļ������Ա��´ν����ʼ���鿴�ʼ�
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
	temp.Format(L"�����Ͷ��й���%d���ʼ���", m_queue.GetCount());
	((CStatic*)GetDlgItem(IDC_TIPS))->SetWindowTextW(temp);
}

void CSMTPServerDlg::stock()
{
	//���ʼ���Ϣ���뷢�Ͷ���
	//��vectorά��
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
	//����Ҫ���ʼ���Ϣ���뱾���ļ��������´δ򿪳���Ҳ���Լ�����Щ�ʼ�
	//��������Ҫ����Ϣ�ڴ��ʼ���ʱ���ٽ���
	ofstream outfile(fn.c_str(), ios::out | ios::binary);
	int len = strlen(date);
	outfile.write((char *)&len, sizeof(int)); //�����ַ���������Ϊ�˷�����ĸ�λ�ö�ȡ
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
	//�˺�����CClientSocket::OnSend()����
	if (!start)	//�����û�н��뵽�Ի�״̬������ͻ��˷����ʺ���Ϣ
	{
		sprintf(sendBuffer, "220 127.0.0.1 SMTP Mail Server\r\n");
		start = true;
	}
	else
		return;
	int ret = pSocket->Send(sendBuffer, strlen(sendBuffer));
	if (ret == SOCKET_ERROR)
	{
		//WSAEWOULDBLOCK��������ģʽ�£�����Ĳ�������������ȴ��ٴε���
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
		pSocket->AsyncSelect(FD_READ); //���ý�����Ϣ����Ϊreceive��׼����ͻ��˻Ự������ʼ�����
	}
}

//�����ʼ���Ϣ
UINT processMail(LPVOID hWnd)
{
	CSMTPServerDlg *pDlg = (CSMTPServerDlg*)AfxGetApp()->GetMainWnd(); //��ȡ�����ھ��
	pDlg->hasimage = false;
	pDlg->firstpicfile = NULL;
	pDlg->Invalidate(true);
	pDlg->m_lookmail.EnableWindow(FALSE);
	pDlg->m_delemail.EnableWindow(FALSE);
	pDlg->m_queue.EnableWindow(FALSE);
	CString mail;
	if (pDlg->islookfile) //������ڲ鿴�����е��ʼ����ʹӶ����л�ȡ���������ʼ�����
	{
		mail = char2CString(pDlg->mailstr);
		pDlg->m_mailrecv.SetWindowTextW(mail);
	}
	else  //��������ڽ����ʼ�����Ӵ����л�ȡ���������ʼ�����
	{
		pDlg->m_mailrecv.GetWindowTextW(mail);
		pDlg->mailstr = CString2char(mail);
	}

	//���������ͼƬ���п���������������������ͼƬ��ʾ������ʾLoading......
	if (mail.Find(L"Content-Type: image") >= 0 || mail.Find(L"Content-Type: application/octet-stream") >= 0)
	{
		pDlg->dealimage = true;
		pDlg->Invalidate(true);
		pDlg->m_picture.UpdateWindow();
	}

	pDlg->m_mailtext.SetWindowTextW(L"From: "+char2CString(pDlg->from)+
		L"\r\nTo: "+char2CString(pDlg->to)+L"\r\n"); //���ʼ�������Ϣ����ʾ���������ռ���

	if (mail.Find(L"Date: ") >= 0) //���ʼ�������Ϣ����ʾ����ʱ��
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

	if (mail.Find(L"Subject: ") >= 0) //���ʼ�������Ϣ����ʾ�ʼ�����
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
				//�жϱ������
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
		if (!isgb && !isutf8) //���û���κ�������ı�����Ϣ����ֱ����ʾ��������
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
			//ȥ��base64�����еĻس�����
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->subject;
			pDlg->subject = new char[base64str.length() + 1];
			::strcpy(pDlg->subject, (char*)base64str.c_str());
			pDlg->subject = pDlg->base64_decode(pDlg->subject);	 //base64����
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
	findall(mail, L"Content-Type: text/plain", txt);  //�ҳ���Ӧ�������Ӵ�������
	findall(mail, L"Content-Type: text/enriched", txt2);
	findall(mail, L"Content-Type: image", img);
	findall(mail, L"Content-Type: application/octet-stream", attach);
	findall(mail, L"Content-Type: application/x-msdownload", attach2);

	//���ʼ�������Ϣ����ʾ�ʼ��ı����ı�����
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
		//������Ӧoutlook��foxmail���ֿͻ���
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
				//ȥ��base64�����еĻس�����
				while ((pos = base64str.find("\r\n")) >= 0)
				{
					base64str = base64str.erase(pos, 2);
					count++;
				}
				delete[]pDlg->txtcontent;
				pDlg->txtcontent = new char[base64str.length() + 1];
				::strcpy(pDlg->txtcontent, (char*)base64str.c_str());
				pDlg->txtcontent = pDlg->base64_decode(pDlg->txtcontent);	//base64����
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
		if (!match1)  //�����������������
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
				outFile.write(pDlg->txtcontent, strlen(pDlg->txtcontent)); //��ͼƬ���浽���أ������ʹӱ��ض�ȡͼƬ������
				outFile.close();
			}  */
		}
		if (isbase64)	 //base64
		{
			string base64str = pDlg->content;
			int pos;
			int count = 0;
			//ȥ��base64�����еĻس�����
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->content;
			pDlg->content = new char[base64str.length() + 1];
			::strcpy(pDlg->content, (char*)base64str.c_str());
			pDlg->content = pDlg->base64_decode(pDlg->content);	//base64����
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

	//���ʼ�������Ϣ����ʾ�ʼ��ı����ı�����
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
		//������Ӧoutlook��foxmail���ֿͻ���
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
				//ȥ��base64�����еĻس�����
				while ((pos = base64str.find("\r\n")) >= 0)
				{
					base64str = base64str.erase(pos, 2);
					count++;
				}
				delete[]pDlg->txtcontent;
				pDlg->txtcontent = new char[base64str.length() + 1];
				::strcpy(pDlg->txtcontent, (char*)base64str.c_str());
				pDlg->txtcontent = pDlg->base64_decode(pDlg->txtcontent);	//base64����
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
		if (!match1)  //�����������������
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
			outFile.write(pDlg->txtcontent, strlen(pDlg->txtcontent)); //��ͼƬ���浽���أ������ʹӱ��ض�ȡͼƬ������
			outFile.close();
			}  */
		}
		if (isbase64)	 //base64
		{
			string base64str = pDlg->content;
			int pos;
			int count = 0;
			//ȥ��base64�����еĻس�����
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->content;
			pDlg->content = new char[base64str.length() + 1];
			::strcpy(pDlg->content, (char*)base64str.c_str());
			pDlg->content = pDlg->base64_decode(pDlg->content);	//base64����
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

	//���������ͼƬ������ж���ͼƬ������ʾ��һ��ͼƬ
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
			if (matchfilename && matchpic)//ȷ��ֻƥ���һ�����ֵ�ͼƬ
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
		//ȥ��base64�����еĻس�����
		while ((pos = base64str.find("\r\n")) >= 0)
		{
			base64str = base64str.erase(pos, 2);
			count++;
		}
		delete[]pDlg->base64src;
		pDlg->base64src = new char[base64str.length() + 1];
		::strcpy(pDlg->base64src, (char*)base64str.c_str());
		pDlg->picbuf = pDlg->base64_decode(pDlg->base64src);  //base64����
		ofstream outFile(pDlg->picfilename, ios::out | ios::binary);
		outFile.write(pDlg->picbuf, pDlg->picbuflen); //��ͼƬ���浽���أ������ʹӱ��ض�ȡͼƬ������
		outFile.close();
		if (!pDlg->hasimage)
		{
			pDlg->firstpicfile = pDlg->picfilename;
			pDlg->dealimage = false;
			pDlg->Invalidate(true);	//����WM_PAINT��Ϣ����ͼƬ
			pDlg->m_picture.UpdateWindow();//������Ϣ����
			pDlg->hasimage = true;
		}
	}

	CString t;
	pDlg->m_mailtext.GetWindowTextW(t);
	pDlg->m_mailtext.SetWindowTextW(t + L"\r\n");

	//�����һ��������Ҳ�п�����ͼƬ��
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
			if (matchfilename && matchpic)//ȷ��ֻƥ���һ�����ֵ�ͼƬ
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
			//ȥ��base64�����еĻس�����
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->base64src;
			pDlg->base64src = new char[base64str.length() + 1];
			::strcpy(pDlg->base64src, (char*)base64str.c_str());
			pDlg->picbuf = pDlg->base64_decode(pDlg->base64src);  //base64����
			ofstream outFile(pDlg->attachname, ios::out | ios::binary);
			outFile.write(pDlg->picbuf, pDlg->picbuflen); //���������浽����
			outFile.close();
			CImage ima;
			ima.Load(char2CString(pDlg->attachname));
			if (!ima.IsNull())	//���Ե���ͼƬ���أ�������ܼ��أ�����ͼƬ
			{
				if (!pDlg->hasimage)  //������Լ��أ��Ϳ���û����ʾ��ͼƬ��û�о���ʾ
				{
					pDlg->firstpicfile = pDlg->attachname;
					pDlg->dealimage = false;
					pDlg->Invalidate(true);	//����WM_PAINT��Ϣ����ͼƬ
					pDlg->m_picture.UpdateWindow();//������Ϣ����
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
			outFile.write(pDlg->base64src, strlen(pDlg->base64src)); //���������浽����
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

	//�����һ��������Ҳ�п�����ͼƬ��
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
			if (matchfilename && matchpic)//ȷ��ֻƥ���һ�����ֵ�ͼƬ
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
			//ȥ��base64�����еĻس�����
			while ((pos = base64str.find("\r\n")) >= 0)
			{
				base64str = base64str.erase(pos, 2);
				count++;
			}
			delete[]pDlg->base64src;
			pDlg->base64src = new char[base64str.length() + 1];
			::strcpy(pDlg->base64src, (char*)base64str.c_str());
			pDlg->picbuf = pDlg->base64_decode(pDlg->base64src);  //base64����
			ofstream outFile(pDlg->attachname, ios::out | ios::binary);
			outFile.write(pDlg->picbuf, pDlg->picbuflen); //���������浽����
			outFile.close();
			CImage ima;
			ima.Load(char2CString(pDlg->attachname));
			if (!ima.IsNull())
			{
				if (!pDlg->hasimage)
				{
					pDlg->firstpicfile = pDlg->attachname;
					pDlg->dealimage = false;
					pDlg->Invalidate(true);	//����WM_PAINT��Ϣ����ͼƬ
					pDlg->m_picture.UpdateWindow();//������Ϣ����
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
			outFile.write(pDlg->base64src, strlen(pDlg->base64src)); //���������浽����
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

	if (!pDlg->islookfile) //����Ǹս��յ��ʼ�����������ڶ�����
	{
		pDlg->stock();
	}
	else
		pDlg->islookfile = false;
	pDlg->clearInfo();	//���һЩ������Ϣ��Ϊ�´���׼��
	return 0;
}

void CSMTPServerDlg::recvClient(CClientSocket *pSocket)
{
	//���տͻ�����Ϣ��������н���
	quit = false;
	memset(recvBuffer, 0, sizeof(recvBuffer));
	int ret = pSocket->Receive(recvBuffer, sizeof(recvBuffer) - 1);
	if (ret == SOCKET_ERROR)
	{
		//WSAEWOULDBLOCK��������ģʽ�£�����Ĳ�������������ȴ��ٴε���
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
		if (isinputdata) //����ͻ������ڷ����ʼ�����
		{
			delethis = false;
			recvBuffer[ret] = '\0'; //terminate the string
			CString temp = char2CString(recvBuffer), temp2;
			m_mailrecv.GetWindowTextW(temp2);
			temp2 += temp;
			m_mailrecv.SetWindowTextW(temp2);
			if (temp.Find(L"\r\n.\r\n") >= 0) //�������������.�ͻس�������ֹͣ�����ʼ����ݣ����ؽ��ճɹ�
			{
				isinputdata = false;
				memset(sendBuffer, 0, sizeof(sendBuffer));
				sprintf(sendBuffer, "250 Message acceptd for delivery\r\n");
				ret = pSocket->Send(sendBuffer, strlen(sendBuffer));
				if (ret == SOCKET_ERROR)
				{
					//WSAEWOULDBLOCK��������ģʽ�£�����Ĳ�������������ȴ��ٴε���
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
					pSocket->AsyncSelect(FD_READ); //�����¼�Ϊ���գ�OnReceive��
					AfxBeginThread(processMail, NULL, THREAD_PRIORITY_NORMAL); //�����ʼ�����
				}
			}
		}
		else  //���������
		{
			CString tempp;
			CString deal = char2CString(recvBuffer);
			tempp = L"R: " + deal;
			m_loglist.InsertString(m_loglist.GetCount(), tempp);
			CString deal2 = deal.MakeLower();  //�ѽ��յ�������תΪСд�ٱȽ�
			deal = char2CString(recvBuffer);
			if (!deal2.Find(L"helo") || !deal2.Find(L"ehlo"))
			{
				regex reg("(helo|ehlo) [\\S]+\\r\\n");
				string test(CString2char(deal2));
				memset(sendBuffer, 0, sizeof(sendBuffer));
				if (!regex_match(test, reg)) //�����ƥ��������ʽ����������﷨����
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
				if (!from || !to) //�����δָ�����ͷ��ͽ��շ����򷵻��������д���
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
						isinputdata = true;	//�����������ģʽ
						hasimage = false;
						m_mailrecv.SetWindowTextW(L"");	//����ս�����ʾ���ϴν��յ��ʼ�����
						m_mailtext.SetWindowTextW(L"");
						m_lookmail.EnableWindow(FALSE);	//����������ť
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
					quit = true; //�رո���ͨ�ŵ��׽���
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
			else  //�������ƥ���������κ�һ������򷵻�����δ����
			{
				memset(sendBuffer, 0, sizeof(sendBuffer));
				sprintf(sendBuffer, "502 Bad command\r\n");
			}
			ret = pSocket->Send(sendBuffer, strlen(sendBuffer)); //�����ʼ��Ļ�������СӦ�ͷ���������Ч����һ��
			if (ret == SOCKET_ERROR)
			{
				//WSAEWOULDBLOCK��������ģʽ�£�����Ĳ�������������ȴ��ٴε���
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
				pSocket->AsyncSelect(FD_READ); //���ý�����Ϣ���ͣ�����֮���ٵ���OnReceive()��
			}
		}
	}
}

BOOL CSMTPServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	ShowWindow(SW_SHOW);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_picture.GetClientRect(&m_picrect); //��ȡͼƬ�����С
	//�ӱ��ض�ȡ���Ͷ���
	string folderPath = "queue";
	if (GetFileAttributesA(folderPath.c_str())== INVALID_FILE_ATTRIBUTES)
	{
		CString dir = char2CString((char *)folderPath.c_str());
		bool flag = CreateDirectory(dir, NULL);
	}
	vector<string> files;
	getfilebytime(folderPath, files); //��ȡ�ļ����������ļ�

	for (int i = 0; i < files.size(); i++)
	{
		ifstream infile(files[i].c_str(), ios::in | ios::binary);
		MailInfo *pinfo = new MailInfo;
		string data;
		int len = 0;
		infile.read((char *)&len, sizeof(int));
		pinfo->date = new char[len + 1];
		infile.read(pinfo->date, len);
		pinfo->date[len] = '\0';  //������������ַ���û��'\0'���ֶ����
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
	((CStatic*)GetDlgItem(IDC_TIPS))->SetWindowTextW(temp1 + temp2 + L"���ʼ���");
	

	CString ip = L"127.0.0.1";
	int port = 25;
	if (!m_pSocket->Create(port, SOCK_STREAM, FD_ACCEPT, ip)) //FD_ACCEPTΪ���õ��¼�Ϊ�������ӵ��¼���OnAccept��
	{
		MessageBox(L"Create socket error!", L"SMTPServer", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	m_pSocket->Bind(port, ip);
	//�������ȴ���������
	//nConnectionBacklog���������ӵĶ��п�������������󳤶ȡ���Ч��Χ�Ǵ�1��5��Ĭ�ϣ���
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
		m_loglist.InsertString(m_loglist.GetCount(), L"*** SMTP������׼����");
	}
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSMTPServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	//����ͼƬ����Ĳ�������OnPaint������ΪOnPaint���ػ�ͼƬ�����¿�����ͼƬ
	if (delethis) //����Ӷ�����ɾ�����ʼ�����ʾ�ڽ����еģ������ͼƬ��ʾ��
	{
		clearImage();
		return;
	}
	if (firstpicfile || dealimage)
	{
		loadImage();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSMTPServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSMTPServerDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_pSocket->Close();
	CDialogEx::OnClose();
}

void CSMTPServerDlg::clearImage()
{
	CRect rect;
	CDC *pDC = m_picture.GetDC();
	//CPen pen(PS_SOLID, 1, RGB(105, 105, 105));
	CPen pen(PS_SOLID, 1, RGB(240, 240, 240)); //���ñ߿�ͶԻ��򱳾�ɫͬɫ�������Ϳ������߿���
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
	m_picture.UnLoad();	 //�ͷż��ع���ͼƬ��Դ
	m_picture.SetPaintRect(&m_picrect);
	clearImage();
	int height, width;
	CRect rect;//���������
	CRect rect1;
	CImage image; //����ͼƬ��
	if (dealimage)	//������ڴ���ͼƬ������ͼƬ������ʾloading
	{
		CDC *pDC = m_picture.GetDC();//���pictrue�ؼ���DC

		CFont font;
		font.CreateFont(26,                                    //   ����ĸ߶�   
			0,                                          //   ����Ŀ��  
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
	CDC *pDC = m_picture.GetDC();//���m_pictrue�ؼ���DC
	CString filen = char2CString(firstpicfile);
	if (filen.Mid(filen.Find(L".") + 1).CompareNoCase(L"gif") == 0)	//�����gif��ʹ��CPictureEx���͵�m_picture�ؼ�����gif����ΪCImage�ǿ�������ͼ�ڶ���
	{
		m_picture.GetClientRect(&rect); //���m_pictrue�ؼ����ڵľ�������					
		m_picture.Load(char2CString(firstpicfile));//����GIF·��
		//m_picture.SetBgMode(CPictureEx::BackgroundMode::TransparentBg, RGB(0, 0, 0));
		pDC->SetBkColor(RGB(240, 240, 240));
		m_picture.SetPaintRect(&rect);
		m_picture.Draw();
	}
	else
	{
		image.Load(char2CString(firstpicfile));//����ͼƬ
		if (image.IsNull())	 //��Ϊ�п��ܴ��˷�ͼƬ���͵ĸ���������Ҫ���ж�һ���ǲ���ͼƬ
			return;
		height = image.GetHeight();//�õ�ͼƬ�߶�
		width = image.GetWidth();//�õ�ͼƬ���
		SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);
		//m_picrectҲ��m_picture�ľ�����������m_picture�ļ��ػ�ı������������Ĵ�С������Ҫ��������һ����Ա����
		//������ԶԴ�ͼƬʵ�����ţ�������ʾЧ���Ͳ���ʧ��
		if (width <= m_picrect.Width() && height <= m_picrect.Width()) //���ͼƬ�ĳߴ�С��ͼƬ�ؼ��ߴ磬��������ʾ
		{
			rect1 = CRect(m_picrect.TopLeft(), CSize(width, height));
			image.StretchBlt(pDC->m_hDC, rect1, SRCCOPY); //��ͼƬ����Picture�ؼ���ʾ�ľ�������
		}
		else//���ͼƬ�ĳߴ����ͼƬ�ؼ��ĳߴ�
		{
			float xScale = (float)m_picrect.Width() / (float)width;//��X�������������
			float yScale = (float)m_picrect.Height() / (float)height;//��Y�������������
			//Ϊ��ͼƬ��ͼƬ�ؼ�����ʾ��ʧ�棬����X��Y�����Ͻ�С������������Ϊ�������ӣ���ʱͼƬ��ʧ�棬���ǿ��ܲ�����������ͼƬ�ؼ�����
			float ScaleIndex = (xScale <= yScale ? xScale : yScale);
			//rect1 = CRect(rect.TopLeft(), CSize((int)width*xScale,(int)height*xScale));//��ʱͼƬʧ�棬���ǻ���������ͼƬ�ؼ�����
			rect1 = CRect(m_picrect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex));
			image.StretchBlt(pDC->m_hDC, rect1, SRCCOPY); //��ͼƬ����Picture�ؼ���ʾ�ľ�������
		}
	}	   
	ReleaseDC(pDC);//�ͷ�picture�ؼ���DC
	dealimage = false; //ͼƬ�������
}


HBRUSH CSMTPServerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if (pWnd->GetDlgCtrlID() == IDC_MAILRECV|| pWnd->GetDlgCtrlID() == IDC_MAILTEXT)
	{  //�����յ����ʼ����ݿ���ʼ�������Ϣ�����óɰ�ɫ
		pDC->SetTextColor(RGB(0, 0, 0));//�����ı���ɫ
		pDC->SetBkColor(RGB(255, 255, 255));//�����ı�����
		hbr = CreateSolidBrush(RGB(255, 255, 255));//�ؼ��ı���ɫ
	}
	else if (pWnd->GetDlgCtrlID() == IDC_PICTURE)
	{
		pDC->SetBkColor(RGB(240, 240, 240));
		hbr = CreateSolidBrush(RGB(240, 240, 240));
	}
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}


void CSMTPServerDlg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���

	//CDialogEx::OnOK();
}



void CSMTPServerDlg::OnBnClickedLookmail()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	islookfile = true;	//����鿴�ʼ�ģʽ
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	remove(ttt.c_str());  //ɾ����Ӧ�ı����ļ�
	m_queue.DeleteString(i);

	//ɾ�������ж�Ӧ����
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
	if (i == fileindex)	//���Ҫɾ�����ʼ���ʾ�ڽ����У�����ս���
	{
		m_mailrecv.SetWindowTextW(L"");
		m_mailtext.SetWindowTextW(L"");
		delethis = true;
		clearImage();
	}
	CString temp2;
	temp2.Format(L"�����Ͷ��й���%d���ʼ���", m_queue.GetCount());
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_lookmail.EnableWindow(TRUE);
	m_delemail.EnableWindow(TRUE);
}
