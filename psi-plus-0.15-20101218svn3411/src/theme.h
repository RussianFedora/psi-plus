/*
 * theme.h - base class for any theme
 * Copyright (C) 2010 Justin Karneges, Michail Pishchagin, Rion (Sergey Ilinyh)
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

#ifndef PSITHEME_H
#define PSITHEME_H

#include <QPixmap>
#include <QSharedData>
#include <QHash>

class ThemeMetaData : public QSharedData
{
protected:
	void init();

public:
	QString name, version, description, creation, homeUrl, filename;
	QStringList authors;
	QHash<QString, QString> info;

public:
	ThemeMetaData();
	ThemeMetaData(const ThemeMetaData &from);
	void setInformation(const ThemeMetaData &from);
	//QByteArray loadData(const QString &fileName, const QString &dir) const;
};

//-----------------------------------------------
// Theme
//-----------------------------------------------
class Theme {
public:
	Theme() {
		md = new ThemeMetaData;
	}

	static QByteArray loadData(const QString &fileName, const QString &dir);

	const QString &name() const;
	const QString &version() const;
	const QString &description() const;
	const QStringList &authors() const;
	const QString &creation() const;
	const QString &homeUrl() const;
	const QString &fileName() const;
	void setFileName(const QString &f);
	void setInformation(const Theme &from);
	const QHash<QString, QString> info() const;
	void setInfo(const QHash<QString, QString> &i);
protected:
	QSharedDataPointer<ThemeMetaData> md;
};


#endif
