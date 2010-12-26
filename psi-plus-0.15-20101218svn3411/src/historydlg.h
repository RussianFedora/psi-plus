/*
 * historydlg.h
 * Copyright (C) 2001-2010  Justin Karneges, Michail Pishchagin, Evgeny Khryukin
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

#ifndef HISTORYDLG_H
#define HISTORYDLG_H

#include "eventdb.h"
#include "ui_history.h"

class PsiAccount;
class PsiContact;
class UserListItem;

namespace XMPP {
	class Jid;
}

class HistoryDlg : public QDialog
{
	Q_OBJECT

public:
	HistoryDlg(const XMPP::Jid&, PsiAccount*);
	virtual ~HistoryDlg();

private slots:
	void openSelectedContact();
	void getLatest();
	void getEarliest();
	void getPrevious();
	void getNext();
	void getDate();
	void refresh();
	void findMessages();
	void edb_finished();
	void highlightBlocks(const QString text);
	void changeAccount(const QString accountName);
	void removeHistory();
	void exportHistory();
	void openChat();
	void doMenu();
	void removedContact(PsiContact*);

protected:
	bool eventFilter(QObject *, QEvent *);

private:
	void setButtons();
	void setButtons(bool act);
	void loadContacts();
	void displayResult(const EDBResult *, int, int max=-1);
	QFont fontForOption(const QString& option);
	void listAccounts();
	UserListItem* currentUserListItem();
	void startRequest();
	void stopRequest();

	class Private;
	Private *d;
	Ui::HistoryDlg ui_;
	QStringList jids_;
};

#endif
