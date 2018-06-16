#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTcpSocket>
#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include <QFile>
#include <sstream>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->pEditDir->setText(QDir::currentPath());
	
	server = new CNetChan(this);
	connect(server, &CNetChan::OnClientHttpGet, this, &MainWindow::OnGetFile);
	
	connect(ui->pBtnSelDir, &QPushButton::clicked, this, &MainWindow::OnSelectClicked);
	connect(ui->pBtnServer, &QPushButton::clicked, this, &MainWindow::OnServerClicked);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::OnSelectClicked()
{
	QString newPath = QFileDialog::getExistingDirectory(this, tr(u8"选择根目录"),
		ui->pEditDir->text());
	if(newPath.isEmpty())
		return;
	
	ui->pEditDir->setText(newPath);
}

void MainWindow::OnServerClicked()
{
	if(server->IsRunning())
	{
		server->StopServer();
		
		ui->pBtnSelDir->setDisabled(false);
		ui->pEditDir->setDisabled(false);
		ui->pEditPort->setDisabled(false);
		ui->pBtnServer->setText(tr(u8"启动"));
	}
	else
	{
		server->StartServer(ui->pEditPort->text().toUShort());
		
		ui->pBtnSelDir->setDisabled(true);
		ui->pEditDir->setDisabled(true);
		ui->pEditPort->setDisabled(true);
		ui->pBtnServer->setText(tr(u8"停止"));
	}
}

void MainWindow::OnGetFile(QTcpSocket* client, QString url)
{
	std::stringstream out;
	QDir root = ui->pEditDir->text();
	if(root.isEmpty() || !root.exists())
	{
		qDebug() << tr(u8"服务器根目录不存在");
		SendNotFound(client);
		return;
	}
	
	QString changeDir = url.section("/", 0, -2);
	if(!root.cd(changeDir))
	{
		qDebug() << tr(u8"无法到达路径：%1").arg(changeDir);
		SendNotFound(client);
		return;
	}
	
	QString fileName = url.section("/", -1);
	QString filePath = FindPathFile(fileName, root);
	if(filePath.isEmpty())
	{
		qDebug() << tr(u8"找不到文件：%1 在 %2").arg(fileName, root.path());
		// qDebug() << tr(u8"%1 %2").arg(changeDir.section(".", 0, 0), QString("index.html").section(".", -1));
		SendNotFound(client);
		return;
	}
	
	// 由于 Windows 系统使用的是反斜杠，在这里需要统一成普通斜杠
	SendFile(client, filePath.replace("\\", "/"));
}

void MainWindow::SendNotFound(QTcpSocket* client)
{
	std::stringstream out;
	out << u8"HTTP/1.1 404 Not Found\r\n";
	out << u8"Server: httpServer\r\n";
	client->write(out.str().c_str());
}

void MainWindow::SendFile(QTcpSocket* client, const QString& path)
{
	QFile file;
	std::stringstream out;
	file.setFileName(path);
	if(!file.open(QFile::ReadOnly))
	{
		out << u8"HTTP/1.1 500 Unknown\r\n";
		out << u8"Server: httpServer\r\n";
		client->write(out.str().c_str());
		return;
	}
	
	std::string fileType = GetFileHttpType(path.section(".", -1));
	out << u8"HTTP/1.1 200 Ok\r\n";
	out << u8"Server: httpServer\r\n";
	out << u8"Accept-Ranges: bytes";
	out << u8"Content-Encoding: utf-8\r\n";
	out << u8"Content-Length: " << file.size() << "\r\n";
	out << u8"Content-Type: " << fileType << "\r\n";
	
	if(fileType == u8"application/octet-stream")
	{
		out << u8"Content-Disposition: attachment;filename=" <<
			path.split("/").back().toStdString() << "\r\n";
	}
	
	out << "\r\n";
	out << QString(file.readAll()).toStdString() << "\r\n";
	client->write(out.str().c_str());
	file.close();
}

std::string MainWindow::GetFileHttpType(const QString& fileExt)
{
	if(!fileExt.compare(u8"htm", Qt::CaseInsensitive) ||
	   !fileExt.compare(u8"html", Qt::CaseInsensitive) ||
	   !fileExt.compare(u8"htx", Qt::CaseInsensitive) ||
	   !fileExt.compare(u8"jsp", Qt::CaseInsensitive) ||
	   !fileExt.compare(u8"php", Qt::CaseInsensitive))
		return u8"text/html";
	else if(!fileExt.compare(u8"png", Qt::CaseInsensitive))
		return u8"image/png";
	else if(!fileExt.compare(u8"jpg", Qt::CaseInsensitive))
		return u8"image/jpeg";
	else if(!fileExt.compare(u8"ico", Qt::CaseInsensitive))
		return u8"image/x-icon";
	else if(!fileExt.compare(u8"bmp", Qt::CaseInsensitive))
		return u8"application/x-bmp";
	else if(!fileExt.compare(u8"mp3", Qt::CaseInsensitive))
		return u8"audio/mp3";
	else if(!fileExt.compare(u8"mp4", Qt::CaseInsensitive))
		return u8"video/mpeg4";
	else if(!fileExt.compare(u8"avi", Qt::CaseInsensitive))
		return u8"video/avi";
	else if(!fileExt.compare(u8"wav", Qt::CaseInsensitive))
		return u8"audio/wav";
	else if(!fileExt.compare(u8"tif", Qt::CaseInsensitive))
		return u8"image/tiff";
	else if(!fileExt.compare(u8"gif", Qt::CaseInsensitive))
		return u8"image/gif";
	else if(!fileExt.compare(u8"wmv", Qt::CaseInsensitive))
		return u8"video/x-ms-wmv";
	else if(!fileExt.compare(u8"css", Qt::CaseInsensitive))
		return u8"text/css";
	else if(!fileExt.compare(u8"js", Qt::CaseInsensitive))
		return u8"application/x-javascript";
	
	/*
	else if(!fileExt.compare(u8"txt", Qt::CaseInsensitive))
		return u8"text/plain";
	*/
	
	return u8"application/octet-stream";
}

QString MainWindow::FindPathFile(const QString& fileName, QDir dir)
{
	dir.setFilter(QDir::Files);
	dir.setSorting(QDir::DirsLast);
	
	QFileInfoList fileList = dir.entryInfoList();
	bool isUnknownExt = (fileName.indexOf(".") == -1);
	
	auto it = std::find_if(fileList.begin(), fileList.end(),
		[&fileName, &isUnknownExt](const QFileInfo& info) -> bool
	{
		if(fileName.isEmpty() || fileName == "/")
		{
			return (!info.fileName().compare("index.html", Qt::CaseInsensitive) ||
				!info.fileName().compare("index.htm", Qt::CaseInsensitive));
		}
		else if(isUnknownExt)
		{
			QString ext = info.fileName().section(".", -1).toLower();
			return ((ext == u8"html" || ext == u8"htm" || ext == u8"htx") &&
				info.fileName().indexOf(fileName) == 0);
		}
		
		return !info.fileName().compare(fileName, Qt::CaseInsensitive);
	});
	
	if(it == fileList.end())
		return "";
	
	return it->filePath();
}

























