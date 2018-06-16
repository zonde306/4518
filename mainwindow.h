#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include "netchan.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
public slots:
	void OnSelectClicked();
	void OnServerClicked();
	void OnGetFile(QTcpSocket* client, QString url);
	
protected:
	void SendNotFound(QTcpSocket* client);
	void SendFile(QTcpSocket* client, const QString& path);
	std::string GetFileHttpType(const QString& fileExt);
	QString FindPathFile(const QString& fileName, QDir dir);
	
private:
	Ui::MainWindow *ui;
	CNetChan* server;
};

#endif // MAINWINDOW_H
