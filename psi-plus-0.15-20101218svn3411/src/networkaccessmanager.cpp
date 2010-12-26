/*
 * networkaccessmanager.cpp - Network Manager for WebView able to process
 * custom url schemas
 * Copyright (C) 2010 senu, Rion
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "networkaccessmanager.h"

#include "bytearrayreply.h"
#include <QCoreApplication>

NetworkAccessManager::NetworkAccessManager(QObject *parent)
: QNetworkAccessManager(parent) {
	setParent(QCoreApplication::instance());
}


NetworkAccessManager::~NetworkAccessManager() {
	schemeHandlers_.clear();
}


QNetworkReply * NetworkAccessManager::createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0) {
    //download local file
	qDebug("url: %s", qPrintable(req.url().toString()));
	if (req.url().scheme() == "file" || req.url().scheme() == "data") {
		return QNetworkAccessManager::createRequest(op, req, outgoingData);
    }

	if (schemeHandlers_.contains(req.url().scheme())) {
		ByteArrayReply *repl = new ByteArrayReply(schemeHandlers_.value(req.url().scheme())->data(req.url()), this);
		connect(repl, SIGNAL(finished()), SLOT(callFinished()));
		return repl;
	}

    //on whiteList?
    whiteListMutex.lock();

    bool whiteListed = whiteList.contains(req.url().toString());

    whiteListMutex.unlock();

    if (whiteListed) {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    }

    //deny all other access
    QNetworkRequest req2(req);

	QNetworkReply * reply = new ByteArrayReply(); //finishes with error
    connect(reply, SIGNAL(finished()), SLOT(callFinished()));

    return reply;
}


void NetworkAccessManager::callFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    
    if (reply) {
        emit finished(reply);
    }
}


void NetworkAccessManager::addUrlToWhiteList(const QString& url) {

    whiteListMutex.lock();
    whiteList.append(url);
    whiteListMutex.unlock();
}

QSharedPointer<NAMSchemeHandler> NetworkAccessManager::schemeHandler(const QString &scheme)
{
	if (schemeHandlers_.contains(scheme)) {
		return schemeHandlers_.value(scheme);
	}
	return QSharedPointer<NAMSchemeHandler>();
}

void NetworkAccessManager::setSchemeHandler(const QString &scheme, NAMSchemeHandler *handler)
{
	if (schemeHandlers_.contains(scheme)) {
		schemeHandlers_.remove(scheme);
	}
	schemeHandlers_.insert(scheme, QSharedPointer<NAMSchemeHandler>(handler));
}

/**
 * Returns the singleton instance of this class
 * \return Instance of NetworkAccessManager
 */
NetworkAccessManager* NetworkAccessManager::instance()
{
	if ( !instance_ )
		instance_ = new NetworkAccessManager();
	return instance_;
}

NetworkAccessManager* NetworkAccessManager::instance_ = NULL;
