/*
 * actionslist.cpp - plugin
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

#include "actionslist.h"

ActionsList::ActionsList(QObject *p)
	: QObject(p)
{
}

ActionsList::~ActionsList()
{
	list_.clear();
}

QAction* ActionsList::newAction(QObject *p, int account, const QString &contact, QIcon ico)
{
	QAction* act = new QAction(ico, tr(" Enable Off the Record Chat.\n"
					   "When checked, the server will not\n"
					   "save the history for this contact."), p);
	QList<QAction*> l = list_.value(account);
	act->setProperty("account", account);
	act->setProperty("jid", contact);
	act->setVisible(false);
	act->setCheckable(true);
	l.push_back(act);
	list_.insert(account, l);
	connect(act, SIGNAL(toggled(bool)), SLOT(actionActivated(bool)));
	connect(act, SIGNAL(destroyed(QObject*)), SLOT(actionDestroyed(QObject*)));
	return act;
}

void ActionsList::updateActionsVisibility(int account, bool isVisible)
{
	if(!list_.contains(account))
		return;

	foreach(QAction* act, list_.value(account)) {
		act->setVisible(isVisible);
	}
}

void ActionsList::updateAction(int account, const QString &jid, bool isChecked)
{
	foreach(QAction* act, list_.value(account)) {
		if(act->property("jid").toString() == jid) {
			act->setChecked(isChecked);
			break;
		}
	}
}

void ActionsList::actionActivated(bool val)
{
	QAction* act = qobject_cast<QAction*>(sender());
	if(act) {
		int account = act->property("account").toInt();
		QString jid = act->property("jid").toString();
		emit changeNoSaveState(account, jid, val);
	}
}

void ActionsList::actionDestroyed(QObject *obj)
{
	QAction* act = qobject_cast<QAction*>(obj);
	if(!act)
		return;

	foreach(QList<QAction*> l, list_.values()) {
		int account = list_.key(l);
		bool change = false;
		foreach(QAction* a, l) {
			if(act == a) {
				l.removeAll(a);
				change = true;
			}
		}
		if(change) {
			list_.insert(account, l);
			break;
		}
	}
}
