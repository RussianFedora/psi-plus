/*
 * screenshot.cpp - plugin
 * Copyright (C) 2009-2010  Khryukin Evgeny
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <QDesktopWidget>
#include <QBuffer>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QDateTime>
#include <QTimer>

#include "screenshot.h"
#include "screenshotoptions.h"

#define PROTOCOL_FTP "ftp"
#define PROTOCOL_HTTP "http"

Screenshot::Screenshot(IconFactoryAccessingHost* icoHost_, QString aFileNameFormat, QString aFormat)
	: QMainWindow()
	, format(aFormat)
	, fileNameFormat(aFileNameFormat)
	, icoHost(icoHost_)
{
	setAttribute(Qt::WA_DeleteOnClose);

	ui_.setupUi(this);
	ui_.progressBar->setVisible(false);
	ui_.lb_url->setVisible(false);
	ui_.pb_cancel->setVisible(false);

	delay_ = 0;
	proxy_ = QNetworkProxy();

	ui_.tb_bar->setIconHost(icoHost);
	ui_.tb_bar->init();

	ui_.lb_pixmap->setToolBar(ui_.tb_bar);

	ui_.pb_upload->setIcon(icoHost->getIcon("psi/upload"));
	ui_.pb_cancel->setIcon(icoHost->getIcon("psi/cancel"));
	ui_.pb_open->setIcon(icoHost->getIcon("psi/browse"));
	ui_.pb_save->setIcon(icoHost->getIcon("psi/download"));
	ui_.pb_new_screenshot->setIcon(icoHost->getIcon("screenshotplugin/screenshot"));

	ui_.pb_save->setShortcut(QKeySequence("Ctrl+s"));
	ui_.pb_upload->setShortcut(QKeySequence("Ctrl+u"));
	ui_.pb_open->setShortcut(QKeySequence("Ctrl+o"));
	ui_.pb_new_screenshot->setShortcut(QKeySequence("Ctrl+n"));

	connect(ui_.pb_save, SIGNAL(pressed()), this, SLOT(saveScreenshot()));
	connect(ui_.pb_upload, SIGNAL(pressed()), this, SLOT(uploadScreenshot()));
	connect(ui_.pb_new_screenshot, SIGNAL(pressed()), this, SLOT(newScreenshot()));
	connect(ui_.pb_cancel,SIGNAL(pressed()),this,SLOT(cancelUpload()));
	connect(ui_.pb_open, SIGNAL(pressed()), this, SLOT(openImage()));
	connect(ui_.lb_pixmap, SIGNAL(adjusted()), this, SLOT(pixmapAdjusted()));
	connect(ui_.lb_pixmap, SIGNAL(settingsChanged(QString,QVariant)), SIGNAL(settingsChanged(QString, QVariant)));

	setWindowIcon(icoHost->getIcon("screenshotplugin/screenshot"));
}

Screenshot::~Screenshot()
{

	if(manager)
		manager->deleteLater();
	foreach(Server *s, servers) {
		delete(s);
	}
	servers.clear();
	qApp->desktop()->releaseMouse();
	qApp->desktop()->removeEventFilter(this);	
	ui_.lb_pixmap->deleteLater();
}

void Screenshot::init(int lineWidth, const QString &color, const QString &font)
{
	ui_.lb_pixmap->init(lineWidth, color, font);
}

void Screenshot::setServersList(QStringList l)
{	
	ui_.cb_servers->clear();
	servers.clear();
	if(l.isEmpty()) {
		ui_.cb_servers->setEditable(false);
		ui_.pb_upload->setEnabled(false);
		return;
	}
	foreach(QString settings, l) {
		Server *s = new Server();
		s->setFromString(settings);
		servers.append(s);
		ui_.cb_servers->addItem(s->displayName());
	}	
}

void Screenshot::setProxy(const QString &host, int port, const QString &user, const QString &pass, const QString &type)
{
	proxy_ = QNetworkProxy(QNetworkProxy::HttpCachingProxy, host, port, user, pass);
	if(type == "socks")
		proxy_.setType(QNetworkProxy::Socks5Proxy);
}

void Screenshot::openImage()
{
	QString fileName = QFileDialog::getOpenFileName(0,tr("Open Image"),"",tr("Images (*.png *.gif *.jpg *.jpeg *.ico)"));
	if(!fileName.isEmpty())
		setImagePath(fileName);
}

void Screenshot::setImagePath(QString path)
{
	originalPixmap = QPixmap(path);
	updateScreenshotLabel();
}

void Screenshot::updateScreenshotLabel()
{
	ui_.lb_pixmap->setPixmap(originalPixmap);
}

void Screenshot::pixmapAdjusted()
{
	if(windowState() == Qt::WindowMaximized)
		return;

	QSize s = ui_.lb_pixmap->size();
	if(s.height() > 600 || s.width() > 800)
		resize(800,600);
	else {
		ui_.scrollArea->setMinimumSize(s + QSize(15,20)); //хак, для красивого уменьшения размера главного окна
		adjustSize();
		QTimer::singleShot(100, this, SLOT(fixSizes())); // необходимо время, чтобы ресайзить главное окно
	}
}

void Screenshot::fixSizes()
{
	ui_.scrollArea->setMinimumSize(0,0);
}

void Screenshot::uploadScreenshot()
{
	Server *s = servers.at(ui_.cb_servers->currentIndex());
	if(!s)
		return;

	QString scheme = QUrl(s->url()).scheme();
	ui_.pb_upload->setEnabled(false);
	ui_.pb_cancel->setVisible(true);
	ui_.cb_servers->setEnabled(false);

	originalPixmap = ui_.lb_pixmap->getPixmap();

	if (scheme.toLower() == QLatin1String(PROTOCOL_FTP)) {
		uploadFtp();
	} else if (scheme.toLower() == QLatin1String(PROTOCOL_HTTP)) {
		uploadHttp();
	}
	else
		cancelUpload();	
}

void Screenshot::cancelUpload()
{
	if(manager) {
		manager->disconnect();
		manager->deleteLater();
	}
	ui_.progressBar->hide();
	ui_.pb_upload->setEnabled(true);
	ui_.pb_cancel->setVisible(false);
	ui_.cb_servers->setEnabled(true);
}

void Screenshot::shootScreen()
{
	qApp->beep();
	originalPixmap = QPixmap(); // clear image for low memory situations
                                // on embedded devices.
	originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());		

	ui_.pb_new_screenshot->setEnabled(true);
	ui_.lb_url->setVisible(false);
	bringToFront();
	updateScreenshotLabel();
}

void Screenshot::bringToFront()
{
	QWidget* w = window();

	if(w->isMaximized())
		w->showMaximized();
	else
		w->showNormal();

	w->raise();
	w->activateWindow();
}

void Screenshot::newScreenshot()
{
	ScreenshotOptions *so = new ScreenshotOptions(delay_);
	connect(so, SIGNAL(captureArea(int)), this, SLOT(captureArea(int)));
	//connect(so, SIGNAL(captureWindow(int)), this, SLOT(captureWindow(int)));
	connect(so, SIGNAL(captureDesktop(int)), this, SLOT(captureDesktop(int)));
	connect(so,SIGNAL(screenshotCanceled()),this,SLOT(screenshotCanceled()));
	setWindowState(Qt::WindowMinimized);
	ui_.pb_new_screenshot->setEnabled(false);
	so->show();
}

void Screenshot::screenshotCanceled()
{
	ui_.pb_new_screenshot->setEnabled(true);
	setWindowState(Qt::WindowActive);
}

void Screenshot::captureArea(int delay)
{
	delay_ = delay;
	qApp->desktop()->installEventFilter(this);
	qApp->desktop()->grabMouse(QCursor(Qt::CrossCursor));
}

void Screenshot::shootArea()
{	
	qApp->beep();
	originalPixmap = QPixmap();
	originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId(), qMin(x,newX), qMin(y,newY), qAbs(newX-x), qAbs(newY-y));

	ui_.pb_new_screenshot->setEnabled(true);
	ui_.lb_url->setVisible(false);
	bringToFront();
	updateScreenshotLabel();
}

/*void Screenshot::captureWindow(int delay)
{
	QTimer::singleShot(delay*1000, this, SLOT(shootWindow()));
}

void Screenshot::shootWindow()
{	

	Window *w = reinterpret_cast<ulong *>(property(QX11Info::appRootWindow(),
						       NET_ACTIVE_WINDOW, XA_WINDOW));

	if(!w) {
		shootScreen();
		return;
	}

	qApp->beep();
	originalPixmap = QPixmap();
	originalPixmap = QPixmap::grabWindow(w);
	updateScreenshotLabel();

	ui_.pb_new_screenshot->setEnabled(true);
	bringToFront();
}*/

void Screenshot::captureDesktop(int delay)
{
	QTimer::singleShot(delay*1000, this, SLOT(shootScreen()));
}

void Screenshot::dataTransferProgress(qint64 done, qint64 total)
{
	ui_.progressBar->setMaximum(total);
	ui_.progressBar->setValue(done);
}

void Screenshot::uploadFtp()
{
	ba.clear();
	QBuffer buffer( &ba );
	buffer.open( QBuffer::ReadWrite );
	originalPixmap.save( &buffer , format.toAscii() );

	QString fileName = tr("%1.").arg(QDateTime::currentDateTime().toString(fileNameFormat)) + format;

	QFileInfo fi(fileName);
	fileName = fi.fileName();

	Server *s = servers.at(ui_.cb_servers->currentIndex());
	if(!s)
		cancelUpload();

	u.clear();
	u.setPort(21);
	u.setUrl(s->url(), QUrl::TolerantMode);
	u.setUserName(s->userName());
	u.setPassword(s->password());

	if(manager) {
		manager->disconnect();
		delete(manager);
	}

	manager = new QNetworkAccessManager(this);
	if(s->useProxy() && !proxy_.hostName().isEmpty()) {
		manager->setProxy(proxy_);
	}

	QString path = u.path();
	if(path.right(1) != "/")
		path += "/";
	u.setPath(path+fileName);
	QNetworkReply *reply = manager->put(QNetworkRequest(u), ba);

	connect(reply, SIGNAL(uploadProgress(qint64 , qint64)), this, SLOT(dataTransferProgress(qint64 , qint64)));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(finished()), this, SLOT(ftpReplyFinished()));

	ui_.progressBar->setValue(0);
	ui_.progressBar->show();
	ui_.lb_url->setVisible(false);
}

void Screenshot::uploadHttp()
{
	ba.clear();
	u.clear();

	QString boundary = "AaB03x";
	QString filename = tr("%1.").arg(QDateTime::currentDateTime().toString(fileNameFormat)) + format;

	Server *s = servers.at(ui_.cb_servers->currentIndex());
	if(!s)
		cancelUpload();

	if (s->servPostdata().length()>0) {
		foreach (QString poststr, s->servPostdata().split("&")) {
			QStringList postpair = poststr.split("=");
			if(postpair.count() < 2)
				continue;
			ba.append("--" + boundary + "\r\n");
			ba.append("Content-Disposition: form-data; name=\"" + postpair[0] + "\"\r\n");
			ba.append("\r\n" + postpair[1] + "\r\n");
		}
	}

	ba.append("--" + boundary + "\r\n");
	ba.append("Content-Disposition: form-data; name=\"" + s->servFileinput() + "\"; filename=\"" + filename.toUtf8() + "\"\r\n");
	ba.append("Content-Transfer-Encoding: binary\r\n");
	ba.append("\r\n");

	QByteArray a;
	QBuffer buffer(&a);
	buffer.open( QBuffer::ReadWrite );
	originalPixmap.save( &buffer , format.toAscii() );
	ba.append(a);

	ba.append("\r\n--" + boundary + "--\r\n");

	if(manager) {
		manager->disconnect();
		delete(manager);
	}

	manager = new QNetworkAccessManager(this);

	if(s->useProxy() && !proxy_.hostName().isEmpty()) {
		manager->setProxy(proxy_);
	}

	QNetworkRequest netreq;
	netreq.setUrl(QUrl(s->url()));

	netreq.setRawHeader("User-Agent", "Psi-Plus Screenshot plugin");
	netreq.setRawHeader("Content-Type", "multipart/form-data, boundary=" + boundary.toLatin1());
	netreq.setRawHeader("Cache-Control", "no-cache");
	netreq.setRawHeader("Accept", "*/*");
	netreq.setRawHeader("Content-Length", QString::number(ba.length()).toLatin1());

	ui_.progressBar->setValue(0);
	ui_.progressBar->show();
	ui_.lb_url->setVisible(false);

	QNetworkReply* reply = manager->post(netreq, ba);
	connect(reply, SIGNAL(uploadProgress(qint64 , qint64)), this, SLOT(dataTransferProgress(qint64 , qint64)));
	//connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpReplyFinished(QNetworkReply*)));
}

void Screenshot::saveScreenshot()
{
	ui_.pb_save->setEnabled(false);
	originalPixmap = ui_.lb_pixmap->getPixmap();
	QString initialPath = QDir::currentPath() + tr("/%1.").arg(QDateTime::currentDateTime().toString(fileNameFormat)) + format;

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
							   initialPath,
							   tr("%1 Files (*.%2);;All Files (*)")
							   .arg(format.toUpper())
							   .arg(format));
	if (!fileName.isEmpty()) {
		originalPixmap.save(fileName, format.toAscii());
	}
	ui_.pb_save->setEnabled(true);
}

/*void Screenshot::replyError(QNetworkReply::NetworkError e)
{
	if(e != QNetworkReply::NoError) {
		showError(tr("Unable to connect to the server "));
		ui_.progressBar->hide();
		ui_.pb_upload->setEnabled(true);
		ui_.pb_cancel->setVisible(false);
		ui_.cb_servers->setEnabled(true);
	}
}*/

void Screenshot::ftpReplyFinished()
{
	QNetworkReply *r = (QNetworkReply*)sender();
	if(r->error() == QNetworkReply::NoError) {
		ui_.progressBar->hide();
		ui_.lb_url->setVisible(true);
		ui_.lb_url->setText(QString("<a href=\"%1\">%1</a>")
				    .arg(u.toString(QUrl::RemoveUserInfo | QUrl::StripTrailingSlash)));		
	}
	r->close();
	r->deleteLater();
	ui_.cb_servers->setEnabled(true);
	ui_.pb_upload->setEnabled(true);
	ui_.pb_cancel->setVisible(false);
}

void Screenshot::httpReplyFinished(QNetworkReply* reply)
{
	qDebug() << "reply";
	if(reply->error() != QNetworkReply::NoError) {
		showError(tr("Unable to connect to the server "));
		ui_.progressBar->hide();
		ui_.pb_upload->setEnabled(true);
		ui_.pb_cancel->setVisible(false);
		ui_.cb_servers->setEnabled(true);
		return;
	}

	if (reply->rawHeader("Location").length()>0) {
		QUrl netrequrl(reply->rawHeader("Location"));
		if (netrequrl.host().isEmpty()) netrequrl = QUrl("http://"+reply->url().encodedHost()+reply->rawHeader("Location"));
		QNetworkRequest netreq(netrequrl);
		manager->get(netreq);
	}
	else {
		Server *s = servers.at(ui_.cb_servers->currentIndex());
		QString page = reply->readAll();

		/*
		Код нужен для анализа html и нахождения ссылки на картинку
		QFile f("~/page.html");
		f.open(QIODevice::ReadWrite);
		QTextStream out(&f);
		out << page;
		f.close();
		*/

		QRegExp rx(s->servRegexp());
		ui_.progressBar->hide();
		ui_.lb_url->setVisible(true);
		if (rx.indexIn(page) != -1) {
			QString imageurl = rx.cap(1);
			ui_.lb_url->setText(QString("<a href=\"%1\">%1</a>").arg(imageurl));
		}
		else ui_.lb_url->setText(tr("Can't parse URL"));
		reply->close();
		reply->deleteLater();
		ui_.pb_upload->setEnabled(true);
		ui_.pb_cancel->setVisible(false);
		ui_.cb_servers->setEnabled(true);
	}
}

void Screenshot::showError(QString error)
{
	QMessageBox::warning(this, tr("Screenshot Plugin"),error);
}

void Screenshot::setFileNameFormat(QString aFileNameFormat)
{
	fileNameFormat = aFileNameFormat;
}

void Screenshot::setFormat(QString aFormat)
{
	format = aFormat;
}

bool Screenshot::eventFilter(QObject *obj, QEvent *e)
{
	if(obj == qApp->desktop()) {
		if(e->type() == QEvent::MouseButtonPress) {
			x = QCursor::pos().x();
			y = QCursor::pos().y();
		}
		else if(e->type() == QEvent::MouseButtonRelease) {
			qApp->desktop()->releaseMouse();
			qApp->desktop()->removeEventFilter(this);
			newX = QCursor::pos().x();
			newY = QCursor::pos().y();
			QTimer::singleShot(delay_*1000, this, SLOT(shootArea()));
		}
	}
	return QObject::eventFilter(obj, e);
};
