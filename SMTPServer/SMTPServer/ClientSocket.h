#pragma once

// CClientSocket ÃüÁîÄ¿±ê

class CClientSocket : public CAsyncSocket
{
public:
	CClientSocket();
	virtual ~CClientSocket();
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	char sendBuffer[4096];
	char recvBuffer[4096];
	bool isinputdata;
	bool start;
	char *from;
	char *to;
};


