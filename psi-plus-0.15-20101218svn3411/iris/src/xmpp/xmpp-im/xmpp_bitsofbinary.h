/*
 * Copyright (C) 2010 Rion
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef XMPP_BITSOFBINARY_H
#define XMPP_BITSOFBINARY_H

#include <QDomElement>
#include <QObject>
#include "xmpp/jid/jid.h"

namespace XMPP
{
	class JT_BitsOfBinary;
	class Client;

	class BoBData
	{
	public:
		BoBData();
		BoBData(const QDomElement &);

		bool isNull() const;

		QString cid() const;
		void setCid(const QString &);

		QByteArray data() const;
		void setData(const QByteArray &);

		QString type() const;
		void setType(const QString &);

		unsigned int maxAge() const;
		void setMaxAge(unsigned int);

		void fromXml(const QDomElement &);
		QDomElement toXml(QDomDocument *doc) const;

	private:
		QByteArray _data;
		QString _type;
		QString _cid;
		unsigned int _maxAge;
	};



	class BoBCache : public QObject
	{
		Q_OBJECT

	public:
		BoBCache(QObject *parent);
		virtual void put(const BoBData &) = 0;
		virtual BoBData get(const QString &) = 0;
	};



	class BoBManager : public QObject
	{
		Q_OBJECT

	public:
		BoBManager(Client *);
		void setCache(BoBCache*);

		BoBData bobData(const QString &);
		BoBData makeBoBData(const QByteArray &data, const QString &type,
							unsigned int maxAge = 0);

		void append(const BoBData &);

	private:
		BoBCache *_cache;
	};

}

#endif // XMPP_BITSOFBINARY_H
