#ifndef TCPSERVEREX_H
#define TCPSERVEREX_H

#include <QTcpServer>

class CTcpServerEx : public QTcpServer
{
	Q_OBJECT
public:
	CTcpServerEx(QObject* parent = nullptr);
	
protected:
	virtual void incomingConnection(qintptr socketDescriptor) override;
	
};

#endif // TCPSERVEREX_H
