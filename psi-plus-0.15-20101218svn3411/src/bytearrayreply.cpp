/*
 * bytearrayreply.cpp - Base class for QNetworReply'es returning QByteArray
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

#include "bytearrayreply.h"

#include <QTimer>

ByteArrayReply::ByteArrayReply(const QByteArray& ba, QObject * parent) : QNetworkReply(parent) {
	setOpenMode(QIODevice::ReadOnly | QIODevice::Unbuffered);

	dataBuffer_.setData(ba);
	dataBuffer_.open(QIODevice::ReadOnly);

	QTimer::singleShot(0, this, SLOT(dataReady()));
}

void ByteArrayReply::abort() {
}


void ByteArrayReply::close() {
}


qint64 ByteArrayReply::bytesAvailable() const {
	return dataBuffer_.bytesAvailable();
}


qint64 ByteArrayReply::readData(char *data, qint64 len) {
	return dataBuffer_.read(data, len);
}


void ByteArrayReply::setReadBufferSize(qint64 size) {
	Q_UNUSED(size);
	Q_ASSERT(0); //should never happened
}


ByteArrayReply::~ByteArrayReply() {
}


void ByteArrayReply::dataReady() {

	qint64 totalSize = dataBuffer_.bytesAvailable();

	if (!totalSize) { //not in IconServer or (internet) access denied
		setError(QNetworkReply::ContentAccessDenied, "Access Denied");
		emit error(QNetworkReply::ContentAccessDenied);
		emit finished();
		return;
	}

	emit downloadProgress(0, totalSize);
	emit downloadProgress(totalSize, totalSize);
	emit readyRead();
	emit finished();
}


bool ByteArrayReply::open(OpenMode mode) {
	Q_UNUSED(mode);
	Q_ASSERT(0); //should never happened
	return true;
}


qint64 ByteArrayReply::size() const {
	return bytesAvailable(); //well, should never happended too
}
