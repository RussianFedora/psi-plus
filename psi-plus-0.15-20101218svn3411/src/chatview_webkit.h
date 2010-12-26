/*
 * chatview_webkit.h - Webkit based chatview
 * Copyright (C) 2010  Rion
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

#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QWidget>
#include <QDateTime>
#include <QFrame>
#include <QPointer>
#include "webview.h"
#include "chatviewshare.h"

class ChatEdit;
class ChatView;
class MessageView;
class PsiAccount;
class ChatViewTheme;

class ChatViewJSObject : public QObject
{
	Q_OBJECT
public:
	ChatViewJSObject(QObject *view);

public slots:
	QString mucNickColor(QString nick, bool isSelf, QStringList validList = QStringList()) const;
	bool isMuc() const;
	QString chatName() const;
	QString jid() const;
	QString account() const;
	void signalInited();
	QString getFont() const;

signals:
	void inited();
};

class ChatView : public QFrame
{
	Q_OBJECT
	friend class ChatViewJSObject;
public:
	ChatView(QWidget* parent);
	~ChatView();

	void markReceived(QString id);

	// reimplemented
	QSize sizeHint() const;

	void setDialog(QWidget* dialog);
	void setSessionData(bool isMuc, const QString &jid, const QString name);
	void setAccount(PsiAccount *acc) { account_ = acc; }

	void contextMenuEvent(QContextMenuEvent* event);
	void sendJsObject(const QVariantMap &);
	void sendJsCommand(const QString &cmd);
	bool handleCopyEvent(QObject *object, QEvent *event, ChatEdit *chatEdit);

	void dispatchMessage(const MessageView &m);

	void scrollUp();
	void scrollDown();
	void clear();
	void doTrackBar();
	bool internalFind(QString str, bool startFromBeginning = false);
	WebView * textWidget();

protected:
	// override the tab/esc behavior
	bool focusNextPrevChild(bool next);
	void changeEvent(QEvent * event);
	//void keyPressEvent(QKeyEvent *);

protected slots:
	void psiOptionChanged(const QString &);
	//void autoCopy();

public slots:
	void init();

private slots:
	void embedJsObject();
	void checkJsBuffer();
	void sessionInited();

signals:
	void showNM(QString);

private:
	ChatViewTheme* currentTheme();

	WebView *webView;
	ChatViewJSObject *jsObject;
	QStringList jsBuffer_;
	bool sessionReady_;
	QPointer<QWidget> dialog_;
	ChatViewShare share_;
	bool isMuc_;
	QString jid_;
	QString name_;
	PsiAccount *account_;
};

#endif
