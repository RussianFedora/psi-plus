/*
 * attentionplugin.cpp - plugin
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

#include <QtGui>
#include <QtCore>
#include <QByteArray>

#include "psiplugin.h"
#include "stanzafilter.h"
#include "accountinfoaccessor.h"
#include "accountinfoaccessinghost.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "activetabaccessinghost.h"
#include "activetabaccessor.h"
#include "stanzasender.h"
#include "stanzasendinghost.h"
#include "iconfactoryaccessor.h"
#include "iconfactoryaccessinghost.h"
#include "toolbariconaccessor.h"
#include "popupaccessor.h"
#include "popupaccessinghost.h"
#include "menuaccessor.h"
#include "applicationinfoaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "plugininfoprovider.h"

#define cVer "0.1.3"
#define constSoundFile "sndfl"
#define constInterval "intrvl"
#define constInfPopup "infpopup"
#define constTimeout "timeout"
#define constDisableDnd "dsbldnd"


class AttentionPlugin: public QObject, public PsiPlugin, public StanzaFilter, public AccountInfoAccessor,
                        public OptionAccessor, public ActiveTabAccessor, public ToolbarIconAccessor, public ApplicationInfoAccessor,
			public IconFactoryAccessor, public PopupAccessor, public StanzaSender, public MenuAccessor, public PluginInfoProvider
{
	Q_OBJECT
        Q_INTERFACES(PsiPlugin StanzaFilter AccountInfoAccessor OptionAccessor ActiveTabAccessor ApplicationInfoAccessor
		     ToolbarIconAccessor IconFactoryAccessor PopupAccessor StanzaSender MenuAccessor PluginInfoProvider)

public:
        AttentionPlugin();
	virtual QString name() const;
	virtual QString shortName() const;
	virtual QString version() const;
        virtual QWidget* options();
	virtual bool enable();
        virtual bool disable();
        virtual void applyOptions();
        virtual void restoreOptions();
        virtual bool incomingStanza(int account, const QDomElement& xml);
	virtual bool outgoingStanza(int account, QDomElement& xml);
        virtual void setAccountInfoAccessingHost(AccountInfoAccessingHost* host);
        virtual void setOptionAccessingHost(OptionAccessingHost* host);
        virtual void optionChanged(const QString& option);
        virtual void setActiveTabAccessingHost(ActiveTabAccessingHost* host);
        virtual void setIconFactoryAccessingHost(IconFactoryAccessingHost* host);
        virtual void setPopupAccessingHost(PopupAccessingHost* host);
        virtual void setStanzaSendingHost(StanzaSendingHost *host);
        virtual QList < QVariantHash >* getButtonParam();
	virtual QAction* getAction(QObject* , int , const QString& ) { return 0; };
        virtual QList < QVariantHash >* getAccountMenuParam();
        virtual QList < QVariantHash >* getContactMenuParam();
        virtual void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host);
	virtual QString pluginInfo();


private:
        bool enabled;
        OptionAccessingHost* psiOptions;
        AccountInfoAccessingHost *AccInfoHost;
        ActiveTabAccessingHost* activeTab;       
        IconFactoryAccessingHost *IcoHost;
        PopupAccessingHost* popup;
        StanzaSendingHost *stanzaSender;
        ApplicationInfoAccessingHost* appInfo;
        QString SoundFile;
        QLineEdit *SoundFileWidget;
        int Interval, Timeout;
        bool InfPopup, DisableDnd;
        QSpinBox *IntervalWidget, *TimeoutWidget;
        void playSound(QString SoundFile);
        void showPopup(QString text);
        QCheckBox *InfPopupWidget, *DisableDndWidget;

        struct Blocked {
            int Acc;
            QString Jid;
            QDateTime LastMes;
        };
        QVector<Blocked> BlockedJids;
	QTimer *nudgeTimer_;
	QPointer<QWidget> nudgeWindow_;
	QPoint oldPoint_;

        bool FindAcc(int account, QString Jid, int &i);
        void sendAttention(int account, QString yourJid, QString jid);
	void nudge();

private slots:
        void checkSound();
        void getSound();
        void sendAttentionFromTab();
        void sendAttentionFromMenu();
	void nudgeTimerTimeout();
    };

Q_EXPORT_PLUGIN(AttentionPlugin);

AttentionPlugin::AttentionPlugin() {
        enabled = false;        
        AccInfoHost = 0;
        psiOptions = 0;
        IcoHost = 0;
        activeTab = 0;
        appInfo = 0;
        popup = 0;
        stanzaSender = 0;
        SoundFile = "sound/attention.wav";
        SoundFileWidget = 0;
        Interval = 8;
        IntervalWidget = 0;
        InfPopup = false;
        InfPopupWidget = 0;
        Timeout = 30;
        DisableDnd = false;
        TimeoutWidget = 0;
        DisableDndWidget = 0;
        BlockedJids.clear();
	nudgeTimer_ = 0;
    }

QString AttentionPlugin::name() const {
        return "Attention Plugin";
    }

QString AttentionPlugin::shortName() const {
        return "attention";
}

QString AttentionPlugin::version() const {
        return cVer;
}

bool AttentionPlugin::enable() {
    QFile file(":/attentionplugin/attention.png");
    if ( file.open(QIODevice::ReadOnly) ) {
        QByteArray image = file.readAll();
        IcoHost->addIcon("attentionplugin/attention",image);
        file.close();
    } else {
        enabled = false;
        return enabled;
    }
    if(psiOptions) {      
        enabled = true;
         SoundFile = psiOptions->getPluginOption(constSoundFile, QVariant(SoundFile)).toString();
         Interval = psiOptions->getPluginOption(constInterval, QVariant(Interval)).toInt();
         Timeout = psiOptions->getPluginOption(constTimeout, QVariant(Timeout)).toInt();
         InfPopup = psiOptions->getPluginOption(constInfPopup, QVariant(InfPopup)).toBool();
         DisableDnd = psiOptions->getPluginOption(constDisableDnd, QVariant(DisableDnd)).toBool();
	 QWidgetList wl = qApp->allWidgets();
	 foreach(QWidget *w, wl) {
		 if(w->objectName() == "MainWin") {
			 nudgeWindow_ = w;
			 break;
		 }
	 }
	 nudgeTimer_ = new QTimer(this);
	 nudgeTimer_->setInterval(50);
	 connect(nudgeTimer_, SIGNAL(timeout()), SLOT(nudgeTimerTimeout()));
    }
    return enabled;
}

bool AttentionPlugin::disable() {
        enabled = false;
	nudgeTimer_->stop();
	delete nudgeTimer_;
	nudgeTimer_ = 0;
	return true;
}

QWidget* AttentionPlugin::options() {
    if(!enabled) {
    return 0;
}
    QWidget *options = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(options);

    SoundFileWidget = new QLineEdit;
    SoundFileWidget->setText(SoundFile);
    QPushButton *playButton = new QPushButton;
    playButton->setIcon(IcoHost->getIcon("psi/play"));
    QPushButton *getButton = new QPushButton;
    getButton->setIcon(IcoHost->getIcon("psi/browse"));
    QHBoxLayout *sndLayout = new QHBoxLayout;
    sndLayout->addWidget(new QLabel(tr("Play sound:")));
    sndLayout->addWidget(SoundFileWidget);
    sndLayout->addWidget(getButton);
    sndLayout->addWidget(playButton);

    IntervalWidget = new QSpinBox();
    IntervalWidget->setMinimum(-1);
    IntervalWidget->setValue(Interval);
    QHBoxLayout *IntervalLayout = new QHBoxLayout();
    IntervalLayout->addWidget(new QLabel(tr("Show popup")));
    IntervalLayout->addWidget(IntervalWidget);
    IntervalLayout->addWidget(new QLabel(tr("sec (0=disable, -1=infinite)")));
    IntervalLayout->addStretch();

    TimeoutWidget = new QSpinBox();
    TimeoutWidget->setValue(Timeout);
    QHBoxLayout *TimeoutLayout = new QHBoxLayout();
    TimeoutLayout->addWidget(new QLabel(tr("Allow receiving one Attention from the same contact every")));
    TimeoutLayout->addWidget(TimeoutWidget);
    TimeoutLayout->addWidget(new QLabel(tr("sec")));
    TimeoutLayout->addStretch();

    InfPopupWidget = new QCheckBox(tr("Don't hide popup if status is Away or XA"));
    InfPopupWidget->setChecked(InfPopup);

    DisableDndWidget = new QCheckBox(tr("Disable notifications if status is DND"));
    DisableDndWidget->setChecked(DisableDnd);

    QLabel *wikiLink = new QLabel(tr("<a href=\"http://psi-plus.com/wiki/plugins#attention_plugin\">Wiki (Online)</a>"));
    wikiLink->setOpenExternalLinks(true);

    layout->addLayout(IntervalLayout);
    layout->addLayout(TimeoutLayout);
    layout->addWidget(InfPopupWidget);
    layout->addWidget(DisableDndWidget);
    layout->addLayout(sndLayout);
    layout->addWidget(wikiLink);
    layout->addStretch();

    connect(playButton, SIGNAL(released()), this, SLOT(checkSound()));
    connect(getButton, SIGNAL(released()), this, SLOT(getSound()));

    return options;
}

bool AttentionPlugin::incomingStanza(int account, const QDomElement& stanza) {
     if (enabled) {
         if(stanza.tagName() == "message"
            && stanza.attribute("type") == "headline"
            && !stanza.firstChildElement("attention").isNull()) {

	     if(DisableDnd && AccInfoHost->getStatus(account) == "dnd")
                 return false;

             QString from = stanza.attribute("from");

             int i = BlockedJids.size();
             if(FindAcc(account, from, i)) {
                 Blocked &B = BlockedJids[i];
                 if(QDateTime::currentDateTime().secsTo(B.LastMes) > -Timeout) {
                     return false;
                 } else {
                     B.LastMes = QDateTime::currentDateTime();
                 }
             } else {
                 Blocked B = { account, from, QDateTime::currentDateTime() };
                 BlockedJids << B;
             }

	     int Interval_ = Interval;
             if(InfPopup && (AccInfoHost->getStatus(account) == "away" || AccInfoHost->getStatus(account) == "xa")) {
		Interval = -1;
	     }
	     showPopup(from + tr(" sends Attention message to you!"));
	     Interval = Interval_;

             if(psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool())
                            playSound(SoundFile);

	     /*QTextEdit *te = activeTab->getEditBox();
	     if(te)
		     nudgeWindow_ = te->window();

	     else
		     nudgeWindow_ = qApp->activeWindow();*/



	     if(nudgeWindow_ && nudgeWindow_->isVisible())
		     nudge();
         }
     }
     return false;
 }

bool AttentionPlugin::outgoingStanza(int account, QDomElement& xml)
{
	return false;
}

void AttentionPlugin::applyOptions() {
    if (!SoundFileWidget || !IntervalWidget || !InfPopupWidget || !TimeoutWidget || !DisableDndWidget)   return;

    QVariant vSoundFile(SoundFileWidget->text());
    psiOptions->setPluginOption(constSoundFile, vSoundFile);
    SoundFile = vSoundFile.toString();

    QVariant vInterval(IntervalWidget->value());
    psiOptions->setPluginOption(constInterval, vInterval);
    Interval = vInterval.toInt();

    QVariant vTimeout(TimeoutWidget->value());
    psiOptions->setPluginOption(constTimeout, vTimeout);
    Timeout = vTimeout.toInt();

    QVariant vInfPopup(InfPopupWidget->isChecked());
    psiOptions->setPluginOption(constInfPopup, vInfPopup);
    InfPopup = vInfPopup.toBool();

    QVariant vDisableDnd(DisableDndWidget->isChecked());
    psiOptions->setPluginOption(constDisableDnd, vDisableDnd);
    DisableDnd = vDisableDnd.toBool();
}

void AttentionPlugin::restoreOptions() {
    if (!SoundFileWidget || !IntervalWidget || !InfPopupWidget || !TimeoutWidget || !DisableDndWidget)   return;

    SoundFileWidget->setText(psiOptions->getPluginOption(constSoundFile, QVariant(SoundFile)).toString());
    IntervalWidget->setValue(psiOptions->getPluginOption(constInterval, QVariant(Interval)).toInt());
    TimeoutWidget->setValue(psiOptions->getPluginOption(constTimeout, QVariant(Timeout)).toInt());
    InfPopupWidget->setChecked(psiOptions->getPluginOption(constInfPopup, QVariant(InfPopup)).toBool());
    DisableDndWidget->setChecked(psiOptions->getPluginOption(constDisableDnd, QVariant(DisableDnd)).toBool());
}

void AttentionPlugin::optionChanged(const QString &option) {
    Q_UNUSED(option);
}

void AttentionPlugin::setAccountInfoAccessingHost(AccountInfoAccessingHost* host) {
     AccInfoHost = host;
 }

void AttentionPlugin::setOptionAccessingHost(OptionAccessingHost* host) {
    psiOptions = host;
}

void AttentionPlugin::setActiveTabAccessingHost(ActiveTabAccessingHost* host) {
    activeTab = host;
}

void AttentionPlugin::setIconFactoryAccessingHost(IconFactoryAccessingHost *host) {
    IcoHost = host;
}

void AttentionPlugin::setStanzaSendingHost(StanzaSendingHost *host) {
    stanzaSender = host;
}

void AttentionPlugin::setPopupAccessingHost(PopupAccessingHost* host) {
    popup = host;
}

void AttentionPlugin::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host) {
    appInfo = host;
}

QList < QVariantHash >* AttentionPlugin::getButtonParam() {
    QList< QVariantHash > * l = new QList< QVariantHash >();    
    QVariantHash hash;
    hash["tooltip"] = QVariant(tr("Send Attention"));
    hash["icon"] = QVariant(QString("attentionplugin/attention"));
    hash["reciver"] = qVariantFromValue(qobject_cast<QObject *>(this));
    hash["slot"] = QVariant(SLOT(sendAttentionFromTab()));
    l->push_back(hash);    
    return l;
}

void AttentionPlugin::playSound(QString f) {
    QFile file(f);
    if(!file.exists()) {
        QString newF = appInfo->appResourcesDir() + "/" + f;
        file.setFileName(newF);
        if(!file.exists()) {
            newF = appInfo->appHomeDir() + "/" + f;
            file.setFileName(newF);
            if(!file.exists())
                return;
        }
    }

    QString str = file.fileName();

    QString player_;
    if (QFile("/proc/asound").exists()) {
        player_ =  "aplay";
    } else {
        player_ =  "play";
    }

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
        QSound::play(str);
#else
        QString player = psiOptions->getGlobalOption("options.ui.notifications.sounds.unix-sound-player").toString();
        if (player == "") player = player_;
        QStringList args = player.split(' ');
        args += str;
        QString prog = args.takeFirst();
        QProcess::startDetached(prog, args);
#endif

}

void AttentionPlugin::getSound() {
    QString fileName = QFileDialog::getOpenFileName(0,tr("Choose a sound file"),"", tr("Sound (*.wav)"));
    if(fileName.isEmpty()) return;
    SoundFileWidget->setText(fileName);
}

void AttentionPlugin::checkSound() {
    playSound(SoundFileWidget->text());
}

void AttentionPlugin::showPopup(QString text) {
    if(Interval) {
        QVariant delay(Interval*1000);
        int delay_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.delays.status").toInt();
        psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

        bool enbl_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.enabled").toBool();
        QVariant enbl(true);
        psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);

        popup->initPopup(text, tr("Attention Plugin"));

        delay = QVariant(delay_);
        psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

        enbl = QVariant(enbl_);
        psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);
    }
}

void AttentionPlugin::sendAttention(int account, QString yourJid, QString jid) {

    if(AccInfoHost->getStatus(account) == "offline") return;

    QString msg = QString("<message from=\"%1\" to=\"%2\" type=\"headline\"><attention xmlns='urn:xmpp:attention:0'/></message>").arg(yourJid).arg(jid);
    stanzaSender->sendStanza(account, msg);

    showPopup(tr("You sent Attention message to %1").arg(jid));
}

void AttentionPlugin::sendAttentionFromTab() {
    if(!enabled) return;
    QString yourJid = activeTab->getYourJid();
    QString jid = activeTab->getJid();
    QString tmpJid("");
    int account = 0;
    while (yourJid != (tmpJid = AccInfoHost->getJid(account))){
        ++account;
        if (tmpJid == "-1") return;
    }

    sendAttention(account, yourJid, jid);
}

void AttentionPlugin::sendAttentionFromMenu() {
    int acc = sender()->property("account").toInt();
    QString jid = sender()->property("jid").toString();
    QString yourJid = AccInfoHost->getJid(acc);

    sendAttention(acc, yourJid, jid);
}

bool AttentionPlugin::FindAcc(int account, QString Jid, int &i) {
    for(; i > 0;) {
         Blocked Block =  BlockedJids[--i];
         if(Block.Acc == account && Block.Jid == Jid) {
             return true;
         }
     }
     return false;
 }

QList < QVariantHash >* AttentionPlugin::getAccountMenuParam() {
    return 0;
}

QList < QVariantHash >* AttentionPlugin::getContactMenuParam() {
        QVariantHash hash;
        hash["icon"] = QVariant(QString("attentionplugin/attention"));
        hash["name"] = QVariant(tr("Send Attention"));
        hash["reciver"] = qVariantFromValue(qobject_cast<QObject *>(this));
        hash["slot"] = QVariant(SLOT(sendAttentionFromMenu()));
        QList< QVariantHash > * l = new QList< QVariantHash >();
        l->push_back(hash);
        return l;
}

void AttentionPlugin::nudge() {
	if(!nudgeWindow_ || !nudgeTimer_ || nudgeTimer_->isActive())
		return;

	oldPoint_ = nudgeWindow_->pos();	
	nudgeTimer_->start();
}

void AttentionPlugin::nudgeTimerTimeout() {
	static uint count = 0;

	if(!nudgeWindow_) {
		nudgeTimer_->stop();
		count = 0;
		return;
	}
	if(count < 40) {
		int rH = qrand()%10, rW = qrand()%10;
		QPoint newPoint(oldPoint_.x()+rH, oldPoint_.y()+rW);
		nudgeWindow_->move(newPoint);
		count++;
	}
	else {
		count = 0;
		nudgeTimer_->stop();
		nudgeWindow_->move(oldPoint_);
	}
}

QString AttentionPlugin::pluginInfo() {
	return tr("Author: ") +  "Dealer_WeARE\n"
			+ tr("Email: ") + "wadealer@gmail.com\n\n"
			+ trUtf8("This plugin is designed to send and receive special messages such as Attentions.\n"
			 "To work correctly, the plugin requires that the client of the other part supports XEP-0224 (for example: Pidgin, Miranda IM with Nudge plugin).");
}

#include "attentionplugin.moc"
