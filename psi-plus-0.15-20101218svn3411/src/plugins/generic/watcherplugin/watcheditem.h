/*
 * watcheditem.h - plugin
 * Copyright (C) 2010  Khryukin Evgeny
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WATCHEDITEM_H
#define WATCHEDITEM_H

#include <QListWidgetItem>

const QString splitStr = "&split&";

class WatchedItem : public QListWidgetItem
{
public:
	WatchedItem(QListWidget *parent = 0);
	WatchedItem(const QString& jid, const QString& text = QString(), const QString& sFile = QString(), bool aUse = false, QListWidget *parent = 0);
	QString settingsString();
	void setSettings(const QString& settings);
	void setJid(const QString& jid) { jid_ = jid; };
	void setWatchedText(const QString& text) { text_ = text; };
	void setSFile(const QString& sFile) { sFile_ = sFile; };
	void setUse(bool use) { aUse_ = use; };
	void setGroupChat(bool gc) { groupChat_ = gc; };
	QString jid() { return jid_; };
	QString watchedText() { return text_; };
	QString sFile() { return sFile_; };
	bool alwaysUse() { return aUse_; };
	bool groupChat() { return groupChat_; };

	WatchedItem* copy();

private:
	QString jid_, text_, sFile_;
	bool aUse_, groupChat_;
};

#endif // WATCHEDITEM_H
