/*
 * jsutil.h
 * Copyright (C) 2009  Rion
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

#ifndef JSUTIL
#define JSUTIL

class QString;
class QVariant;

#include <QVariantMap>

class JSUtil
{
public:
	static QString map2json(const QVariantMap &);
	static QString variant2js(const QVariant &);
	/** Escapes " and \n  (for JS evaluation) */
	static void escapeString(QString &str);

	/** Escapes " and \n  (for JS evaluation) [overload] */
	static QString escapeStringCopy(QString str);
};


#endif