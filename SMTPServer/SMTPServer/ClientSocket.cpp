// ClientSocket.cpp : 实现文件
//

#include "stdafx.h"
#include "SMTPServer.h"
#include "ClientSocket.h"
#include "SMTPServerDlg.h"


// CClientSocket

CClientSocket::CClientSocket()
{
}

CClientSocket::~CClientSocket()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		Close();
	}
}


// CClientSocket 成员函数


void CClientSocket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	CSMTPServerDlg *pDlg = (CSMTPServerDlg*)AfxGetApp()->GetMainWnd();
	pDlg->recvClient(this);
	if (pDlg->quit)
		Close();
	CAsyncSocket::OnReceive(nErrorCode);
}


void CClientSocket::OnSend(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	CSMTPServerDlg *pDlg = (CSMTPServerDlg*)AfxGetApp()->GetMainWnd();
	pDlg->greetClient(this);
	CAsyncSocket::OnSend(nErrorCode);
}
