/*
 * messageview.h - message data for chatview
 * Copyright (C) 2010 Rion
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

#ifndef MESSAGEVIEW_H
#define MESSAGEVIEW_H

#include <QDateTime>
#include <QVariantMap>

class MessageView
{
public:
	enum Type {
		Message,
		System,
		Subject,
		Urls
	};

	MessageView(Type);

	static MessageView fromPlainText(const QString &, Type);
	static MessageView fromHtml(const QString &, Type);
	static MessageView urlsMessage(const QMap<QString, QString> &);
	static MessageView subjectMessage(const QString &subject, const QString &prefix = QString());

	Type type() const;
	QString text() const;
	void setPlainText(const QString &);
	void setHtml(const QString &);
	QString formattedText() const;
	QString formattedUserText() const;

	void setAlert(bool);
	bool isAlert() const;
	void setLocal(bool);
	bool isLocal() const;
	void setEmote(bool);
	bool isEmote() const;
	void setSpooled(bool);
	bool isSpooled() const;
	void setAwaitingReceipt(bool);
	bool isAwaitingReceipt() const;
	void setNick(const QString &);
	QString nick() const;
	void setMessageId(const QString &);
	QString messageId() const;
	void setUserId(const QString &);
	QString userId() const;
	void setDateTime(const QDateTime &dt);
	QDateTime dateTime() const;
	QMap<QString, QString> urls() const { return urls_; }

	QVariantMap toVariantMap(bool isMuc, bool formatted = false) const;

private:
	Type type_;
	bool emote_;
	bool alert_;
	bool local_;
	bool spooled_;
	bool awaitingReceipt_;
	QString messageId_;
	QString userId_;
	QString nick_;
	QString text_;
	QString usertext_;
	QDateTime dateTime_;
	QMap<QString, QString> urls_;
};

#endif
