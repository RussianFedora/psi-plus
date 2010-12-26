/*
 * psithemeprovider.cpp - kinda adapter for set of themes
 * Copyright (C) 2010 Rion (Sergey Ilinyh)
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

#include "psithemeprovider.h"

#include <QFileInfo>
#include <QStringList>

#include "applicationinfo.h"


PsiThemeProvider::PsiThemeProvider(QObject *parent)
	: QObject(parent)
{

}

QString PsiThemeProvider::themePath(const QString &name) const {
	QStringList dirs;
	dirs << ":";
	dirs << ".";
	dirs << ApplicationInfo::homeDir();
	dirs << ApplicationInfo::resourcesDir();

	foreach (QString dir, dirs) {
		QString fileName = dir + "/themes/" + name;

		QFileInfo fi(fileName);
		if ( fi.exists() )
			return fileName;
	}

	qWarning("PsiThemeManager::Private::themePath(\"%s\"): not found", qPrintable(name));
	return QString();
}


// says where theme is able to load in separate thread
bool PsiThemeProvider::threadedLoading() const
{
	return false;
}

// theme title for theme list
const QString PsiThemeProvider::title(const QString &themeId, Theme *theme) const
{
	Q_UNUSED(theme)
	return themeId;
}

QByteArray PsiThemeProvider::screenshot(const QString &themeId,
												   Theme *theme) const
{
	Q_UNUSED(themeId)
	Q_UNUSED(theme)
	return QByteArray();
}

int PsiThemeProvider::screenshotWidth() const
{
	return 0;
}
