/*
 * theme.cpp - base class for any theme
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

#include "theme.h"

#include <QFileInfo>

#ifndef NO_Theme_ZIP
#define Theme_ZIP
#endif

#ifdef Theme_ZIP
#	include "zip/zip.h"
#endif

void ThemeMetaData::init()
{
	name = QObject::tr("Unnamed");
	//version = "1.0";
	//description = "No description";
	//authors << "I. M. Anonymous";
	//creation = "1900-01-01";
	homeUrl = QString::null;
}

ThemeMetaData::ThemeMetaData()
{
	init();
}

ThemeMetaData::ThemeMetaData(const ThemeMetaData &from)
	: QSharedData(from)
{
	init();
	setInformation(from);
}

void ThemeMetaData::setInformation(const ThemeMetaData &from) {
	name = from.name;
	version = from.version;
	description = from.description;
	creation = from.creation;
	homeUrl = from.homeUrl;
	filename = from.filename;
	authors = from.authors;
	info = from.info;
}

QByteArray Theme::loadData(const QString &fileName, const QString &dir)
{
	QByteArray ba;

	QFileInfo fi(dir);
	if ( fi.isDir() ) {
		QFile file ( dir + '/' + fileName );
		if (!file.open(QIODevice::ReadOnly))
			return ba;

		ba = file.readAll();
	}
#ifdef Theme_ZIP
	else if ( fi.suffix() == "jisp" || fi.suffix() == "zip" || fi.suffix() == "theme" ) {
		UnZip z(dir);
		if ( !z.open() )
			return ba;

		QString n = fi.completeBaseName() + '/' + fileName;
		if ( !z.readFile(n, &ba) ) {
			n = "/" + fileName;
			z.readFile(n, &ba);
		}
	}
#endif

	return ba;
}

const QString &Theme::name() const
{
	return md->name;
}

const QString &Theme::version() const
{
	return md->version;
}

const QString &Theme::description() const
{
	return md->description;
}

/**
 * Returns the Theme authors list.
 */
const QStringList &Theme::authors() const
{
	return md->authors;
}

/**
 * Returns the Theme creation date.
 */
const QString &Theme::creation() const
{
	return md->creation;
}

const QString &Theme::homeUrl() const
{
	return md->homeUrl;
}

/**
 * Returns directory (or .zip/.jisp archive) name from which Theme was loaded.
 */
const QString &Theme::fileName() const
{
	return md->filename;
}

/**
 * Sets the Theme directory (.zip archive) name.
 */
void Theme::setFileName(const QString &f)
{
	md->filename = f;
}

/**
 * Sets the information (meta-data) of this Theme to the information from the given Theme.
 */
void Theme::setInformation(const Theme &from)
{
	md.detach();
	md->setInformation( *(from.md) );
}

/**
 * Returns additional Theme information.
 * \sa setInfo()
 */
const QHash<QString, QString> Theme::info() const
{
	return md->info;
}

/**
 * Sets additional Theme information.
 * \sa info()
 */
void Theme::setInfo(const QHash<QString, QString> &i)
{
	md->info = i;
}