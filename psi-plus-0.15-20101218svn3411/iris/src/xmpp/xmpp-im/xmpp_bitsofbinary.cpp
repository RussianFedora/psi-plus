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

#include "xmpp_bitsofbinary.h"
#include "xmpp_xmlcommon.h"
#include "xmpp_client.h"
#include "xmpp_tasks.h"
#include <QCryptographicHash>

using namespace XMPP;

BoBData::BoBData() {

}

BoBData::BoBData(const QDomElement &e) {
	fromXml(e);
}

bool BoBData::isNull() const
{
	return _cid.isEmpty() || _data.isNull();
}

QString BoBData::cid() const
{
	return _cid;
}

void BoBData::setCid(const QString &cid)
{
	_cid = cid;
}

QByteArray BoBData::data() const
{
	return _data;
}

void BoBData::setData(const QByteArray &data)
{
	_data = data;
}

QString BoBData::type() const
{
	return _type;
}

void BoBData::setType(const QString &type)
{
	_type = type;
}

unsigned int BoBData::maxAge() const
{
	return _maxAge;
}

void BoBData::setMaxAge(unsigned int maxAge)
{
	_maxAge = maxAge;
}

void BoBData::fromXml(const QDomElement &data)
{
	_cid = data.attribute("cid");
	_maxAge = data.attribute("max-age").toInt();
	_type = data.attribute("type");
	_data = QCA::Base64().stringToArray(data.text().replace("\n","")).toByteArray();
}

QDomElement BoBData::toXml(QDomDocument *doc) const
{
	QDomElement data = doc->createElement("data");
	data.setAttribute("xmlns", "urn:xmpp:bob");
	data.setAttribute("cid", _cid);
	data.setAttribute("max-age", _maxAge);
	data.setAttribute("type", _type);
	data.appendChild(doc->createTextNode( QCA::Base64().arrayToString(_data) ));
	return data;
}



// ---------------------------------------------------------
// BoBCache
// ---------------------------------------------------------
BoBCache::BoBCache(QObject *parent)
	: QObject(parent)
{

}


//------------------------------------------------------------------------------
// BoBManager
//------------------------------------------------------------------------------
BoBManager::BoBManager(Client *client)
	: QObject(client)
	, _cache(0)
{
	new JT_BoBServer(client->rootTask());
}

void BoBManager::setCache(BoBCache *cache)
{
	_cache = cache;
}

BoBData BoBManager::bobData(const QString &cid)
{
	if (_cache) {
		return _cache->get(cid);
	}
	return BoBData();
}

BoBData BoBManager::makeBoBData(const QByteArray &data, const QString &type,
								unsigned int maxAge)
{
	BoBData b;
	b.setCid(QString("sha1+%1@bob.xmpp.org").arg(QString(
		QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex())));
	b.setData(data);
	b.setMaxAge(maxAge);
	b.setType(type);
	if (_cache) {
		_cache->put(b);
	}
	return b;
}

void BoBManager::append(const BoBData &data)
{
	if (!data.isNull() && _cache) {
		_cache->put(data);
	}
}
