/*
 * bytearrayreply.h - Base class for QNetworReply'es returning QByteArray
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

#ifndef BYTEARRAYREPLY_H
#define BYTEARRAYREPLY_H

#include <QNetworkReply>
#include <QBuffer>


class ByteArrayReply : public QNetworkReply {

	Q_OBJECT

public:
	ByteArrayReply(const QByteArray& ba = QByteArray(), QObject * parent = 0);

	/** Construct IconReply that fails with ContentAccessDenied error */
	//ByteArrayReply();
	~ByteArrayReply();

	//reimplemented
	void abort();
	void close();
	qint64 readData(char *data, qint64 len);
	void setReadBufferSize(qint64 size);
	qint64 bytesAvailable() const;
	bool open(OpenMode mode);
	qint64 size() const;


	private
slots:

	/**
	 * Emits finished().
	 *
	 * We cannot emit finished() in constructor.
	 *
	 * To be precise QNetworkAccessManager::createRequest() must return before we emit finished()
	 * It's Qt bug.
	 */
	void dataReady();

private:
	/** Icon data */
	QBuffer dataBuffer_;
};

#endif
