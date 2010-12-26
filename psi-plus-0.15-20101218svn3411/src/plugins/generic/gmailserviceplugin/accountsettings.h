/*
 * accountsettings.cpp - plugin
 * Copyright (C) 2010 Khryukin Evgeny
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

#ifndef ACCOUNTSETTINGS_H
#define ACCOUNTSETTINGS_H

#include <QMap>
#include <QStringList>

class AccountSettings
{
public:
	AccountSettings(int acc = -1, QString j = QString());
	void fromString(const QString& settings);
	QString toString();

	int account;
	QString jid;
	QString fullJid;
	bool isMailEnabled;
	bool isMailSupported;
	bool isArchivingEnabled;
	bool isSuggestionsEnabled;
	bool notifyAllUnread;
	QString lastMailTime;
	QString lastMailTid;
	bool isSharedStatusEnabled;
	bool isSharedStatusSupported;
	QString status;
	QString message;
	QMap<QString, QStringList> sharedStatuses; // < staus, list of status messages >
	int listMax;
	int listContentsMax;
	int statusMax;
	bool isNoSaveSupported;
	bool isNoSaveEnbaled;
	QMap<QString, bool> noSaveList; // < jid, is no-save enabled >
};

#endif // ACCOUNTSETTINGS_H
