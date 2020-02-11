
// SMTPServerDlg.h : 头文件
//

#pragma once
#include "ServerSocket.h"
#include "MyListBox.h"
#include "ClientSocket.h"
#include "afxwin.h"
#include <vector>
#include "CPictureEx.h"
using namespace std;

class MailInfo
{
public:
	char *from;
	char *to;
	char *date;
	char *subject;
	char *mailstr;
};

// CSMTPServerDlg 对话框
class CSMTPServerDlg : public CDialogEx
{
// 构造
public:
	CSMTPServerDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CSMTPServerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SMTPSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:				 
	CServerSocket *m_pSocket;
	afx_msg void OnClose();
	void greetClient(CClientSocket *pSocket);
	void recvClient(CClientSocket *pSocket);
	CMyListBox m_loglist;
	char sendBuffer[4096];
	char recvBuffer[4096];
	bool isinputdata;
	bool start;
	char *from;
	char *to;
	char *date;
	char *subject;
	CEdit m_mailrecv;
	//void processMail();
	CEdit m_mailtext;
	CPictureEx m_picture;
	char *picfilename;
	char *base64src;
	char *picbuf;
	char *content;
	char *mailstr;
	void loadImage();
	int picbuflen;
	char *base64_decode(char *src);
	vector<MailInfo> info;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	bool dealimage;
	void clearImage();
	void clearInfo();
	void stock();
	afx_msg LRESULT OnClearInfo(WPARAM wParam, LPARAM lParam);
	CButton m_lookmail;
	CButton m_delemail;
	afx_msg void OnBnClickedLookmail();
	afx_msg void OnBnClickedDelemail();
	bool islookfile;
	int fileindex;
	bool delethis;
	CRect m_picrect;
	CComboBox m_queue;
	char *txtcontent;
	char *txtfilename;
	char *attachname;
	vector<char *> txtfilen;
	vector<char *> attachn;
	vector<char *> picfilen;
	bool hasimage;
	char *firstpicfile;
	bool quit;
	afx_msg void OnCbnSelchangeEmailcontents();
};
