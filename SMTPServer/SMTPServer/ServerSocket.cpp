// ServerSocket.cpp : 实现文件
//

#include "stdafx.h"
#include "SMTPServer.h"
#include "ServerSocket.h"
#include "SMTPServerDlg.h"


// CServerSocket

CServerSocket::CServerSocket()
{
	m_pSocket = NULL;
}

CServerSocket::~CServerSocket()
{
	if (m_pSocket)
	{
		delete m_pSocket;
	}
	if (m_hSocket != INVALID_SOCKET)
	{
		Close();
	}
}


// CServerSocket 成员函数


void CServerSocket::OnAccept(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	CSMTPServerDlg *pDlg = (CSMTPServerDlg*)AfxGetApp()->GetMainWnd();
	pDlg->m_loglist.InsertString(pDlg->m_loglist.GetCount(), L"$$$$$$$$$$$$$$$$$$$$$");
	pDlg->m_loglist.InsertString(pDlg->m_loglist.GetCount(), L"*** 收到连接请求");
	m_pSocket = new CClientSocket; //新建用于收发信息的套接字
	if (!Accept(*m_pSocket)) //接收客户端连接请求
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			CString error;
			int errorcode = GetLastError();
			error.Format(L"Socket failed to accept: %d", errorcode);
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
			delete m_pSocket;
			CString temp;
			temp.Format(L"*** 建立连接失败，错误码%d", errorcode);
			pDlg->m_loglist.InsertString(pDlg->m_loglist.GetCount(), temp);
			return;
		}
	}
	else
	{
		pDlg->m_loglist.InsertString(pDlg->m_loglist.GetCount(), L"*** 建立连接");
		//设置套接字需要接受的消息类型（send），用于对客户端发出问候消息
		m_pSocket->AsyncSelect(FD_WRITE);
	}
	CAsyncSocket::OnAccept(nErrorCode);
}
