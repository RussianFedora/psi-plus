/*
 * contentdownloader.cpp - plugin interface
 * Copyright (C) 2010  Ivan Romanov <drizt@land.ru>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <QDebug>
#include <QNetworkInterface>
#include <QNetworkProxy>
#include <QList>
#include "contentdownloader.h"
#include "form.h"

Q_EXPORT_PLUGIN(ContentDownloader);

ContentDownloader::ContentDownloader() :
	enabled(false),
	form_(NULL)
{
}

ContentDownloader::~ContentDownloader()
{
}

// PsiPlugin
QString ContentDownloader::name() const
{
	return "Content Downloader Plugin";
}

QString ContentDownloader::shortName() const
{
	return "cdownloader";
}

QString ContentDownloader::version() const
{
	return "0.1.9";
}

QWidget *ContentDownloader::options()
{
	if ( !enabled ) {
		return 0;
	}

	if ( !appInfoHost || !psiOptions ) {
		return 0;
	}

	Proxy psiProxy = appInfoHost->getProxyFor(name());
	QNetworkProxy::ProxyType type;
	if(psiProxy.type == "socks") {
		type = QNetworkProxy::Socks5Proxy;
	} else {
		type = QNetworkProxy::HttpProxy;
	}
	
	QNetworkProxy proxy(type, psiProxy.host, psiProxy.port, psiProxy.user, psiProxy.pass);
	
	form_ = new Form();
	form_->setHomeDir(appInfoHost->appHomeDir());
	form_->setResourcesDir(appInfoHost->appResourcesDir());
	form_->setPsiOption(psiOptions);
	form_->setProxy(proxy);
	form_->update();
	return qobject_cast<QWidget*>(form_);
}

bool ContentDownloader::enable()
{
	if ( psiOptions ) {
		enabled = true;
	}

	appInfoHost->getProxyFor(name());
	return enabled;
}

bool ContentDownloader::disable()
{
	enabled = false;
	return true;
}

void ContentDownloader::applyOptions()
{
}

void ContentDownloader::restoreOptions()
{
}

void ContentDownloader::setOptionAccessingHost(OptionAccessingHost *host) 
{
	psiOptions = host;
}

void ContentDownloader::optionChanged(const QString &option)
{
	Q_UNUSED(option);
}

void ContentDownloader::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost *host)
{
	appInfoHost = host;
}

QString ContentDownloader::pluginInfo()
{
	return tr("Author: ") +	 "Ivan Romanov\n"
		   + tr("e-mail: ") + "drizt@land.ru\n\n"
		   + tr("This plugin is designed to make it easy to download and install iconsets and other resources for Psi+.");

}
