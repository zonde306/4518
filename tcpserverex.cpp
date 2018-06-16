#include "tcpserverex.h"
#include <QTcpSocket>
#include <QThread>

CTcpServerEx::CTcpServerEx(QObject* parent) : QTcpServer(parent)
{
	
}

void CTcpServerEx::incomingConnection(qintptr socketDescriptor)
{
	QTcpSocket* client = new QTcpSocket();
	connect(this, &CTcpServerEx::destroyed, client, &QTcpSocket::deleteLater);
	client->setSocketDescriptor(socketDescriptor);
	
	QThread* thread = new QThread(client);
	// connect(client, &QTcpSocket::disconnected, thread, &QThread::quit);
	
	addPendingConnection(client);
	client->moveToThread(thread);
	thread->start();
	
	emit newConnection();
}
