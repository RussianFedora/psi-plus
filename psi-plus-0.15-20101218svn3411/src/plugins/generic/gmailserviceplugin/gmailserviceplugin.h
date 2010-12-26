/*
 * gmailserviceplugin.h - plugin
 * Copyright (C) 2009-2010 Kravtsov Nikolai, Khryukin Evgeny
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

#ifndef GMAILSERVICEPLUGIN_H
#define GMAILSERVICEPLUGIN_H

#include "actionslist.h"

#include "psiplugin.h"
#include "stanzafilter.h"
#include "stanzasender.h"
#include "stanzasendinghost.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "eventfilter.h"
#include "plugininfoprovider.h"
#include "accountinfoaccessinghost.h"
#include "accountinfoaccessor.h"
#include "popupaccessinghost.h"
#include "popupaccessor.h"
#include "psiaccountcontroller.h"
#include "psiaccountcontrollinghost.h"
#include "iconfactoryaccessinghost.h"
#include "iconfactoryaccessor.h"
#include "toolbariconaccessor.h"

#include "ui_options.h"
#include "accountsettings.h"

#define OPTION_TEMPLATE_MESSAGES "message"
#define OPTION_LISTS "lists"
#define OPTION_INTERVAL "interval"

#define PLUGIN_VERSION "0.5.6"

class GmailNotifyPlugin : public QObject, public PsiPlugin, public EventFilter, public AccountInfoAccessor,
	public StanzaFilter, public StanzaSender, public OptionAccessor, public PluginInfoProvider,
	public PopupAccessor, public PsiAccountController, public IconFactoryAccessor,
	public ToolbarIconAccessor
{
	Q_OBJECT
	Q_INTERFACES(PsiPlugin StanzaFilter StanzaSender EventFilter OptionAccessor PluginInfoProvider
		     AccountInfoAccessor PopupAccessor PsiAccountController IconFactoryAccessor
		     ToolbarIconAccessor)
public:
	GmailNotifyPlugin();
	virtual QString name() const;
	virtual QString shortName() const;
	virtual QString version() const;
	virtual QWidget* options();
	virtual bool enable();
	virtual bool disable();
	virtual void setOptionAccessingHost(OptionAccessingHost* host);
	virtual void optionChanged(const QString& /*option*/){};
	virtual void applyOptions();
	virtual void restoreOptions();
	virtual bool incomingStanza(int account, const QDomElement& stanza);
	virtual bool outgoingStanza(int account, QDomElement& stanza);
	virtual bool processEvent(int account, QDomElement& e);
	virtual bool processMessage(int /*account*/, const QString& /*fromJid*/, const QString& /*body*/, const QString& /*subject*/){ return false; };
	virtual bool processOutgoingMessage(int , const QString& , QString& , const QString& , QString& ) { return false; }
	virtual void logout(int ) {};
	virtual void setStanzaSendingHost(StanzaSendingHost *host);
	virtual void setAccountInfoAccessingHost(AccountInfoAccessingHost* host);
	virtual void setPopupAccessingHost(PopupAccessingHost* host);
	virtual void setPsiAccountControllingHost(PsiAccountControllingHost* host);
	virtual void setIconFactoryAccessingHost(IconFactoryAccessingHost* host);
	virtual QList < QVariantHash >* getButtonParam();
	virtual QAction* getAction(QObject* parent, int account, const QString& contact);

	virtual QString pluginInfo();

private:
	bool enabled;
	StanzaSendingHost* stanzaSender;
	OptionAccessingHost* psiOptions;
	AccountInfoAccessingHost* accInfo;
	PopupAccessingHost* popup;
	PsiAccountControllingHost* accountController;
	IconFactoryAccessingHost* iconHost;
	QString message;
	int interval;

	QPointer<QWidget> options_;
	Ui::Options ui_;
	QList<AccountSettings*> accounts;
	QTimer* timer_;
	QStringList id_;
	ActionsList* actions_;
	bool optionsApplingInProgress_;

	AccountSettings* findAccountSettings(const QString& jid);
	AccountSettings* create(int account, QString jid);

	bool hasAccountSettings(int account);

	bool checkFeatures(int account, const QDomElement& stanza, const QDomElement& query);
	bool checkEmail(int account, const QDomElement& stanza, const QDomElement& query);
	bool checkSettings(int account, const QDomElement& stanza, const QDomElement& query);
	bool checkSharedStatus(int account, const QDomElement& stanza, const QDomElement& query);
	bool checkNoSave(int account, const QDomElement& stanza, const QDomElement& query);

	void saveLists();
	void loadLists();

	void showPopup(QString text);

	void updateActions(AccountSettings* as);

private slots:
	void timeout();
	void changeNoSaveState(int account, QString jid, bool val);
	void updateOptions(int index);
	void stopOptionsApply();
};

#endif
