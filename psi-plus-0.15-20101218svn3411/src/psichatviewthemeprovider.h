/*
 * psichatviewthemeprovider.h - adapter for set of chatview themes
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


#ifndef PSICHATVIEWTHEMEPROVIDER_H
#define PSICHATVIEWTHEMEPROVIDER_H

#include <QPointer>
#include "psithemeprovider.h"

class ChatViewTheme;

class PsiChatViewThemeProvider : public PsiThemeProvider
{
	Q_OBJECT

public:
	PsiChatViewThemeProvider(QObject *parent_);

	const QString type() const;
	const QStringList themeIds() const;
	Theme* load(const QString &theme);
	bool loadCurrent();
	Theme *current() const;
	const QString title(const QString &themeId, Theme *theme = 0) const;
	QByteArray screenshot(const QString &themeId, Theme *theme) const;
	virtual int screenshotWidth() const;

	QString optionsName() const { return "ChatView"; }
	QString optionsDescription() const { return "Configure your chatview theme here"; }

signals:
	void themeChanged();

private slots:
	void optionChanged(const QString& option);

private:
	ChatViewTheme *curTheme;
	QMap<QString, QString> chatViewScripts;
};

#endif
