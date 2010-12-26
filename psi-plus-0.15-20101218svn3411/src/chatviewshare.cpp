/*
 * chatviewshare.cpp - shared part of any chatview
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

#include "chatviewshare.h"
#include "psioptions.h"
#include <QColor>

ChatViewShare::ChatViewShare(QObject *parent)
		: QObject(parent)
		, nickNumber_(0)
{

}

bool ChatViewShare::updateLastMsgTime(QDateTime t)
{
	bool doInsert = t.date() != lastMsgTime_.date();
	lastMsgTime_ = t;
	return doInsert;
}

QString ChatViewShare::getMucNickColor(QString nick, bool isSelf, QStringList validList)
{
	int sender;
	if(isSelf || nick.isEmpty()) {
		sender = -1;
	} else {
		if (!nicks.contains(nick)) {
			//not found in map
			nicks.insert(nick,nickNumber_);
			nickNumber_++;
		}
		sender=nicks[nick];
	}

	QStringList nickColors = validList.isEmpty()
		? PsiOptions::instance()->getOption("options.ui.look.colors.muc.nick-colors").toStringList()
		: validList;

	if(!PsiOptions::instance()->getOption("options.ui.muc.use-nick-coloring").toBool()) {
		return "#000000";
	} else {
		if (PsiOptions::instance()->getOption("options.ui.muc.use-hash-nick-coloring").toBool()) {
			/* Hash-driven colors */
			unsigned char hash[4];
			QByteArray cnick = nick.toLocal8Bit();
			unsigned nicklen, i, *h = reinterpret_cast<unsigned *>(hash);
			nicklen = cnick.length();
			*h = 12345;
			for (i = 0; i < nicklen; i++) *h *= cnick[i];
			/* I hope, nobody will use this on big-endian 64bit CPUs */
			QColor precolor;
			QString color;
			color.sprintf("#%02hhX%02hhX%02hhX", hash[0], hash[1], hash[2]);
			precolor.setNamedColor(QString(color));
			if ( precolor.saturation() < 150 )
			    precolor.setHsv( precolor.hue(), 250, precolor.value());
			if ( precolor.value() > 210 )
			    precolor.setHsv( precolor.hue(), precolor.saturation() , 210);
			return precolor.name();
		} else {
			/* Colors from list */
			if (nickColors.empty()) return "#000000";
			if(sender == -1 || nickColors.size() == 1) {
				return nickColors[nickColors.size()-1];
			} else {
				int n = sender % (nickColors.size()-1);
				return nickColors[n];
			}
		}
	}
}
