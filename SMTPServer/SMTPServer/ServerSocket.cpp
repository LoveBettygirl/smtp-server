// ServerSocket.cpp : ʵ���ļ�
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


// CServerSocket ��Ա����


void CServerSocket::OnAccept(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���
	CSMTPServerDlg *pDlg = (CSMTPServerDlg*)AfxGetApp()->GetMainWnd();
	pDlg->m_loglist.InsertString(pDlg->m_loglist.GetCount(), L"$$$$$$$$$$$$$$$$$$$$$");
	pDlg->m_loglist.InsertString(pDlg->m_loglist.GetCount(), L"*** �յ���������");
	m_pSocket = new CClientSocket; //�½������շ���Ϣ���׽���
	if (!Accept(*m_pSocket)) //���տͻ�����������
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
			temp.Format(L"*** ��������ʧ�ܣ�������%d", errorcode);
			pDlg->m_loglist.InsertString(pDlg->m_loglist.GetCount(), temp);
			return;
		}
	}
	else
	{
		pDlg->m_loglist.InsertString(pDlg->m_loglist.GetCount(), L"*** ��������");
		//�����׽�����Ҫ���ܵ���Ϣ���ͣ�send�������ڶԿͻ��˷����ʺ���Ϣ
		m_pSocket->AsyncSelect(FD_WRITE);
	}
	CAsyncSocket::OnAccept(nErrorCode);
}
