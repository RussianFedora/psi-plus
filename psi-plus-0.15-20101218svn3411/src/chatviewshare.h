/*
 * chatviewshare.h - shared part of any chatview
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

#ifndef CHATVIEWBASE_H
#define CHATVIEWBASE_H

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QStringList>

class ChatViewShare : public QObject
{
	Q_OBJECT
public:
	ChatViewShare(QObject *parent);
	bool updateLastMsgTime(QDateTime t);

protected:
	QDateTime lastMsgTime_;

private:
	int nickNumber_;
	QMap<QString,int> nicks;

public slots:
	QString getMucNickColor(QString, bool, QStringList validList = QStringList());
};

#endif
