/*
 * birthdayreminderplugin.cpp - plugin
 * Copyright (C) 2009-2010  Khryukin Evgeny
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

#include "psiplugin.h"
#include "stanzafilter.h"
#include "accountinfoaccessor.h"
#include "accountinfoaccessinghost.h"
#include "applicationinfoaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "stanzasender.h"
#include "stanzasendinghost.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "popupaccessor.h"
#include "popupaccessinghost.h"
#include "iconfactoryaccessinghost.h"
#include "iconfactoryaccessor.h"
#include "plugininfoprovider.h"

#define cVer "0.3.1"
#define constLastCheck "lstchck"
#define constDays "days"
#define constInterval "intrvl"
#define consttimeout "timeout"
#define constStartCheck "strtchck"
#define constCheckFromRoster "chckfrmrstr"
#define constLastUpdate "lstupdate"
#define constUpdateInterval "updtintvl"
#define constSoundFile "sndfl"

const QString id = "111aaa222bbb";

class Reminder : public QObject, public PsiPlugin, public StanzaFilter, public AccountInfoAccessor, public ApplicationInfoAccessor,
		public StanzaSender, public OptionAccessor, public PopupAccessor, public IconFactoryAccessor, public PluginInfoProvider
{
	Q_OBJECT
	Q_INTERFACES(PsiPlugin StanzaFilter AccountInfoAccessor ApplicationInfoAccessor StanzaSender OptionAccessor
		     PopupAccessor IconFactoryAccessor PluginInfoProvider)

public:
        Reminder();
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
        virtual void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host);
        virtual void setStanzaSendingHost(StanzaSendingHost *host);
        virtual void setOptionAccessingHost(OptionAccessingHost* host);
	virtual void optionChanged(const QString& ){};
        virtual void setPopupAccessingHost(PopupAccessingHost* host);
        virtual void setIconFactoryAccessingHost(IconFactoryAccessingHost* host);
	virtual QString pluginInfo();

private:
        bool enabled;
        OptionAccessingHost *psiOptions;
        AccountInfoAccessingHost *AccInfoHost;
        ApplicationInfoAccessingHost *AppInfoHost;
        StanzaSendingHost *StanzaHost;
        PopupAccessingHost* popup;
        IconFactoryAccessingHost* IcoHost;
        QString Dir;        
        QString LastCheck;
        int Days;        
        int Interval;       
	int timeout;
	bool StartCheck;
	bool CheckFromRoster;
	QString LastUpdate;
	int UpdateInterval;
	QString SoundFile;
	bool updateInProgress;

	QLineEdit *SoundFileWidget;
	QSpinBox *UpdateIntervalWidget;
	QCheckBox *StartCheckWidget;
	QCheckBox *CheckFromRosterWidget;
	QSpinBox *timeoutWidget;
	QSpinBox *IntervalWidget;
	QSpinBox *DaysWidget;

	QString CheckBirthdays();

private slots:
	 void UpdateVCard();
	 bool Check();
	 void clearCache();
	 void getSound();
	 void checkSound();
	 void playSound(QString);
	 void timeoutStopUpdate();
};

Q_EXPORT_PLUGIN(Reminder);

Reminder::Reminder() {
        enabled = false;        
        AppInfoHost = 0;
        AccInfoHost = 0;
        StanzaHost = 0;
        psiOptions = 0;
        popup = 0;
        Dir = "";
        LastCheck = "1901010101";
        Days = 5;
        DaysWidget = 0;
        Interval = 24;
        IntervalWidget = 0;
	timeout = 15;
	StartCheck = true;
	StartCheckWidget = 0;
	CheckFromRoster = true;
	CheckFromRosterWidget = 0;
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForLocale(codec);
	LastUpdate = "19010101";
	UpdateInterval = 30;
	UpdateIntervalWidget = 0;
	SoundFile = "sound/reminder.wav";
	SoundFileWidget = 0;
	updateInProgress = false;
}

QString Reminder::name() const {
        return "Birthday Reminder Plugin";
    }

QString Reminder::shortName() const {
        return "reminder";
}

QString Reminder::version() const {
        return cVer;
}

bool Reminder::enable() {
  if(!psiOptions)
	  return enabled;

    QFile file(":/reminder/birthday.png");
    if ( file.open(QIODevice::ReadOnly) ) {
	QByteArray image = file.readAll();
	IcoHost->addIcon("reminder/birthdayicon",image);
	file.close();
    } else {
	return enabled;
    }

    enabled = true;

    LastCheck = psiOptions->getPluginOption(constLastCheck, LastCheck).toString();
    timeout = psiOptions->getPluginOption(consttimeout, timeout).toInt();
    Days = psiOptions->getPluginOption(constDays, Days).toInt();
    Interval = psiOptions->getPluginOption(constInterval, Interval).toInt();
    StartCheck = psiOptions->getPluginOption(constStartCheck, StartCheck).toBool();
    CheckFromRoster = psiOptions->getPluginOption(constCheckFromRoster, CheckFromRoster).toBool();
    UpdateInterval = psiOptions->getPluginOption(constUpdateInterval, UpdateInterval).toInt();
    LastUpdate = psiOptions->getPluginOption(constLastUpdate, LastUpdate).toString();
    SoundFile = psiOptions->getPluginOption(constSoundFile, QVariant(SoundFile)).toString();

    Dir = AppInfoHost->appVCardDir() + QDir::separator() + "Birthdays";
    QDir BirthDay(Dir);
    if(!BirthDay.exists(Dir)) {
	BirthDay.mkdir(Dir);
	return enabled;
    }
    if(StartCheck) {
	    LastCheck = QDateTime::currentDateTime().toString("yyyyMMddhh");
	    psiOptions->setPluginOption(constLastCheck, QVariant(LastCheck));
	    QTimer::singleShot(4000, this, SLOT(Check())); //необходимо для инициализации приложения
    }

    return enabled;
}

bool Reminder::disable() {
	enabled = false;        
	return true;
}

QWidget* Reminder::options() {
    if(!enabled)
	    return 0;

    QWidget *options = new QWidget();
    QVBoxLayout *vlayout = new QVBoxLayout(options);
    DaysWidget = new QSpinBox();
    DaysWidget->setMaximum(60);
    DaysWidget->setMinimum(1);
    IntervalWidget = new QSpinBox();
    IntervalWidget->setMinimum(1);
    IntervalWidget->setMaximum(24);
    timeoutWidget = new QSpinBox();
    timeoutWidget->setMinimum(-1);
    UpdateIntervalWidget = new QSpinBox();
    UpdateIntervalWidget->setMinimum(0);
    QHBoxLayout *UpdateIntervalLayout = new QHBoxLayout;
    UpdateIntervalLayout->addWidget(new QLabel(tr("Update birthdays every")));
    UpdateIntervalLayout->addWidget(UpdateIntervalWidget);
    UpdateIntervalLayout->addWidget(new QLabel(tr("days (0 - automatic update disabled)")));
    UpdateIntervalLayout->addStretch();
    QPushButton *CheckButton = new QPushButton(tr("Check Birthdays"));
    QPushButton *UpdateButton = new QPushButton(tr("Update Birthdays"));
    QPushButton *ClearCacheButton = new QPushButton(tr("Clear Birthdays Cache"));
    QHBoxLayout *layout = new QHBoxLayout();
    vlayout->addWidget(new QLabel(tr("If you use this plugin at first time, make sure that all your accounts \nis online and then press \"Update Birthdays\" button. It takes some time.")));
    layout->addWidget(UpdateButton);
    layout->addWidget(ClearCacheButton);
    layout->addStretch();
    layout->addWidget(CheckButton);
    vlayout->addLayout(layout);
    QHBoxLayout *spinLayout = new QHBoxLayout();
    spinLayout->addWidget(new QLabel(tr("Start notifying")));
    spinLayout->addWidget(DaysWidget);
    spinLayout->addWidget(new QLabel(tr("days in advance")));
    spinLayout->addStretch();
    QHBoxLayout *IntervalLayout = new QHBoxLayout();
    IntervalLayout->addWidget(new QLabel(tr("Check birthdays every")));
    IntervalLayout->addWidget(IntervalWidget);
    IntervalLayout->addWidget(new QLabel(tr("hours")));
    IntervalLayout->addStretch();
    QHBoxLayout *timeoutLayout = new QHBoxLayout();
    timeoutLayout->addWidget(new QLabel(tr("Show popup/notification window")));
    timeoutLayout->addWidget(timeoutWidget);
    timeoutLayout->addWidget(new QLabel(tr("sec (-1=infinite)")));
    timeoutLayout->addStretch();
    StartCheckWidget = new QCheckBox(tr("Check birthdays on startup"));
    CheckFromRosterWidget = new QCheckBox(tr("Remind for contacts from active accounts only"));

    SoundFileWidget = new QLineEdit;
    QPushButton *chooseSound = new QPushButton;
    chooseSound->setIcon(IcoHost->getIcon("psi/browse"));
    QPushButton *playSound = new QPushButton;
    playSound->setIcon(IcoHost->getIcon("psi/play"));
    QHBoxLayout *sLayout = new QHBoxLayout;
    sLayout->addWidget(new QLabel(tr("Play sound:")));
    sLayout->addWidget(SoundFileWidget);
    sLayout->addWidget(chooseSound);
    sLayout->addWidget(playSound);

    QLabel *wikiLink = new QLabel(tr("<a href=\"http://psi-plus.com/wiki/plugins#birthday_reminder_plugin\">Wiki (Online)</a>"));
    wikiLink->setOpenExternalLinks(true);

    vlayout->addLayout(spinLayout);
    vlayout->addLayout(IntervalLayout);
    vlayout->addLayout(UpdateIntervalLayout);
    vlayout->addLayout(timeoutLayout);
    vlayout->addWidget(CheckFromRosterWidget);
    vlayout->addWidget(StartCheckWidget);
    vlayout->addLayout(sLayout);
    vlayout->addStretch();
    vlayout->addWidget(wikiLink);

    connect(UpdateButton,SIGNAL(released()),SLOT(UpdateVCard()));
    connect(CheckButton,SIGNAL(released()),SLOT(Check()));
    connect(ClearCacheButton, SIGNAL(released()), SLOT(clearCache()));
    connect(playSound, SIGNAL(released()), this, SLOT(checkSound()));
    connect(chooseSound, SIGNAL(released()), this, SLOT(getSound()));

    restoreOptions();

    return options;
}

bool Reminder::incomingStanza(int /*account*/, const QDomElement& stanza) {
	if (enabled) {
		if(stanza.tagName() == "iq") {
			if(stanza.attribute("id") == id) {
				QDomNode VCard = stanza.firstChild();
				QDomElement BDay = VCard.firstChildElement("BDAY");
				if(!BDay.isNull()) {
					QString Jid = stanza.attribute("from");
					QString Nick = VCard.firstChildElement("NICKNAME").text();
					QString Date = BDay.text();
					if(Date != "") {
						Jid.replace("@", "_at_");
						QFile file(Dir + QDir::separator() + Jid);
						if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
							QTextStream out(&file);
							out.setGenerateByteOrderMark(false);
							out << Date << "__" << Nick << endl;
						}
					}
				}
				return true;
			}
		}

		if(stanza.tagName() == "presence") {
			if((LastCheck.toLong() + Interval) <= QDateTime::currentDateTime().toString("yyyyMMddhh").toLong()) {
				LastCheck = QDateTime::currentDateTime().toString("yyyyMMddhh");
				psiOptions->setPluginOption(constLastCheck, QVariant(LastCheck));
				Check();
			}
			if(UpdateInterval) {
				if((LastUpdate.toLong() + UpdateInterval) <= QDateTime::currentDateTime().toString("yyyyMMdd").toLong()) {
					LastUpdate = QDateTime::currentDateTime().toString("yyyyMMdd");
					psiOptions->setPluginOption(constLastUpdate, QVariant(LastUpdate));
					UpdateVCard();
				}
			}
		}
	}
	return false;
 }

bool Reminder::outgoingStanza(int /*account*/, QDomElement& /*xml*/)
{
	return false;
}

void Reminder::setAccountInfoAccessingHost(AccountInfoAccessingHost* host) {
     AccInfoHost = host;
 }

void Reminder::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host) {
     AppInfoHost = host;
 }

void Reminder::applyOptions() {

    Days = DaysWidget->value();
    psiOptions->setPluginOption(constDays, QVariant(Days));

    Interval = IntervalWidget->value();
    psiOptions->setPluginOption(constInterval, QVariant(Interval));

    timeout = timeoutWidget->value();
    psiOptions->setPluginOption(consttimeout, QVariant(timeout));

    StartCheck = StartCheckWidget->isChecked();
    psiOptions->setPluginOption(constStartCheck, QVariant(StartCheck));

    CheckFromRoster = CheckFromRosterWidget->isChecked();
    psiOptions->setPluginOption(constCheckFromRoster, QVariant(CheckFromRoster));

    UpdateInterval = UpdateIntervalWidget->value();
    psiOptions->setPluginOption(constUpdateInterval, QVariant(UpdateInterval));

    SoundFile = SoundFileWidget->text();
    psiOptions->setPluginOption(constSoundFile, QVariant(SoundFile));
}

void Reminder::restoreOptions() {
        DaysWidget->setValue(Days);
        IntervalWidget->setValue(Interval);
        timeoutWidget->setValue(timeout);
        StartCheckWidget->setChecked(StartCheck);
        CheckFromRosterWidget->setChecked(CheckFromRoster);
        UpdateIntervalWidget->setValue(UpdateInterval);
        SoundFileWidget->setText(SoundFile);
}

void Reminder::setStanzaSendingHost(StanzaSendingHost *host) {
    StanzaHost = host;
}

void Reminder::UpdateVCard() {
    if(enabled && !updateInProgress) {
	    updateInProgress = true;
	    QString path = AppInfoHost->appVCardDir();
	    QDir dir(path);
	    foreach (QString filename, dir.entryList(QDir::Files)) {
		QFile file(path + QDir::separator() + filename);
		if(file.open(QIODevice::ReadOnly)) {
		    QTextStream in(&file);
		    QDomDocument doc;
		    doc.setContent(in.readAll());
		    QDomElement vCard = doc.documentElement();
		    QDomElement BDay = vCard.firstChildElement("BDAY");
		    if(!BDay.isNull()) {
			QString Nick = vCard.firstChildElement("NICKNAME").text();
			QString Date = BDay.text();
			if(Date != "") {
			    filename.replace("%5f", "_");
			    filename.replace("%2d", "-");
			    filename.replace("%25", "%");
			    filename.remove(".xml");
			    QFile file(Dir + QDir::separator() + filename);
			    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				QTextStream out(&file);
				out.setGenerateByteOrderMark(false);
				out << Date << "__" << Nick << endl;
			    }
			}
		    }
		 }
	    }

	    int accs = -1;
	    bool b = true;
	    while(b) {
		    QStringList Jids = AccInfoHost->getRoster(++accs);
		    if(!Jids.isEmpty()) {
			    if(Jids.first() == "-1") {
				    b = false;
			    }
			    else {
				    if(AccInfoHost->getStatus(accs) != "offline") {
					    foreach(QString Jid, Jids) {
						    QString text = "<iq type=\"get\" to=\"" + Jid + "\" id=\"" + id + "\"><vCard xmlns=\"vcard-temp\" version=\"2.0\" prodid=\"-//HandGen//NONSGML vGen v1.0//EN\" /></iq>";
						    StanzaHost->sendStanza(accs, text);
					    }
				    }
			    }
		    }
	    }
	    QTimer::singleShot(30000, this, SLOT(timeoutStopUpdate())); //30 секунд дольжно хватить, чтобы получить все vCard'ы
    }
}

void Reminder::timeoutStopUpdate() {
	updateInProgress = false;
}

QString Reminder::CheckBirthdays() {
	 if(!enabled)
		 return QString();

	 QSet<QString> Roster_;
	 if(CheckFromRoster) {
		 int accs = -1;
		 bool b = true;
		 while(b) {
			 QStringList Jids = AccInfoHost->getRoster(++accs);
			 if(!Jids.isEmpty()) {
				 if(Jids.first() == "-1") {
					 b = false;
				 }
				 else {
					 Roster_ += Jids.toSet();
				 }
			 }
		 }
	 }

	 QString CheckResult;
	 QDir dir(Dir);
	 foreach(QString jid, dir.entryList(QDir::Files)) {
		 if(jid.contains("_at_")) {
			 QFile file(Dir + QDir::separator() + jid);
			 if(file.open(QIODevice::ReadOnly)) {
				 QTextStream in(&file);
				 QString line = in.readLine();
				 QStringList fields = line.split("__");
				 QString Date = fields.takeFirst();
				 QString Nick = "";
				 if(!fields.isEmpty()) {
					 Nick = fields.takeFirst();
				 }
				 QDate Birthday = QDate::currentDate();
				 if(Date.contains("-")) {
					 Birthday = QDate::fromString(Date, "yyyy-MM-dd");
				 } else {
					 if(Date.contains(".")) {
						 Birthday = QDate::fromString(Date, "d.MM.yyyy");
					 } else {
						 if(Date.contains("/")) {
							 Birthday = QDate::fromString(Date, "d/MM/yyyy");
						 }
					 }
				 }
				 QDate current = QDate::currentDate();
				 if(current != Birthday) {
					 int years = current.year() - Birthday.year();
					 Birthday = Birthday.addYears(years);
					 int daysTo = current.daysTo(Birthday);
					 QString days;
					 days.setNum(daysTo);
					 jid.replace("_at_", "@");
					 if(!CheckFromRoster || Roster_.contains(jid)) {
						 if(daysTo == 0) {
							 CheckResult += Nick + " (" + jid + ") " + tr("celebrates birthday today!") + "\n";
						 } else {
							 if(daysTo <= Days && daysTo > 0) {
								 CheckResult += Nick + " (" + jid + ") " + tr("celebrates birthday in ") + days + tr(" day(s)") + "\n";
							 } else {
								 if(daysTo == -1) {
									 CheckResult += Nick + " (" + jid + ") " + tr("celebrates birthday yesterday.") + "\n";
								 }
							 }
						 }
					 }
				 }
			 }
		 }
	 }
	 return CheckResult;
}

void Reminder::setOptionAccessingHost(OptionAccessingHost *host) {
    psiOptions = host;
}

void Reminder::setIconFactoryAccessingHost(IconFactoryAccessingHost* host) {
    IcoHost = host;
}

bool Reminder::Check() {
	QString text = CheckBirthdays();
	if(text.isEmpty())
		return false;
	text.chop(1);

	if(psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool())
		playSound(SoundFile);


	QVariant delay(timeout*1000);
	int delay_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.delays.status").toInt();
	psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

	bool enbl_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.enabled").toBool();
	QVariant enbl(true);
	psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);

	text = text.replace("\n", "<br>");
	popup->initPopup(text, tr("Birthday Reminder"), "reminder/birthdayicon");

	delay = QVariant(delay_);
	psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

	enbl = QVariant(enbl_);
	psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);

	return true;
}

void Reminder::clearCache() {
    QDir dir(Dir);
    foreach(QString file, dir.entryList(QDir::Files)) {
        QFile File(Dir + QDir::separator() + file);
        if(File.open(QIODevice::ReadWrite)) {
            File.remove();
        }
    }
    LastUpdate = "19010101";
    psiOptions->setPluginOption(constLastUpdate, QVariant(LastUpdate));
}

void Reminder::setPopupAccessingHost(PopupAccessingHost* host) {
    popup = host;
}

void Reminder::playSound(QString f) {
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

void Reminder::getSound() {
	QString fileName = QFileDialog::getOpenFileName(0,tr("Choose a sound file"),"", tr("Sound (*.wav)"));
	if(fileName.isEmpty()) return;
	SoundFileWidget->setText(fileName);
}

void Reminder::checkSound() {
	playSound(SoundFileWidget->text());
}

QString Reminder::pluginInfo() {
	return tr("Author: ") +  "Dealer_WeARE\n"
			+ tr("Email: ") + "wadealer@gmail.com\n\n"
			+ trUtf8("This plugin is designed to show reminders of upcoming birthdays.\n"
			 "The first time you install this plugin, you need to log on to all of your accounts, go to the plugin settings and click \"Update Birthdays\"."
			 "The plugin will then collect the information about the birthdays of all the users in your roster, but when the 'Use vCards cache' option is"
			 "selected, the users' vCards that are cached on your hard disk will be used. ");
}

#include "birthdayreminderplugin.moc"
