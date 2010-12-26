/*
 * psithemeprovider.h - kinda adapter for set of themes
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

#ifndef PSITHEMEPROVIDER_H
#define PSITHEMEPROVIDER_H

#include <QFuture>
#include "theme.h"

class Theme;

class PsiThemeProvider : public QObject
{
	Q_OBJECT

public:
	PsiThemeProvider(QObject *parent);

	virtual const QString type() const = 0;
	virtual Theme* load(const QString &) = 0;
	virtual const QStringList themeIds() const = 0;
	virtual bool loadCurrent() = 0;
	virtual Theme* current() const = 0;

	virtual bool threadedLoading() const;
	virtual const QString title(const QString &themeId, Theme *theme = 0) const;
	virtual QByteArray screenshot(const QString &themeId,
										Theme *theme = 0) const;
	virtual int screenshotWidth() const;

	virtual QString optionsName() const = 0;
	virtual QString optionsDescription() const = 0;

	QString themePath(const QString &name) const;
};

#endif
