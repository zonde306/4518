#include "netchan.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QRegExp>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QThread>

#include <sstream>
#include <algorithm>

#ifdef __MINGW32__
#include <tr1/functional>
#include <functional>
#else
#include <functional>
#endif

CNetChan::CNetChan(QObject *parent) : QObject(parent)
{
	m_pServer = new QTcpServer(this);
	m_pServer->setMaxPendingConnections(999);
	connect(m_pServer, &QTcpServer::newConnection, this, &CNetChan::OnNewConnect);
}

bool CNetChan::StartServer(quint16 port)
{
	if(!m_pServer->listen(QHostAddress::Any, port))
	{
		qDebug() << tr(u8"监听端口 %1 失败。").arg(QString::number(port));
		return false;
	}
	
	qDebug() << tr(u8"监听端口 %1 成功。").arg(QString::number(port));
	return true;
}

bool CNetChan::StopServer()
{
	m_pServer->close();
	return true;
}

bool CNetChan::IsRunning()
{
	return m_pServer->isListening();
}

void CNetChan::OnNewConnect()
{
	QTcpSocket* client = m_pServer->nextPendingConnection();
	connect(client, &QTcpSocket::readyRead, std::tr1::bind(&CNetChan::OnClientSendData, this, client));
	connect(client, &QTcpSocket::disconnected, std::tr1::bind(&CNetChan::OnClientCrash, this, client));
	
	qDebug() << tr(u8"客户端 %1 连接").arg(client->peerAddress().toString());
	m_conClient.insert(client->peerAddress().toString(), client);
	emit OnClientConnect(client);
}

void CNetChan::OnClientSendData(QTcpSocket* client)
{
	QString headers = client->readLine();
	QString body = client->readAll();
	static QRegExp reHeaders(u8"^(GET|POST|HEAD|OPTIONS|TRACE|PUT|DELETE|CONNECT)"
		u8" ([^ ]+)", Qt::CaseInsensitive);
	
	if(reHeaders.indexIn(headers) != 0)
	{
		qDebug() << tr(u8"无效的请求头部：") << headers;
		return;
	}
	
	QString mh = reHeaders.cap(1);
	if(!mh.compare(u8"GET", Qt::CaseInsensitive))
	{
		emit OnClientHttpGet(client, reHeaders.cap(2));
		
		/*
		std::string result =
			u8"<!DOCTYPE html>\r\n"
			u8"<html>\r\n"
			u8"<head>\r\n"
				u8"<meta charset=\"utf-8\">\r\n"
				u8"<title>这是一个标题</title>\r\n"
			u8"</head>\r\n"
			u8"<body>\r\n"
				u8"<h1>标题</h1>\r\n"
				u8"<p>\t\t正文</p>\r\n"
			u8"</body>\r\n"
			u8"</html>\r\n";
		
		std::stringstream ss;
		ss << u8"HTTP/1.1 200 Ok\r\n";
		ss << u8"Server: httpServer\r\n";
		ss << u8"Accept-Ranges: bytes";
		ss << u8"Content-Encoding: utf-8\r\n";
		ss << u8"Content-Length: " << result.size() << "\r\n";
		ss << u8"Content-Type: text/html\r\n";
		ss << "\r\n" << result << "\r\n";
		client->write(ss.str().c_str());
		*/
	}
	else if(!mh.compare(u8"POST", Qt::CaseInsensitive))
	{
		emit OnClientHttpPost(client, reHeaders.cap(2), ProccessPostData(body));
	}
	
	qDebug() << tr(u8"客户端 %1 请求 %2。").arg(client->peerAddress().toString(), headers);
	client->waitForBytesWritten();
	client->disconnectFromHost();
}

void CNetChan::OnClientCrash(QTcpSocket* client)
{
	auto it = m_conClient.find(client->peerAddress().toString());
	if(it != m_conClient.end())
	{
		qDebug() << tr(u8"客户端 %1 断开了连接").arg(it.key());
		m_conClient.erase(it);
	}
	
	emit OnClientDisonnect(client);
	// client->deleteLater();
}

QMap<QString, QString> CNetChan::ProccessPostData(const QString& data)
{
	QMap<QString, QString> keyValues;
	for(const QString& line : data.split("\r\n"))
	{
		static QRegExp rePost("^([^=:&]+)=([^=:&]+)", Qt::CaseInsensitive);
		if(line.isEmpty() || line.indexOf(rePost) != 0)
			continue;
		
		for(const QString& kv : line.split("&"))
		{
			QStringList kvs = kv.split("=");
			if(kvs.length() != 2)
				continue;
			
			keyValues.insert(kvs.at(0), kvs.at(1));
		}
	}
	
	return keyValues;
}

QString CNetChan::ProccessHeader()
{
	return "";
}
