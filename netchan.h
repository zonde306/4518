#ifndef NETCHAN_H
#define NETCHAN_H

#include <QMap>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class CNetChan : public QObject
{
	Q_OBJECT
public:
	CNetChan(QObject *parent = nullptr);
	
	bool StartServer(quint16 port);
	bool StopServer();
	bool IsRunning();
	
signals:
	void OnClientConnect(QTcpSocket* client = nullptr);
	void OnClientDisonnect(QTcpSocket* client = nullptr);
	void OnClientHttpGet(QTcpSocket* client = nullptr, const QString& url = "");
	void OnClientHttpPost(QTcpSocket* client = nullptr, const QString& url = "", const QMap<QString, QString>& fromData = QMap<QString, QString>());
	
private slots:
	void OnNewConnect();
	void OnClientSendData(QTcpSocket* client);
	void OnClientCrash(QTcpSocket* client);
	
private:
	QMap<QString, QString> ProccessPostData(const QString& data);
	QString ProccessHeader();
	
private:
	QTcpServer* m_pServer;
	QMap<QString, QTcpSocket*> m_conClient;
};

#endif // NETCHAN_H
