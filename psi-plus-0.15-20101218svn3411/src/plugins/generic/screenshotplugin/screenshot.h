/*
 * screenshot.h - plugin
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

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QNetworkReply>
#include <QNetworkProxy>

#include "ui_screenshot.h"
#include "server.h"
#include "iconfactoryaccessinghost.h"
#include "toolbar.h"

class Screenshot : public QMainWindow
{
    Q_OBJECT

public:
	Screenshot(IconFactoryAccessingHost* icoHost_, QString aFileNameFormat = "pic-yyyyMMdd-hhmmss", QString aFormat = "png");
	~Screenshot();

	void init(int lineWidth, const QString& color, const QString& font);
	void setFormat(QString aFormat);
	void setFileNameFormat(QString aFileNameFormat);
	void setServersList(QStringList servers);
	void setImagePath(QString path);
	void setProxy(const QString& host, int port, const QString& user, const QString& pass, const QString& type);

protected:
	bool eventFilter(QObject *obj, QEvent *e);

public slots:
	void shootScreen();
	void openImage();

signals:
	void settingsChanged(QString, QVariant);

private slots:
	void saveScreenshot();
	void uploadScreenshot();
	void cancelUpload();
	void dataTransferProgress( qint64 done, qint64 total );
	void ftpReplyFinished();
	void httpReplyFinished(QNetworkReply*);
	//void replyError(QNetworkReply::NetworkError);
	void newScreenshot();
	void captureDesktop(int);
	//void captureWindow(int);
	void captureArea(int);	
	//void shootWindow();
	void shootArea();
	void screenshotCanceled();
	void pixmapAdjusted();
	void fixSizes();

private:
	void updateScreenshotLabel();
	void uploadFtp();
	void uploadHttp();
	void showError(QString error);
	void bringToFront();

	QPixmap originalPixmap;
	Ui::Screenshot ui_;
	QString format;
	QString fileNameFormat;
	QList<Server*> servers;
	QPointer<QNetworkAccessManager> manager;
	QUrl u;
	QByteArray ba;
	QNetworkProxy proxy_;

	int x, y, newX, newY, delay_;

	IconFactoryAccessingHost* icoHost;
};


#endif
