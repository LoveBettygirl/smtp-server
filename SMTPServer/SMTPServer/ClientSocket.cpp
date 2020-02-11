// ClientSocket.cpp : ʵ���ļ�
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


// CClientSocket ��Ա����


void CClientSocket::OnReceive(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���
	CSMTPServerDlg *pDlg = (CSMTPServerDlg*)AfxGetApp()->GetMainWnd();
	pDlg->recvClient(this);
	if (pDlg->quit)
		Close();
	CAsyncSocket::OnReceive(nErrorCode);
}


void CClientSocket::OnSend(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���
	CSMTPServerDlg *pDlg = (CSMTPServerDlg*)AfxGetApp()->GetMainWnd();
	pDlg->greetClient(this);
	CAsyncSocket::OnSend(nErrorCode);
}
