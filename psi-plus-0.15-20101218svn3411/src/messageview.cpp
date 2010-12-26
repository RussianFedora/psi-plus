/*
 * messageview.cpp - message data for chatview
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

#include "messageview.h"
#include "textutil.h"
#include "psioptions.h"

#include <QTextDocument>

static const QString me_cmd = "/me ";

MessageView::MessageView(Type t)
	: type_(t)
	, emote_(false)
	, alert_(false)
	, local_(false)
	, awaitingReceipt_(false)
	, spooled_(false)
	, dateTime_(QDateTime::currentDateTime())
{

}

MessageView MessageView::fromPlainText(const QString &text, Type type)
{
	MessageView mv(type);
	mv.setPlainText(text);
	return mv;
}

MessageView MessageView::fromHtml(const QString &text, Type type)
{
	MessageView mv(type);
	mv.setHtml(text);
	return mv;
}

MessageView MessageView::urlsMessage(const QMap<QString, QString> &urls)
{
	MessageView mv(Urls);
	mv.type_ = Urls;
	mv.urls_ = urls;
	return mv;
}

MessageView MessageView::subjectMessage(const QString &subject, const QString &prefix)
{
	MessageView mv(Subject);
	mv.text_ = Qt::escape(prefix);
	mv.usertext_ = subject;
	return mv;
}

// getters and setters

void MessageView::setPlainText(const QString &text)
{
	if (!text.isEmpty()) {
		if (type_ == Message) {
			emote_ = text.startsWith(me_cmd);
		}
		text_ = TextUtil::plain2rich(text);
		if (type_ == Message) {
			text_ = TextUtil::linkify(text_);
		}
	}
}

void MessageView::setHtml(const QString &text)
{
	if (type_ == Message) {
		QString str = TextUtil::rich2plain(text).trimmed();
		emote_ = str.startsWith(me_cmd);
		if(emote_) {
			setPlainText(str);
			return;
		}
	}
	text_ = text;
}

QString MessageView::text() const
{
	return text_;
}

QString MessageView::formattedText() const
{
	QString txt = text_;

	if (emote_ && type_ == Message) {
		int cmd = txt.indexOf(me_cmd);
		txt = txt.remove(cmd, me_cmd.length());
	}
	if (PsiOptions::instance()->getOption("options.ui.emoticons.use-emoticons").toBool())
		txt = TextUtil::emoticonify(txt);
	if (PsiOptions::instance()->getOption("options.ui.chat.legacy-formatting").toBool())
		txt = TextUtil::legacyFormat(txt);

	return txt;
}

QString MessageView::formattedUserText() const
{
	if (!usertext_.isEmpty()) {
		QString text = TextUtil::plain2rich(usertext_);
		text = TextUtil::linkify(text);
		if (PsiOptions::instance()->getOption("options.ui.emoticons.use-emoticons").toBool())
			text = TextUtil::emoticonify(text);
		if (PsiOptions::instance()->getOption("options.ui.chat.legacy-formatting").toBool())
			text = TextUtil::legacyFormat(text);
		return text;
	}
	return "";
}

void MessageView::setAlert(bool alert)
{
	alert_ = alert;
}

bool MessageView::isAlert() const
{
	return alert_;
}

void MessageView::setLocal(bool local)
{
	local_ = local;
}

bool MessageView::isLocal() const
{
	return local_;
}

void MessageView::setEmote(bool emote)
{
	emote_ = emote;
}

bool MessageView::isEmote() const
{
	return emote_;
}

void MessageView::setSpooled(bool spooled)
{
	spooled_ = spooled;
}

bool MessageView::isSpooled() const
{
	return spooled_;
}

void MessageView::setAwaitingReceipt(bool b)
{
	awaitingReceipt_ = b;
}

bool MessageView::isAwaitingReceipt() const
{
	return awaitingReceipt_;
}

void MessageView::setNick(const QString &nick)
{
	nick_ = nick;
}

QString MessageView::nick() const
{
	return nick_;
}

void MessageView::setMessageId(const QString &id)
{
	messageId_ = id;
}

QString MessageView::messageId() const
{
	return messageId_;
}

void MessageView::setUserId(const QString &id)
{
	userId_ = id;
}

QString MessageView::userId() const
{
	return userId_;
}

MessageView::Type MessageView::type() const
{
	return type_;
}

void MessageView::setDateTime(const QDateTime &dt)
{
	dateTime_ = dt;
}

QDateTime MessageView::dateTime() const
{
	return dateTime_;
}

QVariantMap MessageView::toVariantMap(bool isMuc, bool formatted) const
{
	QVariantMap m;
	m["time"] = dateTime_;
	switch (type_) {
		case Message:
			m["type"] = "message";
			m["message"] = formatted?formattedText():text_;
			m["emote"] = emote_;
			m["local"] = local_;
			m["sender"] = nick_;
			m["userid"] = userId_;
			m["spooled"] = spooled_;
			m["id"] = messageId_;
			if (isMuc) { // maybe w/o conditions ?
				m["alert"] = alert_;
			} else {
				m["awaitingReceipt"] = awaitingReceipt_;
			}
			break;
		case System:
			m["type"] = "system";
			m["message"] = formatted?formattedText():text_;
			m["usertext"] = formatted?formattedUserText():usertext_;
			break;
		case Subject:
			m["type"] = "subject";
			m["message"] = formatted?formattedText():text_;
			m["usertext"] = formatted?formattedUserText():usertext_;
			break;
		case Urls:
			m["type"] = "urls";
			QVariantMap vmUrls;
			foreach (const QString &u, urls_.keys()) {
				vmUrls[u] = urls_.value(u);
			}
			m["urls"] = vmUrls;
			break;
	}
	return m;
}
