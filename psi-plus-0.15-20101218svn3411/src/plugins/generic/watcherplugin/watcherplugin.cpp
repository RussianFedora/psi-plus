/*
 * watcherplugin.cpp - plugin
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

#include <QFileDialog>
#include <QSound>

#include "view.h"
#include "model.h"
#include "tooltip.h"
#include "ui_options.h"
#include "watcheditem.h"
#include "edititemdlg.h"

#include "psiplugin.h"
#include "stanzafilter.h"
#include "popupaccessor.h"
#include "popupaccessinghost.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "iconfactoryaccessor.h"
#include "iconfactoryaccessinghost.h"
#include "menuaccessor.h"
#include "applicationinfoaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "plugininfoprovider.h"
#include "activetabaccessinghost.h"
#include "activetabaccessor.h"
#include "contactinfoaccessinghost.h"
#include "contactinfoaccessor.h"


#define constVersion "0.3.5"

#define constSoundFile "sndfl"
#define constInterval "intrvl"
#define constCount "count"
#define constSndFiles "sndfiles"
#define constJids "jids"
#define constWatchedItems "watcheditem"
#define constDisableSnd "dsblsnd"


class Watcher : public QObject, public PsiPlugin, public PopupAccessor, public MenuAccessor, public PluginInfoProvider,
	    public OptionAccessor, public StanzaFilter, public IconFactoryAccessor, public ApplicationInfoAccessor,
	    public ActiveTabAccessor, public ContactInfoAccessor
{
        Q_OBJECT
	Q_INTERFACES(PsiPlugin PopupAccessor OptionAccessor StanzaFilter IconFactoryAccessor
		      PluginInfoProvider MenuAccessor ApplicationInfoAccessor ActiveTabAccessor ContactInfoAccessor)
public:
        Watcher();
        virtual QString name() const;
        virtual QString shortName() const;
        virtual QString version() const;
        virtual QWidget* options();
        virtual bool enable();
        virtual bool disable();
        virtual void optionChanged(const QString& option);
        virtual void applyOptions();
        virtual void restoreOptions();
        virtual void setPopupAccessingHost(PopupAccessingHost* host);
        virtual void setOptionAccessingHost(OptionAccessingHost* host);
        virtual bool incomingStanza(int account, const QDomElement& xml);
	virtual bool outgoingStanza(int account, QDomElement& xml);
        virtual void setIconFactoryAccessingHost(IconFactoryAccessingHost* host);
        QList < QVariantHash >* getAccountMenuParam();
        QList < QVariantHash >* getContactMenuParam();
        virtual void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host);
	virtual QString pluginInfo();
	virtual void setActiveTabAccessingHost(ActiveTabAccessingHost* host);
	virtual void setContactInfoAccessingHost(ContactInfoAccessingHost* host);
private:
        OptionAccessingHost *psiOptions;
        PopupAccessingHost* popup;
        IconFactoryAccessingHost* IcoHost;
        ApplicationInfoAccessingHost* AppInfoHost;
	ActiveTabAccessingHost* activeTab;
	ContactInfoAccessingHost* contactInfo;
	bool enabled;
        QString SoundFile;
	int Interval;
	QPointer<QWidget> optionsWid;
	Model *model_;
	Ui::Options ui_;
	QList<WatchedItem*> items_;
	bool isSndEnable;
	bool disableSnd;

	void showPopup(const QString& text);
	bool checkWatchedItem(const QString& from, const QString& body, WatchedItem *wi);

private slots:
        void checkSound(QModelIndex index = QModelIndex());
        void getSound(QModelIndex index = QModelIndex());
        void addLine();
        void delSelected();
        void Hack();
        void onOptionsClose();
        void addJidFromMenu();
        void checked(QString jid, bool check);
	void playSound(QString SoundFile);

	void addItemAct();
	void delItemAct();
	void editItemAct();
	void addNewItem(QString settings);
	void editCurrentItem(QString setting);
	void timeOut();
};

Q_EXPORT_PLUGIN(Watcher);

Watcher::Watcher() {
    psiOptions = 0;
    IcoHost = 0;
    popup = 0;
    AppInfoHost = 0;
    contactInfo = 0;
    enabled = false;
    SoundFile = "sound/watcher.wav";
    Interval = 2;
    disableSnd = true;
    model_ = 0;
}

QString Watcher::name() const {
        return "Watcher Plugin";
}

QString Watcher::shortName() const {
        return "watcher";
}

QString Watcher::version() const {
        return constVersion;
}

bool Watcher::enable(){
    if(psiOptions) {
        enabled = true;        
        SoundFile = psiOptions->getPluginOption(constSoundFile, QVariant(SoundFile)).toString();
        Interval = psiOptions->getPluginOption(constInterval, QVariant(Interval)).toInt();
	disableSnd = psiOptions->getPluginOption(constDisableSnd, QVariant(disableSnd)).toBool();

        QStringList jids = psiOptions->getPluginOption(constJids, QVariant(QStringList())).toStringList();
        QStringList soundFiles = psiOptions->getPluginOption(constSndFiles, QVariant(QStringList())).toStringList();

        if(!model_) {
            model_ = new Model(jids, soundFiles, this);
            connect(model_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(Hack()));
        }

	items_.clear();
	QStringList list = psiOptions->getPluginOption(constWatchedItems).toStringList();
	foreach(QString settings, list) {
		WatchedItem* wi = new WatchedItem();
		wi->setSettings(settings);
		items_.push_back(wi);
		if(!wi->jid().isEmpty())
			wi->setText(wi->jid());
		else if(!wi->watchedText().isEmpty())
			wi->setText(wi->watchedText());
		else wi->setText(tr("Empty item"));
	}
    }

    return enabled;
}

bool Watcher::disable(){
	if(optionsWid) {
		delete(ui_.tableView); //если этого не сделать, падает при попытке оключения плагина
		delete(ui_.listWidget);
	}
	foreach(WatchedItem *wi, items_)
		delete(wi);
	items_.clear();

	enabled = false;
        return true;
}

QWidget* Watcher::options(){
        if (!enabled) {
		return 0;
	}
	optionsWid = new QWidget();
	connect(optionsWid, SIGNAL(destroyed()), this, SLOT(onOptionsClose()));

	ui_.setupUi(optionsWid);

	restoreOptions();

	ui_.cb_hack->setVisible(false);
	ui_.tb_open->setIcon(IcoHost->getIcon("psi/browse"));
	ui_.tb_test->setIcon(IcoHost->getIcon("psi/play"));
	ui_.pb_add->setIcon(IcoHost->getIcon("psi/addContact"));
	ui_.pb_del->setIcon(IcoHost->getIcon("psi/remove"));
	ui_.pb_add_item->setIcon(IcoHost->getIcon("psi/addContact"));
	ui_.pb_delete_item->setIcon(IcoHost->getIcon("psi/remove"));
	ui_.pb_edit_item->setIcon(IcoHost->getIcon("psi/action_templates_edit"));

	ui_.tableView->setModel(model_);
	ui_.tableView->init(IcoHost);

	connect(ui_.tableView, SIGNAL(checkSound(QModelIndex)), this, SLOT(checkSound(QModelIndex)));
	connect(ui_.tableView, SIGNAL(getSound(QModelIndex)), this, SLOT(getSound(QModelIndex)));
	connect(ui_.tb_test, SIGNAL(pressed()), this, SLOT(checkSound()));
	connect(ui_.tb_open, SIGNAL(pressed()), this, SLOT(getSound()));
	connect(ui_.pb_add, SIGNAL(released()), this, SLOT(addLine()));
	connect(ui_.pb_del, SIGNAL(released()), this, SLOT(delSelected()));

	connect(ui_.pb_add_item, SIGNAL(clicked()), this, SLOT(addItemAct()));
	connect(ui_.pb_delete_item, SIGNAL(clicked()), this, SLOT(delItemAct()));
	connect(ui_.pb_edit_item, SIGNAL(clicked()), this, SLOT(editItemAct()));
	connect(ui_.listWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editItemAct()));

	return optionsWid;
}

void Watcher::addLine() {
    model_->addRow();
    Hack(); //activate apply button
}

void Watcher::delSelected() {
    model_->deleteSelected();
    Hack(); //activate apply button
}

void Watcher::applyOptions() {
    SoundFile = ui_.le_sound->text();
    psiOptions->setPluginOption(constSoundFile, QVariant(SoundFile));

    disableSnd = ui_.cb_disable_snd->isChecked();
    psiOptions->setPluginOption(constDisableSnd, QVariant(disableSnd));

    Interval = ui_.sb_delay->value();
    psiOptions->setPluginOption(constInterval,QVariant(Interval));

    model_->apply();
    psiOptions->setPluginOption(constJids, QVariant(model_->getWatchedJids()));
    psiOptions->setPluginOption(constSndFiles, QVariant(model_->getSounds()));

    foreach(WatchedItem *wi, items_)
	    delete(wi);
    items_.clear();
    QStringList l;
    for(int i = 0; i < ui_.listWidget->count(); i++) {
	    WatchedItem *wi = (WatchedItem*)ui_.listWidget->item(i);
	    if(wi) {
		    items_.push_back(wi->copy());
		    l.push_back(wi->settingsString());
	    }
    }
    psiOptions->setPluginOption(constWatchedItems, QVariant(l));
}

void Watcher::restoreOptions() {
	ui_.le_sound->setText(SoundFile);
	ui_.sb_delay->setValue(Interval);
	ui_.cb_disable_snd->setChecked(disableSnd);
	model_->reset();
	foreach(WatchedItem* wi, items_) {
		ui_.listWidget->addItem(wi->copy());
	}
}

bool Watcher::incomingStanza(int acc, const QDomElement &stanza) {
	if(enabled) {
		if(stanza.tagName() == "presence") {
			if(stanza.attribute("type") == "error")
				return false;

			QString from = stanza.attribute("from");
			if(from.isEmpty())
				return false;

			bool find = false;
			if(model_->getWatchedJids().contains(from, Qt::CaseInsensitive))
				find = true;
			else {
				from = from.split("/").takeFirst();
				if(model_->getWatchedJids().contains(from, Qt::CaseInsensitive))
				find = true;
			}
			if(find) {
				QString status = stanza.firstChildElement("show").text();
				if(status.isEmpty()) {
					if(stanza.attribute("type") == "unavailable") {
						status = "offline";
					}
					else {
						status = "online";
						if(model_->statusByJid(from) != status && psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) {
							QString snd = model_->soundByJid(from);
							if(snd.isEmpty())
								snd = SoundFile;
							playSound(snd);
						}
					}
				}
				if(model_->statusByJid(from) != status) {
					model_->setStatusForJid(from, status);
					status[0] = status[0].toUpper();
					QString nick = contactInfo->name(acc, from);
					QString text;
					if(!nick.isEmpty())
						from = " [" + from + "]";
					text = nick + from + tr(" change status to ") + status;
					showPopup(text);
				}
			}
		}
		else if(stanza.tagName() == "message") {
			QString body = stanza.firstChildElement("body").text();
			if(!body.isEmpty()) {
				QString from = stanza.attribute("from");
				QString type = stanza.attribute("type");
				if(disableSnd) {
					QString jid = activeTab->getJid();
					if(jid.split("/").first().toLower() == from.split("/").first().toLower())
						return false;
				}

				if(type == "groupchat") {
					foreach(WatchedItem *wi, items_) {
						if(!wi->groupChat())
							continue;

						if(checkWatchedItem(from, body, wi))
							break;
					}
				}
				else {
					foreach(WatchedItem *wi, items_) {
						if(wi->groupChat())
							continue;

						if(checkWatchedItem(from, body, wi))
							break;
					}
				}
			}
		}
	}
	return false;
}

bool Watcher::outgoingStanza(int account, QDomElement& xml)
{
	return false;
}

bool Watcher::checkWatchedItem(const QString &from, const QString &body, WatchedItem *wi) {
	if(!wi->jid().isEmpty() && from.contains(QRegExp(wi->jid(),Qt::CaseInsensitive, QRegExp::Wildcard))) {
		isSndEnable = psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool();
		if(wi->alwaysUse() || isSndEnable) {
			psiOptions->setGlobalOption("options.ui.notifications.sounds.enable", QVariant(false));
			playSound(wi->sFile());
			QTimer::singleShot(500, this, SLOT(timeOut())); // включаем все звуки через секунду, чтобы не игралось два звука одновременно
			return true;
		}
	}
	if(!wi->watchedText().isEmpty()) {
		foreach(QString txt, wi->watchedText().split(QRegExp("\\s+"), QString::SkipEmptyParts)) {
			if(body.contains(QRegExp(txt, Qt::CaseInsensitive, QRegExp::Wildcard)) ) {
				psiOptions->setGlobalOption("options.ui.notifications.sounds.enable", QVariant(false));
				playSound(wi->sFile());
				QTimer::singleShot(500, this, SLOT(timeOut())); // включаем все звуки через секунду, чтобы не игралось два звука одновременно
				return true;
			}
		}
	}
	return false;
}


void Watcher::timeOut() {
	psiOptions->setGlobalOption("options.ui.notifications.sounds.enable", QVariant(isSndEnable));
}

void Watcher::setPopupAccessingHost(PopupAccessingHost* host) {
    popup = host;
}

void Watcher::setIconFactoryAccessingHost(IconFactoryAccessingHost* host) {
    IcoHost = host;
}

void Watcher::setActiveTabAccessingHost(ActiveTabAccessingHost *host) {
	activeTab = host;
}

void Watcher::setOptionAccessingHost(OptionAccessingHost *host) {
    psiOptions = host;
}

void Watcher::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host) {
     AppInfoHost = host;
}

void Watcher::setContactInfoAccessingHost(ContactInfoAccessingHost *host) {
	contactInfo = host;
}

void Watcher::optionChanged(const QString &option) {
    Q_UNUSED(option);
}

void Watcher::playSound(QString f) {
     QFile file(f);
    if(!file.exists()) {
        QString newF = AppInfoHost->appResourcesDir() + "/" + f;
        file.setFileName(newF);
        if(!file.exists()) {
            newF = AppInfoHost->appHomeDir() + "/" + f;
            file.setFileName(newF);
            if(!file.exists())
                return;
        }
    }

    QString str = file.fileName();

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
        QSound::play(str);
#else
        QString player = psiOptions->getGlobalOption("options.ui.notifications.sounds.unix-sound-player").toString();
        if (player == "")
        {
            if (QFile("/proc/asound").exists()) {
                player =  "aplay";
            } else {
                player =  "play";
            }
        }
        QStringList args = player.split(' ');
        args += str;
        QString prog = args.takeFirst();
        QProcess::startDetached(prog, args);
#endif

}

void Watcher::getSound(QModelIndex index) {
    if(ui_.tb_open->isDown()) {
	QString fileName = QFileDialog::getOpenFileName(0,tr("Choose a sound file"),
							psiOptions->getPluginOption(constLastFile, QVariant("")).toString(),
							tr("Sound (*.wav)"));
        if(fileName.isEmpty()) return;
	QFileInfo fi(fileName);
	psiOptions->setPluginOption(constLastFile, QVariant(fi.absolutePath()));
	ui_.le_sound->setText(fileName);
    } else {
	    QString fileName = QFileDialog::getOpenFileName(0,tr("Choose a sound file"),
							    psiOptions->getPluginOption(constLastFile, QVariant("")).toString(),
							    tr("Sound (*.wav)"));
        if(fileName.isEmpty()) return;
	QFileInfo fi(fileName);
	psiOptions->setPluginOption(constLastFile, QVariant(fi.absolutePath()));
        const QModelIndex editIndex = model_->index(index.row(), 2, QModelIndex());
        model_->setData(editIndex, QVariant(fileName));
    }
}

void Watcher::checkSound(QModelIndex index) {
    if(ui_.tb_test->isDown()) {
	playSound(ui_.le_sound->text());
    } else {
        playSound(model_->tmpSoundFile(index));
     }
}

void Watcher::showPopup(const QString& text) {
    if(Interval) {
        QVariant delay(Interval*1000);
        int delay_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.delays.status").toInt();
        psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

        bool enbl_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.enabled").toBool();
        QVariant enbl(true);
        psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);

	popup->initPopup(text, tr("Watcher Plugin"), "psi/search");

        delay = QVariant(delay_);
        psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

        enbl = QVariant(enbl_);
        psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);
    }
}

void Watcher::Hack() {
    ui_.cb_hack->toggle();
}

void Watcher::onOptionsClose() {
	model_->reset();
}

QList < QVariantHash >* Watcher::getAccountMenuParam() {
    return 0;
}

QList < QVariantHash >* Watcher::getContactMenuParam() {
    QVariantHash hash;
        hash["icon"] = QVariant(QString("psi/search"));
        hash["name"] = QVariant(tr("Watch for JID"));
        hash["reciver"] = qVariantFromValue(qobject_cast<QObject *>(this));
        hash["slot"] = QVariant(SLOT(addJidFromMenu()));
        QList< QVariantHash > * l = new QList< QVariantHash >();
        l->push_back(hash);
        return l;
}

void Watcher::addJidFromMenu() {
    QString jid = sender()->property("jid").toString();
    bool checked = model_->getWatchedJids().contains(jid);
    ToolTip *tooltip = new ToolTip(jid, checked, sender());
    connect(tooltip, SIGNAL(check(QString,bool)), this, SLOT(checked(QString,bool)));
    tooltip->show();
}

void Watcher::checked(QString jid, bool check) {
    if(!enabled) return;
    if(check)
       model_->addRow(jid);
    else
        model_->deleteRow(jid);

    psiOptions->setPluginOption(constJids, QVariant(model_->getWatchedJids()));
    psiOptions->setPluginOption(constSndFiles, QVariant(model_->getSounds()));
}

void Watcher::addItemAct() {
	EditItemDlg *eid = new EditItemDlg(IcoHost, psiOptions, optionsWid);
	connect(eid, SIGNAL(testSound(QString)), this, SLOT(playSound(QString)));
	connect(eid, SIGNAL(dlgAccepted(QString)), this, SLOT(addNewItem(QString)));
	eid->show();
}

void Watcher::addNewItem(QString settings) {
	WatchedItem *wi = new WatchedItem(ui_.listWidget);
	wi->setSettings(settings);
	if(!wi->jid().isEmpty())
		wi->setText(wi->jid());
	else if(!wi->watchedText().isEmpty())
		wi->setText(wi->watchedText());
	else wi->setText(tr("Empty item"));
	Hack();
}

void Watcher::delItemAct() {
	WatchedItem *wi = (WatchedItem*)ui_.listWidget->currentItem();
	if(wi) {
		int index = items_.indexOf(wi);
		if(index != -1)
			items_.removeAt(index);

		delete(wi);
		Hack();
	}
}

void Watcher::editItemAct() {
	WatchedItem *wi = (WatchedItem*)ui_.listWidget->currentItem();
	if(wi) {
		EditItemDlg *eid = new EditItemDlg(IcoHost, psiOptions, optionsWid);
		eid->init(wi->settingsString());
		connect(eid, SIGNAL(testSound(QString)), this, SLOT(playSound(QString)));
		connect(eid, SIGNAL(dlgAccepted(QString)), this, SLOT(editCurrentItem(QString)));		
		eid->show();
	}
}

void Watcher::editCurrentItem(QString settings) {
	WatchedItem *wi = (WatchedItem*)ui_.listWidget->currentItem();
	if(wi) {
		wi->setSettings(settings);
		if(!wi->jid().isEmpty())
			wi->setText(wi->jid());
		else if(!wi->watchedText().isEmpty())
			wi->setText(wi->watchedText());
		else wi->setText(tr("Empty item"));
		Hack();
	}
}

QString Watcher::pluginInfo() {
	return tr("Author: ") +  "Dealer_WeARE\n"
			+ tr("Email: ") + "wadealer@gmail.com\n\n"
			+ trUtf8("This plugin is designed to monitor the status of specific roster contacts, as well as for substitution of standard sounds of incoming messages.\n"
			 "On the first tab set up a list of contacts for the status of which is monitored. When the status of such contacts changes a popup window will be shown"
			 " and when the status changes to online a custom sound can be played."
			 "On the second tab is configured list of items, the messages are being monitored. Each element can contain a regular expression"
			" to check for matches with JID, from which the message arrives, a list of regular expressions to check for matches with the text"
			" of an incoming message, the path to sound file which will be played in case of coincidence, as well as the setting, whether the sound"
			" is played always, even if the global sounds off. ");
}

#include "watcherplugin.moc"

