/*
 * server.h - plugin
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

#ifndef SERVER_H
#define SERVER_H

#include <QListWidgetItem>

class Server : public QListWidgetItem
{
public:
	Server(QListWidget *parent = 0);
	QString displayName() { return displayName_; };
	QString url() { return url_; };
	QString userName() { return userName_; };
	QString password() { return password_; };
	/*QString proxyHost() { return proxyHost_; };
	int proxyPort() { return proxyPort_; };
	QString proxyUserPass() { return proxyUserPass_; };
	QString proxyUserName() { return proxyUserName_; };*/
	QString servPostdata() { return servPostdata_; };
	QString servFileinput() { return servFileinput_; };
	QString servRegexp() { return servRegexp_; };
	//QString servFilefilter() { return servFilefilter_; };
	void setServer(const QString& url, const QString& user = "", const QString& pass = "");
	void setServerData(const QString& post = "", const QString& fInput = "", const QString& reg = ""/*, QString fFilter = ""*/);
	//void setProxy(QString host, QString port = "3128", QString user = "", QString pass = "");
	void setDisplayName(QString n);
	void setUseProxy(bool use) { useProxy_ = use; };
	bool useProxy() { return useProxy_; };

	QString settingsToString();
	void setFromString(const QString& settings);

	static QString splitString();

private:
	QString displayName_;
	QString url_, userName_, password_;
	/*QString proxyHost_, proxyUserName_, proxyUserPass_;
	int proxyPort_;*/
	QString servPostdata_, servFileinput_, servRegexp_/*, servFilefilter_*/;
	bool useProxy_;

	void processOltSettingsString(QStringList l);
};

#endif // SERVER_H
