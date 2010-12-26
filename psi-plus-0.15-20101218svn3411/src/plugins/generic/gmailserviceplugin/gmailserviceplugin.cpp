/*
 * gmailserviceplugin.cpp - plugin
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

#include "gmailserviceplugin.h"
#include "common.h"

Q_EXPORT_PLUGIN(GmailNotifyPlugin);


GmailNotifyPlugin::GmailNotifyPlugin()
	: enabled(false)
	, optionsApplingInProgress_(false)
{
	message = "From: %1 %2\n"\
		  "Header: %3\n"\
		  "Url: http://mail.google.com/mail/";
	interval = 4;
}

QString GmailNotifyPlugin::name() const
{
	return "GMail Service Plugin";
}

QString GmailNotifyPlugin::shortName() const
{
	return "gmailnotify";
}

QString GmailNotifyPlugin::version() const
{
	return PLUGIN_VERSION;
}

QWidget* GmailNotifyPlugin::options()
{
	if (!enabled)
		return 0;

	options_ = new QWidget;
	ui_.setupUi(options_);
	restoreOptions();

	connect(ui_.cb_accounts, SIGNAL(currentIndexChanged(int)), SLOT(updateOptions(int)));

	return options_;
}

bool GmailNotifyPlugin::enable()
{
	enabled = true;
	optionsApplingInProgress_ = false;
	id_.clear();
	accounts.clear();
	actions_ = new ActionsList(this);
	connect(actions_, SIGNAL(changeNoSaveState(int,QString,bool)), this, SLOT(changeNoSaveState(int,QString,bool)));

	QFile f(":/icons/gmailnotify.png");
	if(f.open(QIODevice::ReadOnly))
		iconHost->addIcon("gmailnotify/menu", f.readAll());
	f.close();

	f.setFileName(":/icons/nohistory.png");
	if(f.open(QIODevice::ReadOnly))
		iconHost->addIcon("gmailnotify/nohistory", f.readAll());
	f.close();

	//Read default values
	message = psiOptions->getPluginOption(OPTION_TEMPLATE_MESSAGES, QVariant(message)).toString();
	interval = psiOptions->getPluginOption(OPTION_INTERVAL, QVariant(interval)).toInt();
	loadLists();

	//Update features
	bool end = false;
	int acc = 0;
	while(!end) {
		QString jid = accInfo->getJid(acc);
		if(jid == "-1") {
			end = true;
			continue;
		}
		QStringList l = jid.split("@");
		QString domen = l.last().split("/").first();
		QString id = stanzaSender->uniqueId(acc);
		id_.append(id);
		if(accInfo->getStatus(acc) != "offline")
			stanzaSender->sendStanza(acc, QString("<iq type='get' to='%1' id='%2' ><query xmlns='http://jabber.org/protocol/disco#info'/></iq>")
					 .arg(domen)
					 .arg(id));
		acc++;
	}

	timer_ = new QTimer();
	timer_->setInterval(3000);
	connect(timer_, SIGNAL(timeout()), SLOT(timeout()));
	//timer_->start();

	return true;
}

bool GmailNotifyPlugin::disable()
{
	timer_->stop();
	delete timer_;

	qDeleteAll(accounts);
	accounts.clear();

	delete actions_;

	enabled = false;
	return true;
}

void GmailNotifyPlugin::applyOptions()
{
	if (!options_)
		return;

	optionsApplingInProgress_ = true;

	message = ui_.te_message->toPlainText();
	psiOptions->setPluginOption(OPTION_TEMPLATE_MESSAGES, QVariant(message));

	interval = ui_.sb_interval->value();
	psiOptions->setPluginOption(OPTION_INTERVAL, QVariant(interval));

	int index = ui_.cb_accounts->currentIndex();
	if(accounts.size() <= index || index == -1)
		return;

	AccountSettings* as = accounts.at(index);
	as->notifyAllUnread = !ui_.rb_new_messages->isChecked();

	if (as->notifyAllUnread){
		as->lastMailTid.clear();
		as->lastMailTime.clear();
	}

	as->isMailEnabled = ui_.cb_mail->isChecked();
	as->isArchivingEnabled = ui_.cb_archiving->isChecked();
	as->isSuggestionsEnabled = ui_.cb_suggestions->isChecked();
	as->isSharedStatusEnabled = ui_.cb_shared_statuses->isChecked();
	as->isNoSaveEnbaled = ui_.cb_nosave->isChecked();

	Utils::updateSettings(as, stanzaSender, accInfo);

	if(as->isMailEnabled)
		Utils::requestMail(as, stanzaSender, accInfo);

	if(as->isSharedStatusEnabled)
		Utils::requestSharedStatusesList(as, stanzaSender, accInfo);

	if(as->isNoSaveEnbaled && as->isArchivingEnabled)
		Utils::updateNoSaveState(as, stanzaSender, accInfo);

	updateActions(as);

	saveLists();

	QTimer::singleShot(2000, this, SLOT(stopOptionsApply()));
}

void GmailNotifyPlugin::stopOptionsApply()
{
	optionsApplingInProgress_ = false;
}

void GmailNotifyPlugin::saveLists()
{
	QStringList l;
	foreach(AccountSettings *as, accounts)
		l.append(as->toString());
	psiOptions->setPluginOption(OPTION_LISTS, QVariant(l));
}

void GmailNotifyPlugin::loadLists()
{
	QStringList l = psiOptions->getPluginOption(OPTION_LISTS, QVariant()).toStringList();
	foreach(QString settings, l) {
		AccountSettings *as = new AccountSettings();
		as->fromString(settings);
		accounts.append(as);
	}
}

void GmailNotifyPlugin::restoreOptions()
{
	if (!options_ || optionsApplingInProgress_)
		return;

	ui_.lb_error->hide();
	ui_.gb_settings->setEnabled(true);
	ui_.te_message->setPlainText(message);
	ui_.sb_interval->setValue(interval);
	ui_.cb_mail->setVisible(true);
	ui_.cb_shared_statuses->setVisible(true);
	ui_.cb_nosave->setVisible(true);

	ui_.cb_accounts->setEnabled(true);
	ui_.cb_accounts->clear();
	if(!accounts.isEmpty()) {
		foreach(AccountSettings* as, accounts) {
			if(as->account != -1)
				ui_.cb_accounts->addItem(as->jid);
		}
	}

	if(!ui_.cb_accounts->count()) {
		ui_.cb_accounts->setEnabled(false);
		ui_.gb_mail_settings->setEnabled(false);
		ui_.gb_settings->setEnabled(false);
		ui_.lb_error->setVisible(true);
	}
	else {
		ui_.cb_accounts->setCurrentIndex(0);
		updateOptions(0);
	}
}

void GmailNotifyPlugin::updateOptions(int index)
{
	if (!options_ || index >= accounts.size() || index < 0)
		return;

	AccountSettings *as = accounts.at(index);

	ui_.cb_mail->setChecked(as->isMailEnabled);
	ui_.cb_mail->setVisible(as->isMailSupported);
	ui_.gb_mail_settings->setEnabled(ui_.cb_mail->isChecked());
	ui_.rb_new_messages->setChecked(!as->notifyAllUnread);
	ui_.rb_all_messages->setChecked(as->notifyAllUnread);

	ui_.cb_archiving->setChecked(as->isArchivingEnabled);
	ui_.cb_suggestions->setChecked(as->isSuggestionsEnabled);	

	ui_.cb_shared_statuses->setChecked(as->isSharedStatusEnabled);
	ui_.cb_shared_statuses->setVisible(as->isSharedStatusSupported);

	ui_.cb_nosave->setChecked(as->isNoSaveEnbaled);
	ui_.cb_nosave->setVisible(as->isNoSaveSupported);
	ui_.cb_nosave->setEnabled(ui_.cb_archiving->isChecked());
}

bool GmailNotifyPlugin::incomingStanza(int account, const QDomElement& stanza)
{
	if (enabled) {
		if (stanza.tagName() == "iq") {
			QDomElement query = stanza.firstChild().toElement();
			if (!query.isNull()) {
				if(checkFeatures(account, stanza, query))
					return true;

				if(checkEmail(account, stanza, query))
					return true;

				if(checkSettings(account, stanza, query))
					return true;

				if(checkSharedStatus(account, stanza, query))
					return true;

				if(checkNoSave(account, stanza, query))
					return true;
			}
		}
		else if(stanza.tagName() == "message") {
			QDomElement x = stanza.firstChildElement("x");
			if(!x.isNull() && x.attribute("xmlns") == "google:nosave") {
				QString jid = stanza.attribute("from").split("/").first();
				bool val = (x.attribute("value") == "enabled");
				AccountSettings *as = findAccountSettings(accInfo->getJid(account));
				if(as && as->noSaveList.contains(jid)
					&& as->noSaveList.value(jid) != val)
				{
					as->noSaveList.insert(jid, val);
					showPopup(tr("No-save state for contact %1 is changed").arg(jid));
					return true;
				}
			}
		}
	}
	return false;
}

bool GmailNotifyPlugin::outgoingStanza(int account, QDomElement& stanza)
{
	return false;
}

bool GmailNotifyPlugin::checkFeatures(int account, const QDomElement &stanza, const QDomElement& query)
{
	bool foundFeatures = false;
	if (stanza.attribute("type") == "result"		
	     && query.tagName() == "query"
	     && query.attribute("xmlns") == "http://jabber.org/protocol/disco#info")
	{
		if(id_.contains(stanza.attribute("id"))) {
			id_.removeAll(stanza.attribute("id"));
			foundFeatures = true;
		}
		bool foundGoogleExt = false;
		for (QDomNode child = query.firstChild(); !child.isNull(); child = child.nextSibling()) {
			QDomElement feature = child.toElement();
			if(feature.isNull() || feature.tagName() != "feature")
				continue;

			QString fullJid = stanza.attribute("to");
			QString jid = fullJid.split("/").at(0);

			//If the server supports the Gmail extension
			if(feature.attribute("var") == "google:mail:notify" && feature.attribute("node").isEmpty()) {
				AccountSettings *as = create(account, jid);
				as->isMailSupported = true;
				//Utils::requestMail(as, stanzaSender, accInfo);
				foundGoogleExt = true;
			}
			else if(feature.attribute("var") == "google:setting" && feature.attribute("node").isEmpty()) {
				AccountSettings *as = create(account, jid);
				Utils::getUserSettings(as, stanzaSender, accInfo);
				foundGoogleExt = true;
			}
			else if(feature.attribute("var") == "google:shared-status" && feature.attribute("node").isEmpty()) {
				AccountSettings *as = create(account, jid);
				as->isSharedStatusSupported = true;
				as->status = accInfo->getStatus(account);
				as->message = accInfo->getStatusMessage(account);
				as->fullJid = fullJid;
				if(as->isSharedStatusEnabled)
					Utils::requestSharedStatusesList(as, stanzaSender, accInfo);
				foundGoogleExt = true;
			}
			else if(feature.attribute("var") == "google:nosave" && feature.attribute("node").isEmpty()) {
				AccountSettings *as = create(account, jid);
				as->isNoSaveSupported = true;
				updateActions(as);
				if(as->isNoSaveEnbaled)
					Utils::updateNoSaveState(as, stanzaSender, accInfo);
				foundGoogleExt = true;
			}

		}
		if(foundGoogleExt) {
			optionsApplingInProgress_ = false;
			restoreOptions();
		}
	}
	return foundFeatures;
}

bool GmailNotifyPlugin::checkEmail(int account, const QDomElement &stanza, const QDomElement& query)
{
	if (stanza.attribute("type") == "set"
		&& query.tagName() == "new-mail"
		&& query.attribute("xmlns") == "google:mail:notify")
	{
		//Server reports new mail
		//send success result
		QString from = stanza.attribute("to");
		QString to = from.split("/").at(0);
		QString iqId = stanza.attribute("id");
		QString reply = QString("<iq type='result' from='%1' to='%2' id='%3' />").arg(from,to,iqId);
		stanzaSender->sendStanza(account, reply);
		AccountSettings *as = create(account, to);

		//requests new mail
		Utils::requestMail(as, stanzaSender, accInfo);
		//block stanza processing
		return true;
	}
	else if(stanza.attribute("type") == "result"
		&& query.tagName() == "mailbox"
		&& query.attribute("xmlns") == "google:mail:notify")
	{
		//Email Query Response
		QString from = stanza.attribute("to");
		AccountSettings *as = create(account, from);

		as->lastMailTime = query.attribute("result-time");

		QDomElement lastmail = query.firstChildElement("mail-thread-info");
		if (!lastmail.isNull())
			as->lastMailTid = lastmail.attribute("tid");

		//save last check values
		if (!as->notifyAllUnread) {
			saveLists();
		}
		for (QDomNode child = query.lastChild(); !child.isNull(); child = child.previousSibling()) {
			//invalid url
			QString url = child.toElement().attribute("url");
			QString subject = child.firstChildElement("subject").text();
			QString snippet = child.firstChildElement("snippet").text();
			QDomElement senders = child.firstChildElement("senders");
			QString name , email;
			for (QDomNode sender = senders.firstChild(); !sender.isNull(); sender = sender.nextSibling()) {
				QDomElement adresser = sender.toElement();
				if (!adresser.isNull() && adresser.attribute("originator")=="1") {
					name = adresser.attribute("name");
					email = adresser.attribute("address");
				}
			}
			QString reply = QString("<message to=\"%1\" id=\"new-message-%2\" >"\
						"<subject>%3</subject><body>%4</body></message>")
					.arg(from,stanzaSender->uniqueId(account))
					.arg(subject.isEmpty() ? "\n" : subject)
					.arg(message.arg(name,email,snippet));
			stanzaSender->sendStanza(account, reply);
		}
		return true;
	}
	return false;
}

bool GmailNotifyPlugin::checkSettings(int account, const QDomElement &stanza, const QDomElement& query)
{
	bool foundSettings = false;
	if ( (stanza.attribute("type") == "result"
	     || stanza.attribute("type") == "set")
	    && query.tagName() == "usersetting"
	    && query.attribute("xmlns") == "google:setting")
	{
		foundSettings = true;
		//id_.removeAll(stanza.attribute("id"));
		QString jid = stanza.attribute("to").split("/").at(0);
		AccountSettings *as = create(account, jid);
		for (QDomNode child = query.firstChild(); !child.isNull(); child = child.nextSibling()) {
			QDomElement setting = child.toElement();
			QString value = setting.attribute("value");
			if(setting.isNull() || value.isEmpty())
				continue;

			if(setting.tagName() == "autoacceptsuggestions")
				as->isSuggestionsEnabled = (value == "true");
			else if(setting.tagName() == "mailnotifications") {
				as->isMailEnabled = (value == "true");
				Utils::requestMail(as, stanzaSender, accInfo);
			}
			else if(setting.tagName() == "archivingenabled") {
				as->isArchivingEnabled = (value == "true");
				updateActions(as);
			}
		}
		restoreOptions();

		if(stanza.attribute("type") == "set")
			showPopup(tr("Settings for an account %1 are changed").arg(jid));
	}

	return foundSettings;
}

bool GmailNotifyPlugin::checkSharedStatus(int account, const QDomElement &stanza, const QDomElement &query)
{
	bool found = false;
	if (query.tagName() == "query"
	    && query.attribute("xmlns") == "google:shared-status") {
		found = true;
		QString jid = stanza.attribute("to");
		AccountSettings *as = create(account, jid);
		QString type = stanza.attribute("type");
		if(type == "set")
			as->sharedStatuses.clear();
		if(query.hasAttribute("status-max"))
			as->statusMax = query.attribute("status-max").toInt();
		if(query.hasAttribute("status-list-contents-max"))
			as->listContentsMax = query.attribute("status-list-contents-max").toInt();
		if(query.hasAttribute("status-list-max"))
			as->listMax = query.attribute("status-list-max").toInt();

		if(type == "result" || type == "set") {
			for (QDomNode child = query.firstChild(); !child.isNull(); child = child.nextSibling()) {
				QDomElement settings = child.toElement();
				if(settings.isNull())
					continue;
				QString tagName = settings.tagName();
				if(tagName == "status")
					as->message = settings.text();
				else if(tagName == "show")
					as->status = settings.text().replace("default", "online");
				else if(tagName == "status-list") {
					QStringList l;
					for (QDomNode child = settings.firstChild(); !child.isNull(); child = child.nextSibling()) {
						QDomElement st = child.toElement();
						if(st.isNull() || st.tagName() != "status")
							continue;

						l.append(st.text());
					}
					if(!l.isEmpty())
						as->sharedStatuses.insert(settings.attribute("show").replace("default", "online"), l);
				}
			}
		}

		if(as->sharedStatuses.isEmpty())
			as->sharedStatuses.insert(as->status, QStringList(as->message));


		if(as->isSharedStatusEnabled) {
			timer_->start();
			accountController->setStatus(account, as->status, as->message);
			showPopup(tr("Shared Status for an account %1 is updated").arg(jid));
		}
	}

	return found;
}

bool GmailNotifyPlugin::checkNoSave(int account, const QDomElement &stanza, const QDomElement &query)
{
	bool found = false;
	if(query.tagName() == "query"
	   && query.attribute("xmlns") == "google:nosave")
	{
		found = true;
		QString jid = stanza.attribute("from");
		AccountSettings *as = create(account, jid);
		for(QDomNode child = query.firstChild(); !child.isNull(); child = child.nextSibling()) {
			QDomElement noSave = child.toElement();
			if(noSave.isNull() || noSave.tagName() != "item")
				continue;

			QString item = noSave.attribute("jid");
			QString state = noSave.attribute("value");
			bool val = (state == "enabled");
			as->noSaveList.insert(item, val);
			actions_->updateAction(account, item, val);

			if(stanza.attribute("type") == "set")
				showPopup(tr("No-save state for contact %1 is changed").arg(item));
		}
	}

	return found;
}

bool GmailNotifyPlugin::processEvent(int account, QDomElement& e)
{
	if (enabled && hasAccountSettings(account)) {
		QDomElement msg = e.lastChildElement();
		if(!msg.isNull()) {
			QString from = msg.attribute("from");
			QString to = msg.attribute("to");
			if ( to == from ) {
				from.replace(QRegExp("(.*)/.*"),"\\1/gmail");
				msg.setAttribute("from",from);
			}
		}
	}

	return false;
}

AccountSettings* GmailNotifyPlugin::findAccountSettings(const QString &jid)
{
	if(!jid.isEmpty()) {
		foreach(AccountSettings* as, accounts) {
			if(as->jid == jid.toLower())
				return as;
		}
	}

	return 0;
}

AccountSettings* GmailNotifyPlugin::create(int account, QString jid)
{
	jid = jid.toLower();
	if(jid.contains("/"))
		jid = jid.split("/").first();

	AccountSettings *as = findAccountSettings(jid);
	if(!as) {
		as = new AccountSettings(account, jid);
		accounts.append(as);
	}
	else
		as->account = account;

	return as;
}

void GmailNotifyPlugin::changeNoSaveState(int account, QString jid, bool val)
{
	if(!Utils::checkAccount(account, accInfo))
		return;

	QString str = QString("<iq type='set' to='%1' id='%2'>"
			    "<query xmlns='google:nosave'>"
			    "<item xmlns='google:nosave' jid='%3' value='%4'/>"
			    "</query></iq>")
			.arg(accInfo->getJid(account), stanzaSender->uniqueId(account))
			.arg(jid, (val ? "enabled" : "disabled"));

	stanzaSender->sendStanza(account, str);
	AccountSettings *as = findAccountSettings(accInfo->getJid(account));
	if(as) {
		as->noSaveList.insert(jid, (val ? "enabled" : "disabled"));
	}
}

bool GmailNotifyPlugin::hasAccountSettings(int account)
{
	bool has = false;
	foreach(AccountSettings *as, accounts) {
		if(as->account == account) {
			has = true;
			break;
		}
	}

	return has;
}

void GmailNotifyPlugin::setStanzaSendingHost(StanzaSendingHost *host)
{
	stanzaSender = host;
}

void GmailNotifyPlugin::setAccountInfoAccessingHost(AccountInfoAccessingHost* host)
{
	accInfo = host;
}

void GmailNotifyPlugin::setOptionAccessingHost(OptionAccessingHost* host)
{
	psiOptions = host;
}

void GmailNotifyPlugin::setPopupAccessingHost(PopupAccessingHost *host)
{
	popup = host;
}

void GmailNotifyPlugin::setPsiAccountControllingHost(PsiAccountControllingHost *host)
{
	accountController = host;
}

void GmailNotifyPlugin::setIconFactoryAccessingHost(IconFactoryAccessingHost *host)
{
	iconHost = host;
}

void GmailNotifyPlugin::showPopup(QString text)
{
	if(!interval)
		return;

	QVariant delay(interval*1000);
	int delay_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.delays.status").toInt();
	psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

	bool enbl_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.enabled").toBool();
	QVariant enbl(true);
	psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);

	popup->initPopup(text, name());

	delay = QVariant(delay_);
	psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

	enbl = QVariant(enbl_);
	psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);
}

void GmailNotifyPlugin::timeout()
{
	bool findEnabledAcc = false;
	foreach(AccountSettings *as, accounts) {
		int account = as->account;
		if(account == -1 || !as->isSharedStatusEnabled || !as->isSharedStatusSupported)
			continue;

		findEnabledAcc = true;
		QString status = accInfo->getStatus(account);
		QString message = accInfo->getStatusMessage(account);
		if(message.length() > as->statusMax)
			message.chop(message.length() - as->statusMax);
		if(status == as->status && message == as->message)
			continue;
		as->message = message;
		as->status = status;
		if(as->sharedStatuses.contains(status)) {
			QStringList l = as->sharedStatuses.value(status);
			if(l.contains(message))
				l.removeAll(message);
			l.push_front(message);
			while(l.size() > as->listContentsMax)
				l.removeLast();
			as->sharedStatuses.insert(status, l);
		}
		else {
			as->sharedStatuses.insert(status, QStringList() << message);
			while(as->sharedStatuses.size() > as->listMax) {
				foreach(QString key, as->sharedStatuses.keys()) {
					if(key != status) {
						as->sharedStatuses.remove(key);
						break;
					}
				}
			}
		}
		Utils::updateSharedStatus(as, stanzaSender, accInfo);
	}
	if(!findEnabledAcc)
		timer_->stop();
}

QList < QVariantHash >* GmailNotifyPlugin::getButtonParam()
{
	return 0;
}

QAction* GmailNotifyPlugin::getAction(QObject* parent, int account, const QString& contact)
{
	QAction *act = actions_->newAction(parent, account, contact, iconHost->getIcon("gmailnotify/nohistory"));
	AccountSettings* as = findAccountSettings(accInfo->getJid(account));
	if(as) {
		act->setVisible(as->isNoSaveEnbaled && as->isNoSaveSupported && as->isArchivingEnabled);
		if(as->noSaveList.contains(contact))
			act->setChecked(as->noSaveList.value(contact));
	}
	return act;
}

void GmailNotifyPlugin::updateActions(AccountSettings *as)
{
	bool val = as->isNoSaveEnbaled && as->isNoSaveSupported && as->isArchivingEnabled;
	actions_->updateActionsVisibility(as->account, val);
}

QString GmailNotifyPlugin::pluginInfo()
{
	return tr("Authors: ") +  "VampiRUS\nDealer_WeARE\n\n"
			+ trUtf8("Shows notifications of new messages in your Gmailbox.\n"
			 "Note: The plugin only checks the root of your Inbox folder in your"
			 " Gmailbox for new messages. When using server side mail filtering, you may not be notified about all new messages.");
}
