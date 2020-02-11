#pragma once
#include "ClientSocket.h"
// CServerSocket ÃüÁîÄ¿±ê

class CServerSocket : public CAsyncSocket
{
public:
	CServerSocket();
	virtual ~CServerSocket();
	virtual void OnAccept(int nErrorCode);
	CClientSocket *m_pSocket;
};


