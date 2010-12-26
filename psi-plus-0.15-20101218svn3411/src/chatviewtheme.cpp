/*
 * chatviewtheme.cpp - theme for webkit based chatview
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

#include "chatviewtheme.h"
#include "psioptions.h"
#include "jsutil.h"
#include "psiwkavatarhandler.h"
#include "webview.h"
#include <QWebPage>
#include <QWebFrame>
#include <QFileInfo>
#include <QApplication>


class ChatViewTheme;
class ChatViewTheme::Private
{
public:
	QString html;
	QString jsNamespace;
	QString adapterPath;
	QStringList scripts;
	QPointer<QObject> jso;
	QPointer<WebView> wv;
};

class ChatViewThemeJS : public QObject {
	Q_OBJECT
private:
	ChatViewTheme *theme;
	QMap<QString,QVariant> cache_;
public:
	ChatViewThemeJS(ChatViewTheme *theme_)
		: theme(theme_)
	{
	}

public slots:
	void setMetaData(const QVariantMap &map)
	{
		theme->md->name = map["name"].toString();
	}

	QString psiOption(const QString &option) const
	{
		return JSUtil::variant2js(PsiOptions::instance()->getOption(option));
	}

	QString getFileContents(const QString &name) const
	{
		return QString(Theme::loadData(name, theme->md->filename));
	}

	QString getFileContentsFromAdapterDir(const QString &name) const
	{
		QFile file(theme->d->adapterPath + "/" + name);
		if (file.open(QIODevice::ReadOnly)) {
			return file.readAll();
		}
		return QString();
	}

	void setHtml(const QString &h)
	{
		theme->d->html = h;
	}

	QString formatDate(const QDateTime &dt, const QString &format) const
	{
		return dt.toLocalTime().toString(format);
	}

	QString getPaletteTextcolor() const
	{
		return QApplication::palette().color(QPalette::Text).name();
	}

	void console(const QString &text) const
	{
		qDebug("%s", qPrintable(text));
	}

	// we don't need not text cache since binary data(images?)
	// most likely will be cached by webkit itself
	void toCache(const QString &name, const QVariant &data)
	{
		cache_.insert(name, data);
	}

	QVariant cache(const QString &name) const
	{
		return cache_.value(name);
	}

	void setDefaultAvatar(const QString &filename, const QString &host = QString())
	{
		QByteArray ba = Theme::loadData(filename, theme->md->filename);
		if (!ba.isEmpty()) {
			((PsiWKAvatarHandler *)NetworkAccessManager::instance()
				->schemeHandler("avatar").data())->setDefaultAvatar(ba, host);
		}
	}

	void setAvatarSize(int width, int height)
	{
		((PsiWKAvatarHandler *)NetworkAccessManager::instance()
				->schemeHandler("avatar").data())->setAvatarSize(QSize(width,
																	   height));
	}

	void setJSNamespace(const QString &ns)
	{
		theme->d->jsNamespace = ns;
	}

	const QString jsNamespace() const
	{
		return theme->d->jsNamespace;
	}

};


//------------------------------------------------------------------------------
// ChatViewTheme
//------------------------------------------------------------------------------
ChatViewTheme::ChatViewTheme()
{
	d = new Private;
	d->jsNamespace = "chat";
}

ChatViewTheme::~ChatViewTheme()
{
	if (!d->jso.isNull())
		delete d->jso;
	if (!d->wv.isNull())
		delete d->wv;
	delete d;
}

bool ChatViewTheme::load(const QString &file, const QStringList &helperScripts,
						 const QString &adapterPath)
{
	if (d->jso.isNull())
		d->jso = new ChatViewThemeJS(this);
	if (d->wv.isNull()) {
		d->wv = new WebView(0);
		d->wv->setHtml("", QUrl("theme://theme"));
	}
	d->adapterPath = adapterPath;
	md->filename = file;
	d->scripts = helperScripts;

	d->wv->page()->mainFrame()->addToJavaScriptWindowObject("chatServer", d->jso,
												QScriptEngine::QtOwnership);
	foreach (const QString &script, d->scripts) {
		d->wv->page()->mainFrame()->evaluateJavaScript(script);
	}
	QString resStr = d->wv->page()->mainFrame()->evaluateJavaScript("try { chat.adapter.loadTheme() } catch(e) { e.toString(); }").toString();
	//qDebug()<<"script returned: "<< qPrintable(resStr);

	bool res = (resStr == "ok");
	if (!res) {
		qWarning("javascript part of the theme loader "
				 "didn't return expected result: %s", qPrintable(resStr));
	}
	return res;
}

QByteArray ChatViewTheme::screenshot()
{
	return Theme::loadData("screenshot.png", md->filename);
}

QObject * ChatViewTheme::jsHelper()
{
	return d->jso;
}

QStringList ChatViewTheme::scripts()
{
	return d->scripts;
}

QString ChatViewTheme::html(QObject *session)
{
	if (d->html.isEmpty()) {
		if (session) {
			d->wv->page()->mainFrame()->addToJavaScriptWindowObject("chatSession", session);
		}
		return d->wv->page()->mainFrame()->evaluateJavaScript(
				"try { "+d->jsNamespace+".adapter.getHtml(); } catch(e) { e.toString(); }").toString();
	}
	return d->html;
}

QString ChatViewTheme::jsNamespace()
{
	return d->jsNamespace;
}


#include "chatviewtheme.moc"
