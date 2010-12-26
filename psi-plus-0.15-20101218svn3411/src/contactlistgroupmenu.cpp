/*
 * contactlistgroupmenu.cpp - context menu for contact list groups
 * Copyright (C) 2008-2010  Yandex LLC (Michail Pishchagin)
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

#include "contactlistgroupmenu.h"

#include <QPointer>
#include <QMimeData>

#include "iconaction.h"
#include "iconset.h"
#include "contactlistgroup.h"
#include "psioptions.h"
#include "shortcutmanager.h"
#include "psicontactlist.h"
#include "iconaction.h"
#include "psicontact.h"
#include "psiaccount.h"
#include "statusdlg.h"
#include "xmpp_tasks.h"

#include "groupchatdlg.h"

class ContactListGroupMenu::Private : public QObject
{
	Q_OBJECT

	QPointer<ContactListGroup> group;
	QAction* renameAction_;
	QAction* removeGroupAndContactsAction_;
#ifdef YAPSI
	QAction* addGroupAction_;
#endif
	QAction* sendMessageAction_;
	QAction* removeGroupWithoutContactsAction_;
	QMenu* authMenu_;
	QAction* actionAuth_;
	QAction* actionAuthRequest_;
	QAction* actionAuthRemove_;
	QAction* actionCustomStatus_;
	QAction* actionMucHide_;
	QAction* actionMucShow_;
	QAction* actionMucLeave_;

public:
	Private(ContactListGroupMenu* menu, ContactListGroup* _group)
		: QObject(0)
		, group(_group)
		, menu_(menu)
	{
		connect(menu, SIGNAL(aboutToShow()), SLOT(updateActions()));

		renameAction_ = new IconAction("", tr("Re&name"), menu->shortcuts("contactlist.rename"), this, "act_rename");
		connect(renameAction_, SIGNAL(triggered()), this, SLOT(rename()));

		actionAuth_ = new QAction(tr("Resend Authorization to Group"), this);
		connect(actionAuth_, SIGNAL(triggered()), this, SLOT(authResend()));

		actionAuthRequest_ = new QAction(tr("Request Authorization from Group"), this);
		connect(actionAuthRequest_, SIGNAL(triggered()), this, SLOT(authRequest()));

		actionAuthRemove_ = new QAction(tr("Remove Authorization from Group"), this);
		connect(actionAuthRemove_, SIGNAL(triggered()), this, SLOT(authRemove()));

		actionCustomStatus_ = new IconAction(tr("Send Status to Group"), this, "psi/action_direct_presence");
		connect(actionCustomStatus_, SIGNAL(triggered()), this, SLOT(customStatus()));

		removeGroupAndContactsAction_ = new IconAction(tr("Remove Group and Contacts"), this, "psi/remove");
#ifdef YAPSI
		removeGroupAndContactsAction_->setText(tr("&Remove"));
#endif
		removeGroupAndContactsAction_->setShortcuts(ShortcutManager::instance()->shortcuts("contactlist.delete"));
		connect(removeGroupAndContactsAction_, SIGNAL(triggered()), SLOT(removeGroupAndContacts()));

		removeGroupWithoutContactsAction_ = new IconAction(tr("Remove Group"), this, "psi/remove");
		connect(removeGroupWithoutContactsAction_, SIGNAL(triggered()), this, SLOT(removeGroupWithoutContacts()));

		sendMessageAction_ = new IconAction(tr("Send Message to Group"), this, "psi/sendMessage");
		connect(sendMessageAction_, SIGNAL(triggered()), SLOT(sendMessage()));

#ifdef YAPSI
		addGroupAction_ = new QAction(tr("&Add group..."), this);
		connect(addGroupAction_, SIGNAL(triggered()), SLOT(addGroup()));
#endif

		actionMucHide_ = new IconAction(tr("Hide All"), this, "psi/action_muc_hide");
		connect(actionMucHide_, SIGNAL(triggered()), SLOT(mucHide()));

		actionMucShow_ = new IconAction(tr("Show All"), this, "psi/action_muc_show");
		connect(actionMucShow_, SIGNAL(triggered()), SLOT(mucShow()));
		
		actionMucLeave_ = new IconAction(tr("Leave All"), this, "psi/action_muc_leave");
		connect(actionMucLeave_, SIGNAL(triggered()), SLOT(mucLeave()));

		authMenu_ = 0;

		if (ContactListGroup::SpecialType_Conference != group->specialGroupType()) {
#ifdef YAPSI
			menu->addAction(renameAction_);
			menu->addAction(removeGroupAndContactsAction_);
			menu->addAction(addGroupAction_);
#else
			menu->addAction(renameAction_);
			menu->addAction(sendMessageAction_);
			menu->addAction(actionCustomStatus_);
			menu->addSeparator();
			authMenu_ = menu->addMenu(tr("Authorization"));
			authMenu_->addAction(actionAuth_);
			authMenu_->addAction(actionAuthRequest_);
			authMenu_->addAction(actionAuthRemove_);
			menu->addAction(removeGroupWithoutContactsAction_);
			menu->addAction(removeGroupAndContactsAction_);
		} else {
			menu->addAction(actionMucHide_);
			menu->addAction(actionMucShow_);
			menu->addAction(actionMucLeave_);
			menu->addSeparator();
			menu->addAction(actionCustomStatus_);
		}
#endif
		updateActions();
	}

private slots:
	void updateActions()
	{
		if (!group)
			return;

		sendMessageAction_->setVisible(PsiOptions::instance()->getOption("options.ui.message.enabled").toBool());
		sendMessageAction_->setEnabled(group->contacts().first()->account()->isAvailable());                
		actionCustomStatus_->setEnabled(group->contacts().first()->account()->isAvailable());
		if(authMenu_)
			authMenu_->setEnabled(group->contacts().first()->account()->isAvailable());
		renameAction_->setEnabled(group->isEditable());
		removeGroupAndContactsAction_->setEnabled(group->isRemovable());
		removeGroupWithoutContactsAction_->setEnabled(group->isRemovable());
#ifdef YAPSI
		addGroupAction_->setEnabled(group->model()->contactList()->haveAvailableAccounts());
#endif
	}

	void mucHide()
	{
		PsiAccount *pa = group->contacts().first()->account();
		foreach (QString gc, pa->groupchats()) {
			Jid j(gc);
			GCMainDlg *gcDlg = pa->findDialog<GCMainDlg*>(j.bare());
			if (gcDlg && !gcDlg->isTabHidden()) gcDlg->hideTab();
		}
	}

	void mucShow()
	{
		PsiAccount *pa = group->contacts().first()->account();
		foreach (QString gc, pa->groupchats()) {
			Jid j(gc);
			GCMainDlg *gcDlg = pa->findDialog<GCMainDlg*>(j.bare());
			if (gcDlg && gcDlg->isTabHidden()) gcDlg->showTab();
		}
	}

	void mucLeave()
	{
		PsiAccount *pa = group->contacts().first()->account();
		foreach (QString gc, pa->groupchats()) {
			Jid j(gc);
			GCMainDlg *gcDlg = pa->findDialog<GCMainDlg*>(j.bare());
			if (gcDlg) gcDlg->close();
		}
	}

	void rename()
	{
		if (!group)
			return;

		menu_->model()->renameSelectedItem();
	}

	void removeGroupAndContacts()
	{
		if (!group)
			return;

		emit menu_->removeSelection();
	}

	void authResend()
	{
		if (!group)
			return;

		QList<PsiContact*> contacts = group->contacts();
		if (!contacts.isEmpty()) {
			foreach(PsiContact* contact, contacts) {
				contact->account()->actionAuth(contact->jid());
			}
		}
	}

	void authRequest()
	{
		if (!group)
		return;

		QList<PsiContact*> contacts = group->contacts();
		if (!contacts.isEmpty()) {
			foreach(PsiContact* contact, contacts) {
				contact->account()->actionAuthRequest(contact->jid());
			}
		}
	}

	void authRemove()
	{
		if (!group)
			return;

		QList<PsiContact*> contacts = group->contacts();
		if (!contacts.isEmpty()) {
			foreach(PsiContact* contact, contacts) {
				contact->account()->actionAuthRemove(contact->jid());
			}
		}
	}

	void customStatus()
	{
		if (!group)
			return;

		PsiAccount *pa = group->contacts().first()->account();
		StatusSetDlg *w = new StatusSetDlg(pa->psi(), makeLastStatus(pa->status().type()), lastPriorityNotEmpty());
		QList<XMPP::Jid> list;
		foreach(PsiContact* contact, group->contacts()) {
			if(contact->isPrivate()) continue;
			list << contact->jid();
		}
		w->setJidList(list);
		connect(w, SIGNAL(setJidList(const QList<XMPP::Jid> &, const Status &)), SLOT(setStatusFromDialog(const QList<XMPP::Jid> &, const Status &)));
		w->show();
	}

	void setStatusFromDialog(const QList<XMPP::Jid> &j, const Status &s)
	{
		PsiAccount *pa = group->contacts().first()->account();
		for(QList<Jid>::const_iterator it = j.begin(); it != j.end(); ++it)
		{
			JT_Presence *p = new JT_Presence(pa->client()->rootTask());
			p->pres(*it,s);
			p->go(true);
		}
	}

	void removeGroupWithoutContacts()
	{
		if (!group)
			return;

		QModelIndexList indexes;
		indexes += group->model()->groupToIndex(group);
		QMimeData* data = group->model()->mimeData(indexes);
		emit menu_->removeGroupWithoutContacts(data);
		delete data;
	}

	void sendMessage()
	{
		if (!group)
			return;

		QList<PsiContact*> contacts = group->contacts();
		if (!contacts.isEmpty()) {
			QList<XMPP::Jid> list;
			foreach(PsiContact* contact, contacts) {
				list << contact->jid();
			}
			contacts.first()->account()->actionSendMessage(list);
		}
	}

#ifdef YAPSI
	void addGroup()
	{
		if (!group)
			return;

		emit menu_->addGroup();
	}
#endif

private:
	ContactListGroupMenu* menu_;
};

ContactListGroupMenu::ContactListGroupMenu(ContactListGroup* group, ContactListModel* model)
	: ContactListItemMenu(group, model)
{
	d = new Private(this, group);
}

ContactListGroupMenu::~ContactListGroupMenu()
{
	delete d;
}

#include "contactlistgroupmenu.moc"
