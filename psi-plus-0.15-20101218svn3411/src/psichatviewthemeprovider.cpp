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


#include "psichatviewthemeprovider.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "chatviewtheme.h"
#include "psioptions.h"
#include "theme.h"
#include "applicationinfo.h"
#include "psiwkavatarhandler.h"

class PsiChatViewThemeProvider;
class PsiChatViewThemeUrlHandler : public NAMSchemeHandler
{
private:
	PsiChatViewThemeProvider *provider;
public:
	PsiChatViewThemeUrlHandler(PsiChatViewThemeProvider *provider_)
		: provider(provider_)
	{
	}
	QByteArray data(const QUrl &url) const
	{
		qDebug("loading theme file: %s", qPrintable(url.toString()));
		Theme *theme = provider->current();
		if (theme) {
			QByteArray td = Theme::loadData(url.path(), theme->fileName());
			qDebug("content of %d bytes", td.size());
			return td;
		}
		return QByteArray();
	}

};

PsiChatViewThemeProvider::PsiChatViewThemeProvider(QObject *parent_)
	: PsiThemeProvider(parent_)
	, curTheme(0)
{
	NetworkAccessManager::instance()->setSchemeHandler(
			"avatar", new PsiWKAvatarHandler((PsiCon*)parent()) );
	NetworkAccessManager::instance()->setSchemeHandler(
			"theme", new PsiChatViewThemeUrlHandler(this));
	connect(PsiOptions::instance(),
			SIGNAL(optionChanged(const QString&)),
			SLOT(optionChanged(const QString&)));
}

const QString PsiChatViewThemeProvider::type() const
{
	return "chatview";
}

const QStringList PsiChatViewThemeProvider::themeIds() const
{
	QStringList dirs;
	dirs << ":";
	dirs << ".";
	dirs << ApplicationInfo::homeDir();
	dirs << ApplicationInfo::resourcesDir();

	QSet<QString> ret;
	foreach (QString dir, dirs) {
		foreach (QFileInfo tDirInfo, QDir(dir+"/themes/chatview/")
			.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
			QString typeName = tDirInfo.fileName();
			foreach (QFileInfo themeInfo,
					QDir(tDirInfo.absoluteFilePath())
						.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot) +
					QDir(tDirInfo.absoluteFilePath())
						.entryInfoList(QStringList("*.theme"), QDir::Files)) {
				ret<<(QString("%1/%2").arg(typeName).arg(themeInfo.fileName()));
				//qDebug("found theme: %s", qPrintable(QString("%1/%2").arg(typeName).arg(themeInfo.fileName())));
			}
		}
	}

	return ret.values();
}

Theme * PsiChatViewThemeProvider::load(const QString &theme)
{
	QString up;
	if (chatViewScripts.value("util").isEmpty()) {
		if (!(up = themePath("chatview/util.js")).isEmpty()) {
			QFile file(up);
			if (file.open(QIODevice::ReadOnly)) {
				chatViewScripts["util"] = file.readAll();
			}
		}
	}
	if (!chatViewScripts.value("util").isEmpty()) {
		int pos;
		if ((pos = theme.indexOf('/')) > 0) {
			QString tp = themePath("chatview/" + theme);
			QString typeName = theme.mid(0, pos);
			if (!tp.isEmpty()) { // theme exists
				QString ap, ad;
				if (chatViewScripts.value(typeName).isEmpty() &&
					!(ap = themePath("chatview/" + typeName + "/adapter.js")).isEmpty()) {
					QFile afile(ap);
					if (afile.open(QIODevice::ReadOnly)) {
						chatViewScripts[typeName] = afile.readAll();
						ad = QFileInfo(afile).dir().path();
					}
				}
				if (!chatViewScripts.value(typeName).isEmpty()) {
					ChatViewTheme *theme = new ChatViewTheme();
					if (theme->load(tp, QStringList()<<chatViewScripts["util"]
									<<chatViewScripts[typeName], ad)) {
						return theme;
					}
				}
			}
		}
	}
	return 0;
}

bool PsiChatViewThemeProvider::loadCurrent()
{
	QString themeName = PsiOptions::instance()->getOption("options.ui.chat.theme").toString();
	ChatViewTheme *theme = 0;
	if (!(theme = (ChatViewTheme *)load(themeName))) {
		if (themeName != "psi/classic") {
			qDebug("fallback to classic chatview theme");
			if ( (theme = (ChatViewTheme *)load("psi/classic")) ) {
				PsiOptions::instance()->setOption("options.ui.chat.theme", "psi/classic");
			}
		}
	}
	if (theme) {
		if (curTheme) {
			delete curTheme;
		}
		curTheme = theme;
		emit themeChanged();
		return true;
	}
	return false;
}

Theme *PsiChatViewThemeProvider::current() const
{
	return curTheme;
}

const QString PsiChatViewThemeProvider::title(const QString &themeId, Theme *theme) const
{
	QString name = theme->name();
	return name.isEmpty()?themeId:name;
}

QByteArray PsiChatViewThemeProvider::screenshot(const QString &themeId,
												Theme *theme) const
{
	Q_UNUSED(themeId)
	return ((ChatViewTheme *)theme)->screenshot();
}

int PsiChatViewThemeProvider::screenshotWidth() const
{
	return 512;
}

void PsiChatViewThemeProvider::optionChanged(const QString& option)
{
	if (option == "options.ui.chat.theme") {
		loadCurrent();
	}
}
