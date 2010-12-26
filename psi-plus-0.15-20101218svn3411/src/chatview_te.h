/*
 * chatview_te.h - subclass of PsiTextView to handle various hotkeys
 * Copyright (C) 2001-2010  Justin Karneges, Michail Pishchagin, Rion
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

#ifndef CHATVIEW_TE_H
#define CHATVIEW_TE_H

#include <QWidget>
#include <QDateTime>
#include <QPointer>

#include "psitextview.h"
#include "chatviewshare.h"

class ChatEdit;
class ChatViewBase;
class MessageView;

class ChatView : public PsiTextView
{
	Q_OBJECT
public:
	ChatView(QWidget* parent);
	~ChatView();

	void addLogIconsResources();
	void markReceived(QString id);

	// reimplemented
	QSize sizeHint() const;
	void clear();
	void contextMenuEvent(QContextMenuEvent *e);
	QMenu* createStandardContextMenu(const QPoint &position);

	void init();
	void setDialog(QWidget* dialog);
	void setSessionData(bool isMuc, const QString &jid, const QString name);

	void appendText(const QString &text);
	void dispatchMessage(const MessageView &);
	bool handleCopyEvent(QObject *object, QEvent *event, ChatEdit *chatEdit);

	void deferredScroll();
	void scrollUp();
	void scrollDown();
	void doTrackBar();
	bool internalFind(QString str, bool startFromBeginning = false);
	ChatView *textWidget();

protected:
	// override the tab/esc behavior
	bool focusNextPrevChild(bool next);
	void keyPressEvent(QKeyEvent *);

	QString formatTimeStamp(const QDateTime &time);
	QString colorString(bool local, bool spooled) const;
	void updateLastMsgTime(const QDateTime &t);

	void renderMucMessage(const MessageView &);
	void renderMessage(const MessageView &);
	void renderSysMessage(const MessageView &);
	void renderSubject(const MessageView &);
	void renderMucSubject(const MessageView &);
	void renderUrls(const MessageView &);

protected slots:
	void autoCopy();
	void quoteEvent();
	void changeActQuoteState();

private slots:
	void slotScroll();

signals:
	void showNM(QString);
	void quote(const QString &text);

private:
	bool isMuc_;
	QString jid_;
	QString name_;
	int  oldTrackBarPosition;
	QPointer<QWidget> dialog_;
	ChatViewShare share_;
	bool useMessageIcons_;

	QPixmap logIconSend;
	QPixmap logIconReceive;
	QPixmap logIconDelivered;
	QPixmap logIconTime;
	QPixmap logIconInfo;
	QAction *actQuote_;
};

#endif
