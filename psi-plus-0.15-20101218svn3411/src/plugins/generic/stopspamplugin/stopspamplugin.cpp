/*
 * stopspamplugin.cpp - plugin
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

#include <QPointer>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

#include "psiplugin.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "stanzafilter.h"
#include "stanzasender.h"
#include "stanzasendinghost.h"
#include "accountinfoaccessor.h"
#include "accountinfoaccessinghost.h"
#include "applicationinfoaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "popupaccessor.h"
#include "popupaccessinghost.h"
#include "iconfactoryaccessor.h"
#include "iconfactoryaccessinghost.h"
#include "plugininfoprovider.h"
#include "eventfilter.h"
#include "contactinfoaccessinghost.h"
#include "contactinfoaccessor.h"

#include "view.h"
#include "viewer.h"
#include "model.h"

#define cVer "0.4.7"
#define constQuestion "qstn"
#define constAnswer "answr"
#define constUnblocked "UnblockedList"
#define constJids "dsblJids"
#define constselected "slctd"
#define constCounter "cntr"
#define constHeight "Height"
#define constWidth "Width"
#define constCongratulation "cngrtltn"
#define constPopupNotify "popupntf"
#define constInterval "intrvl"
#define constTimes "times"
#define constResetTime "resettm"
#define constLogHistory "lghstr"
#define constDefaultAct "dfltact"

#define constUseMuc "usemuc"
#define constAdmin "affadmin"
#define constModer "rolemoder"
#define constOwner "affowner"
#define constMember "affmember"
#define constParticipant "roleparticipant"
#define constNone "affnone"
#define constVisitor "rolevisitor"
#define constBlockAll "blockall"
#define constBlockAllMes "blockallmes"
#define constEnableBlockAllMes "enableblockallmes"
#define constLastUnblock "lastunblock"


class StopSpam: public QObject, public PsiPlugin, public OptionAccessor, public StanzaSender,  public StanzaFilter,
    public AccountInfoAccessor, public ApplicationInfoAccessor, public PopupAccessor, public IconFactoryAccessor,
    public PluginInfoProvider, public EventFilter, public ContactInfoAccessor
{
	Q_OBJECT
        Q_INTERFACES(PsiPlugin OptionAccessor StanzaSender StanzaFilter AccountInfoAccessor ApplicationInfoAccessor
		     PopupAccessor IconFactoryAccessor PluginInfoProvider EventFilter ContactInfoAccessor)

public:
        StopSpam();
	virtual QString name() const;
	virtual QString shortName() const;
	virtual QString version() const;
	virtual PsiPlugin::Priority priority();
        virtual QWidget* options();
	virtual bool enable();
        virtual bool disable();
        virtual void applyOptions();
        virtual void restoreOptions();
        virtual void setOptionAccessingHost(OptionAccessingHost* host);
	virtual void optionChanged(const QString& ) {};
        virtual void setStanzaSendingHost(StanzaSendingHost *host);
        virtual bool incomingStanza(int account, const QDomElement& xml);
	virtual bool outgoingStanza(int account, QDomElement& xml);
        virtual void setAccountInfoAccessingHost(AccountInfoAccessingHost* host);
        virtual void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host);
        virtual void setPopupAccessingHost(PopupAccessingHost* host);
        virtual void setIconFactoryAccessingHost(IconFactoryAccessingHost* host);
	virtual void setContactInfoAccessingHost(ContactInfoAccessingHost* host);
	virtual QString pluginInfo();

	virtual bool processEvent(int , QDomElement& ) { return false; }
	virtual bool processMessage(int , const QString& , const QString& , const QString& ) { return false; };
	virtual bool processOutgoingMessage(int account, const QString& fromJid, QString& body, const QString& type, QString& subject);
	virtual void logout(int ) {};

private:
	bool enabled;
	OptionAccessingHost* psiOptions;        
        StanzaSendingHost* StanzaHost;
        AccountInfoAccessingHost *AccInfoHost;
        ApplicationInfoAccessingHost *AppInfoHost;
        IconFactoryAccessingHost* icoHost;
        PopupAccessingHost* popup;
	ContactInfoAccessingHost* contactInfo;

        QTextEdit *questionWidget;
        QLineEdit *answerWidget;
        QString Question;
        QString Answer;
        QString Unblocked;
        QStringList Jids;
        QVariantList selected;
        int Counter;
        QLineEdit *countWid;
        struct Blocked {
            int Acc;
            QString Jid;
            int count;
            QDateTime LastMes;
        };
        QVector<Blocked> BlockedJids;        


        QPointer<ViewLog> viewer;
        int Height;
        int Width;
        QString Congratulation;
	QTextEdit *CongratulationWidget;
        QCheckBox *PopupNotifyWidget;
        bool PopupNotify, DefaultAct;
        int Interval;
        QSpinBox *IntervalWidget;
        int Times;
        int ResetTime;
        QSpinBox *spinWidget;
        QSpinBox *resetWidget;
        bool LogHistory;
        QCheckBox *LogHistoryWidget, *DefaultActWidget;

        void updateCounter(QDomElement stanza, bool b);
        bool FindAcc(int account, QString Jid, int &i);
        bool FindMuc(QString mucJid, QString nick, int &i);
        void logHistory(QDomElement stanza);

        bool processMuc(int account, const QDomElement& stanza);
        bool UseMuc, BlockAll, EnableBlockAllMes;
        bool Admin, Owner, None, Member;
        bool Moder, Participant, Visitor;
        QString BlockAllMes;
        QCheckBox *AdminWidget, *OwnerWidget, *NoneWidget, *MemberWidget;
        QCheckBox *ModerWidget, *ParticipantWidget, *VisitorWidget;
        QCheckBox *UseMucWidget, *BlockAllWidget, *EnableBlockAllMesWidget;
        QTextEdit *BlockAllMesWidget;
        QGroupBox *affBox, *roleBox;

	QPointer<Viewer> viewWidget;
        Model *model_;        

        struct MucUser {
            QString mucJid;
            QString nick;
            QString jid;
            QString role;
            QString affiliation;
        };
	QVector<MucUser> mucUsers_;

	bool findMucNS(const QDomElement& stanza);

 private slots:
        void resetCounter();
        void view();
        void close(int w, int h);
        void changeWidgetsState();
        void addRow();
        void removeRow();
        void hack();
        void onOptionsClose();
    };

Q_EXPORT_PLUGIN(StopSpam);

StopSpam::StopSpam() {
        enabled = false;        
        Question = "2+3=?";
        Answer = "5";
        questionWidget = 0;
        viewWidget = 0;
        model_ = 0;
        answerWidget = 0;
        psiOptions = 0;        
        StanzaHost = 0;
        AccInfoHost = 0;
        AppInfoHost = 0;
        icoHost = 0;
        popup = 0;
	contactInfo = 0;
        Unblocked = "";
        Jids.clear();
        Jids.append("");
        selected.clear();
        Counter = 0;
        countWid = 0;
        viewer = 0;
        Height = 500;
        Width = 600;
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForLocale(codec);
        Congratulation = "Congratulations! Now you can chat!";
        CongratulationWidget = 0;
        PopupNotify = true;
        PopupNotifyWidget = 0;
        Interval = 5;
        IntervalWidget = 0;
        DefaultActWidget = 0;
        Times = 2;
        ResetTime = 5;
        spinWidget = 0;
        resetWidget = 0;
        LogHistory = false;
        LogHistoryWidget = 0;
        DefaultAct = false;

        UseMuc = false;
        BlockAll = false;
        Admin = false;
        Owner = false;
        None = true;
        Member = false;
        Moder = false;
        Participant = true;
        Visitor = true;
        UseMucWidget = 0;
        BlockAllWidget = 0;
        AdminWidget = 0;
        OwnerWidget = 0;
        NoneWidget = 0;
        MemberWidget = 0;
        ModerWidget = 0;
        ParticipantWidget = 0;
        VisitorWidget = 0;
        BlockAllMesWidget = 0;
        EnableBlockAllMesWidget = 0;
        BlockAllMes = "The private messages are blocked! Send your message to groupchat, please.";
        EnableBlockAllMes = true;
        affBox = 0;
        roleBox = 0;

    }

QString StopSpam::name() const {
        return "Stop Spam Plugin";
    }

QString StopSpam::shortName() const {
        return "stopspam";
}

QString StopSpam::version() const {
        return cVer;
}

PsiPlugin::Priority StopSpam::priority()
{
	return PriorityHighest;
}

bool StopSpam::enable() {
    if (psiOptions) {
        enabled = true;

        Question = psiOptions->getPluginOption(constQuestion, QVariant(Question)).toString();
        Answer = psiOptions->getPluginOption(constAnswer, QVariant(Answer)).toString();
        Congratulation = psiOptions->getPluginOption(constCongratulation, QVariant(Congratulation)).toString();
        Unblocked = psiOptions->getPluginOption(constUnblocked, QVariant(Unblocked)).toString();
        DefaultAct = psiOptions->getPluginOption(constDefaultAct, QVariant(DefaultAct)).toBool();
        Height = psiOptions->getPluginOption(constHeight, QVariant(Height)).toInt();
        Width = psiOptions->getPluginOption(constWidth, QVariant(Width)).toInt();
        PopupNotify = psiOptions->getPluginOption(constPopupNotify, QVariant(PopupNotify)).toBool();
        Interval = psiOptions->getPluginOption(constInterval, QVariant(Interval)).toInt();
        Times = psiOptions->getPluginOption(constTimes, QVariant(Times)).toInt();
        ResetTime = psiOptions->getPluginOption(constResetTime, QVariant(ResetTime)).toInt();
        LogHistory = psiOptions->getPluginOption(constLogHistory, QVariant(LogHistory)).toBool();

        UseMuc = psiOptions->getPluginOption(constUseMuc, QVariant(UseMuc)).toBool();
        BlockAll = psiOptions->getPluginOption(constBlockAll, QVariant(BlockAll)).toBool();
        Admin = psiOptions->getPluginOption(constAdmin, QVariant(Admin)).toBool();
        Owner = psiOptions->getPluginOption(constOwner, QVariant(Owner)).toBool();
        None = psiOptions->getPluginOption(constNone, QVariant(None)).toBool();
        Member = psiOptions->getPluginOption(constMember, QVariant(Member)).toBool();
        Moder = psiOptions->getPluginOption(constModer, QVariant(Moder)).toBool();
        Participant = psiOptions->getPluginOption(constParticipant, QVariant(Participant)).toBool();
        Visitor = psiOptions->getPluginOption(constVisitor, QVariant(Visitor)).toBool();
        BlockAllMes = psiOptions->getPluginOption(constBlockAllMes, QVariant(BlockAllMes)).toString();
        EnableBlockAllMes = psiOptions->getPluginOption(constEnableBlockAllMes, QVariant(EnableBlockAllMes)).toBool();

	QDate luTime = QDate::fromString(psiOptions->getPluginOption( constLastUnblock,
					QVariant(QDate::currentDate().toString("yyyyMMdd")) ).toString(), "yyyyMMdd");
	if(!Unblocked.isEmpty() && luTime.daysTo(QDate::currentDate()) > 3) {
		Unblocked.clear();
		psiOptions->setPluginOption(constUnblocked, QVariant(Unblocked));
	}

        Jids = psiOptions->getPluginOption(constJids, QVariant(Jids)).toStringList();
        selected = psiOptions->getPluginOption(constselected, QVariant(selected)).value<QVariantList>();
        model_ = new Model(Jids, selected, this);
        connect(model_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(hack()));
    }
    return enabled;
}

bool StopSpam::disable() {
    if(viewer) {
        delete(viewer);
        viewer = 0;
    }
    if(viewWidget) {
	    delete(viewWidget);
    }
	enabled = false;        
	return true;
}

void StopSpam::applyOptions() {
    if (!answerWidget || !questionWidget || !countWid || !CongratulationWidget || !viewWidget || !DefaultActWidget
        || !PopupNotifyWidget || !IntervalWidget || !spinWidget || !resetWidget || !LogHistoryWidget
        || !UseMucWidget || !AdminWidget || !OwnerWidget || !NoneWidget || !MemberWidget
        || !ModerWidget || !ParticipantWidget || !VisitorWidget || !BlockAllWidget
        || !BlockAllMesWidget || !EnableBlockAllMesWidget)   return;

    QVariant vQuestion(questionWidget->toPlainText());
    psiOptions->setPluginOption(constQuestion, vQuestion);
    Question = vQuestion.toString();
    QVariant vAnswer(answerWidget->text());
    psiOptions->setPluginOption(constAnswer, vAnswer);
    Answer = vAnswer.toString();
    QVariant vCongratulation(CongratulationWidget->toPlainText());
    psiOptions->setPluginOption(constCongratulation, vCongratulation);
    Congratulation = vCongratulation.toString();
    DefaultAct = DefaultActWidget->isChecked();
    psiOptions->setPluginOption(constDefaultAct, QVariant(DefaultAct));
    QVariant vPopupNotify(PopupNotifyWidget->isChecked());
    psiOptions->setPluginOption(constPopupNotify, vPopupNotify);
    PopupNotify = vPopupNotify.toBool();
    QVariant vInterval(IntervalWidget->value());
    psiOptions->setPluginOption(constInterval, vInterval);
    Interval = vInterval.toInt();
    QVariant vTimes(spinWidget->value());
    psiOptions->setPluginOption(constTimes, vTimes);
    Times = vTimes.toInt();
    QVariant vResetTime(resetWidget->value());
    psiOptions->setPluginOption(constResetTime, vResetTime);
    ResetTime = vResetTime.toInt();
    QVariant vLogHistory(LogHistoryWidget->isChecked());
    psiOptions->setPluginOption(constLogHistory, vLogHistory);
    LogHistory = vLogHistory.toBool();

    QVariant vUseMuc(UseMucWidget->isChecked());
    psiOptions->setPluginOption(constUseMuc, vUseMuc);
    UseMuc = vUseMuc.toBool();
    QVariant vBlockAll(BlockAllWidget->isChecked());
    psiOptions->setPluginOption(constBlockAll, vBlockAll);
    BlockAll = vBlockAll.toBool();

    QVariant vAdmin(AdminWidget->isChecked());
    psiOptions->setPluginOption(constAdmin, vAdmin);
    Admin = vAdmin.toBool();
    QVariant vOwner(OwnerWidget->isChecked());
    psiOptions->setPluginOption(constOwner, vOwner);
    Owner = vOwner.toBool();
    QVariant vNone(NoneWidget->isChecked());
    psiOptions->setPluginOption(constNone, vNone);
    None = vNone.toBool();
    QVariant vMember(MemberWidget->isChecked());
    psiOptions->setPluginOption(constMember, vMember);
    Member = vMember.toBool();

    QVariant vModer(ModerWidget->isChecked());
    psiOptions->setPluginOption(constModer, vModer);
    Moder = vModer.toBool();
    QVariant vParticipant(ParticipantWidget->isChecked());
    psiOptions->setPluginOption(constParticipant, vParticipant);
    Participant = vParticipant.toBool();
    QVariant vVisitor(VisitorWidget->isChecked());
    psiOptions->setPluginOption(constVisitor, vVisitor);
    Visitor = vVisitor.toBool();

    QVariant vEnableBlockAll(EnableBlockAllMesWidget->isChecked());
    psiOptions->setPluginOption(constEnableBlockAllMes, vEnableBlockAll);
    EnableBlockAllMes = vEnableBlockAll.toBool();
    QVariant vBlockAllMes(BlockAllMesWidget->toPlainText());
    psiOptions->setPluginOption(constBlockAllMes, vBlockAllMes);
    BlockAllMes = vBlockAllMes.toString();

    model_->apply();
    Jids = model_->getJids();
    selected = model_->enableFor();
    psiOptions->setPluginOption(constJids, Jids);
    psiOptions->setPluginOption(constselected, selected);
}

void StopSpam::restoreOptions() {
    if (!answerWidget || !questionWidget || !countWid || !CongratulationWidget || !viewWidget || !DefaultActWidget
        || !PopupNotifyWidget || !IntervalWidget || !spinWidget || !resetWidget || !LogHistoryWidget
        || !UseMucWidget || !AdminWidget || !OwnerWidget || !NoneWidget || !MemberWidget
        || !ModerWidget || !ParticipantWidget || !VisitorWidget || !BlockAllWidget
        || !BlockAllMesWidget || !EnableBlockAllMesWidget)   return;

        questionWidget->setText(psiOptions->getPluginOption(constQuestion, QVariant(Question)).toString());
        answerWidget->setText(psiOptions->getPluginOption(constAnswer, QVariant(Answer)).toString());
        CongratulationWidget->setText(psiOptions->getPluginOption(constCongratulation, QVariant(Congratulation)).toString());
        DefaultActWidget->setChecked(psiOptions->getPluginOption(constDefaultAct, QVariant(DefaultAct)).toBool());
        PopupNotifyWidget->setChecked(psiOptions->getPluginOption(constPopupNotify, QVariant(PopupNotify)).toBool());
        IntervalWidget->setValue(psiOptions->getPluginOption(constInterval, QVariant(Interval)).toInt());
        spinWidget->setValue(psiOptions->getPluginOption(constTimes, QVariant(Times)).toInt());
        resetWidget->setValue(psiOptions->getPluginOption(constResetTime, QVariant(ResetTime)).toInt());
        LogHistoryWidget->setChecked(psiOptions->getPluginOption(constLogHistory, QVariant(LogHistory)).toBool());

        UseMucWidget->setChecked(psiOptions->getPluginOption(constUseMuc, QVariant(UseMuc)).toBool());
        BlockAllWidget->setChecked(psiOptions->getPluginOption(constBlockAll, QVariant(BlockAll)).toBool());

        AdminWidget->setChecked(psiOptions->getPluginOption(constAdmin, QVariant(Admin)).toBool());
        OwnerWidget->setChecked(psiOptions->getPluginOption(constOwner, QVariant(Owner)).toBool());
        NoneWidget->setChecked(psiOptions->getPluginOption(constNone, QVariant(None)).toBool());
        MemberWidget->setChecked(psiOptions->getPluginOption(constMember, QVariant(Member)).toBool());

        ModerWidget->setChecked(psiOptions->getPluginOption(constModer, QVariant(Moder)).toBool());
        ParticipantWidget->setChecked(psiOptions->getPluginOption(constParticipant, QVariant(Participant)).toBool());
        VisitorWidget->setChecked(psiOptions->getPluginOption(constVisitor, QVariant(Visitor)).toBool());

        EnableBlockAllMesWidget->setChecked(psiOptions->getPluginOption(constEnableBlockAllMes, QVariant(EnableBlockAllMes)).toBool());
        BlockAllMesWidget->setText(psiOptions->getPluginOption(constBlockAllMes, QVariant(BlockAllMes)).toString());

        model_->reset();
}

QWidget* StopSpam::options() {
        if (!enabled) {
                return 0;
        }
        QWidget *optionsWid = new QWidget();
        connect(optionsWid, SIGNAL(destroyed()), this, SLOT(onOptionsClose()));

        QVBoxLayout *layout = new QVBoxLayout(optionsWid);
        QTabWidget *tabWidget = new QTabWidget;
        QWidget *tab1 = new QWidget;
        QWidget *tab2 = new QWidget;
        QWidget *tab3 = new QWidget;
        QVBoxLayout *tab1Layout = new QVBoxLayout(tab1);
        QVBoxLayout *tab2Layout = new QVBoxLayout(tab2);
        QVBoxLayout *tab3Layout = new QVBoxLayout(tab3);
        tabWidget->addTab(tab1, tr("Main"));
        tabWidget->addTab(tab2, tr("MUC"));
        tabWidget->addTab(tab3, tr("Misc"));

        //tab1-------------------------
	CongratulationWidget = new QTextEdit();
	CongratulationWidget->setText(Congratulation);
	CongratulationWidget->setMaximumHeight(70);
        answerWidget = new QLineEdit();
        answerWidget->setText(Answer);
        questionWidget = new QTextEdit();
        questionWidget->setText(Question);


        tab1Layout->addWidget(new QLabel(tr("Question:")));
        tab1Layout->addWidget(questionWidget);
        tab1Layout->addWidget(new QLabel(tr("Answer:")));
        tab1Layout->addWidget(answerWidget);
        tab1Layout->addWidget(new QLabel(tr("Congratulation:")));
        tab1Layout->addWidget(CongratulationWidget);

        //tab2-------------------
        UseMucWidget = new QCheckBox(tr("Enable Stop Spam for private messages in MUC"));
        UseMucWidget->setChecked(UseMuc);

        BlockAllWidget = new QCheckBox(tr("Block all private messages"));
        BlockAllWidget->setChecked(BlockAll);

        affBox = new QGroupBox(tr("Enable Stop Spam for affiliations:"));
        QHBoxLayout *affLayout = new QHBoxLayout(affBox);
        MemberWidget = new QCheckBox(tr("Member"));
        MemberWidget->setChecked(Member);
        AdminWidget = new QCheckBox(tr("Admin"));
        AdminWidget->setChecked(Admin);
        OwnerWidget = new QCheckBox(tr("Owner"));
        OwnerWidget->setChecked(Owner);
        NoneWidget = new QCheckBox(tr("None"));
        NoneWidget->setChecked(None);
        affLayout->addWidget(OwnerWidget);
        affLayout->addWidget(AdminWidget);
        affLayout->addWidget(MemberWidget);
        affLayout->addWidget(NoneWidget);
        affLayout->addStretch();

        roleBox = new QGroupBox(tr("Enable Stop Spam for roles:"));
        QHBoxLayout *roleLayout = new QHBoxLayout(roleBox);
        ModerWidget = new QCheckBox(tr("Moderator"));
        ModerWidget->setChecked(Moder);
        ParticipantWidget = new QCheckBox(tr("Participant"));
        ParticipantWidget->setChecked(Participant);
        VisitorWidget = new QCheckBox(tr("Visitor"));
        VisitorWidget->setChecked(Visitor);
        roleLayout->addWidget(ModerWidget);
        roleLayout->addWidget(ParticipantWidget);
        roleLayout->addWidget(VisitorWidget);
        roleLayout->addStretch();

        BlockAllMesWidget = new QTextEdit;
        BlockAllMesWidget->setPlainText(BlockAllMes);
        BlockAllMesWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        EnableBlockAllMesWidget = new QCheckBox(tr("Send message:"));
        EnableBlockAllMesWidget->setChecked(EnableBlockAllMes);

        tab2Layout->addWidget(UseMucWidget);        
        tab2Layout->addWidget(affBox);
        tab2Layout->addWidget(roleBox);
        tab2Layout->addWidget(BlockAllWidget);
        tab2Layout->addWidget(EnableBlockAllMesWidget);
        tab2Layout->addWidget(BlockAllMesWidget);

        //tab3---------------------
        countWid = new QLineEdit();
        QVariant vCounter(Counter);
        vCounter = psiOptions->getPluginOption(constCounter);
        if (!vCounter.isNull()) {
            countWid->setText(vCounter.toString());
        }
        else {
            countWid->setText("0");
        }
        countWid->setProperty("isOption", QVariant(false));
	countWid->setReadOnly(true);
        countWid->setMaximumWidth(60);
        QPushButton *resetButton = new QPushButton(tr("Reset"));
        connect(resetButton, SIGNAL(released()), SLOT(resetCounter()));
        QPushButton *viewButton = new QPushButton(tr("View Log"));
        connect(viewButton, SIGNAL(released()), SLOT(view()));
        QHBoxLayout *count = new QHBoxLayout;
        count->addWidget(new QLabel(tr("Number of blocked stanzas:")));
        count->addWidget(countWid);
        count->addWidget(viewButton);
        count->addWidget(resetButton);
        count->addStretch();

        PopupNotifyWidget = new QCheckBox(tr("Enable popup notifications"));
        PopupNotifyWidget->setChecked(PopupNotify);
        IntervalWidget = new QSpinBox();
        IntervalWidget->setMinimum(1);
        IntervalWidget->setValue(Interval);
        QHBoxLayout *IntervalLayout = new QHBoxLayout;
        IntervalLayout->addWidget(new QLabel(tr("Show popup")));
        IntervalLayout->addWidget(IntervalWidget);
        IntervalLayout->addWidget(new QLabel(tr("sec")));
        IntervalLayout->addStretch();
        LogHistoryWidget = new QCheckBox(tr("Log blocked messages to contact's history"));
        LogHistoryWidget->setChecked(LogHistory);
        spinWidget = new QSpinBox();
        spinWidget->setMinimum(1);
        spinWidget->setValue(Times);
        QHBoxLayout *timesLayout = new QHBoxLayout;
        timesLayout->addWidget(new QLabel(tr("Send maximum")));
        timesLayout->addWidget(spinWidget);
        timesLayout->addWidget(new QLabel(tr("times")));
        timesLayout->addStretch();
        resetWidget = new QSpinBox();
        resetWidget->setMaximum(2000);
        resetWidget->setMinimum(1);
        resetWidget->setValue(ResetTime);
        QHBoxLayout *resetLayout = new QHBoxLayout;
        resetLayout->addWidget(new QLabel(tr("Timeout to reset counter:")));
        resetLayout->addWidget(resetWidget);
        resetLayout->addWidget(new QLabel(tr("min.")));
        resetLayout->addStretch();

        viewWidget = new Viewer(optionsWid);
        // viewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        viewWidget->setModel(model_);
        viewWidget->init();

        QPushButton *addButton = new QPushButton(tr("Add"));
        QPushButton *delButton = new QPushButton(tr("Del"));
        DefaultActWidget = new QCheckBox(tr("Disable stopspam, if don't match any rule above\n"
                                            "(does not work for muc private messages)"));
        DefaultActWidget->setChecked(DefaultAct);
        QHBoxLayout *butLayout = new QHBoxLayout;
        butLayout->addWidget(DefaultActWidget);
        butLayout->addStretch();
        butLayout->addWidget(addButton);
        butLayout->addWidget(delButton);

        tab3Layout->addLayout(count);
        tab3Layout->addLayout(timesLayout);
        tab3Layout->addLayout(resetLayout);
        tab3Layout->addWidget(PopupNotifyWidget);
        tab3Layout->addLayout(IntervalLayout);
        tab3Layout->addWidget(LogHistoryWidget);
        tab3Layout->addWidget(new QLabel(tr("Rules:")));
        tab3Layout->addWidget(viewWidget);
        tab3Layout->addLayout(butLayout);

        QLabel *wikiLink = new QLabel(tr("<a href=\"http://psi-plus.com/wiki/plugins#stop_spam_plugin\">Wiki (Online)</a>"));
        wikiLink->setOpenExternalLinks(true);

        layout->addWidget(tabWidget);
        layout->addWidget(wikiLink);

        connect(EnableBlockAllMesWidget, SIGNAL(stateChanged(int)), this, SLOT(changeWidgetsState()));
        connect(UseMucWidget, SIGNAL(stateChanged(int)), this, SLOT(changeWidgetsState()));
        connect(BlockAllWidget, SIGNAL(stateChanged(int)), this, SLOT(changeWidgetsState()));

        connect(addButton, SIGNAL(released()), this, SLOT(addRow()));
        connect(delButton, SIGNAL(released()), this, SLOT(removeRow()));

        changeWidgetsState();

        return optionsWid;
    }

void StopSpam::setOptionAccessingHost(OptionAccessingHost* host) {
	psiOptions = host;
}

void StopSpam::setIconFactoryAccessingHost(IconFactoryAccessingHost* host) {
    icoHost = host;
}

void StopSpam::setPopupAccessingHost(PopupAccessingHost* host) {
    popup = host;
}

void StopSpam::setStanzaSendingHost(StanzaSendingHost *host) {
    StanzaHost = host;
}

void StopSpam::setAccountInfoAccessingHost(AccountInfoAccessingHost* host) {
    AccInfoHost = host;
}

void StopSpam::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host) {
     AppInfoHost = host;
}

void StopSpam::setContactInfoAccessingHost(ContactInfoAccessingHost *host) {
	contactInfo = host;
}

bool StopSpam::incomingStanza(int account, const QDomElement& stanza) {
     if (enabled) {	     
         if(stanza.tagName() == "iq") {
             QDomElement query = stanza.firstChildElement("query");
	     if(!Unblocked.isEmpty()
		 && !query.isNull()
		 && query.attribute("xmlns") == "jabber:iq:roster") {
                     QStringList Roster = AccInfoHost->getRoster(account);
                     QStringList UnblockedList = Unblocked.split("\n");
                     while(!Roster.isEmpty()) {
                         QString jid = Roster.takeFirst();
                         UnblockedList.removeOne(jid);
                     }
                     Unblocked = "";
                     while(!UnblockedList.isEmpty()) {
                         QString jid = UnblockedList.takeFirst();
                         if(jid != "") {
                             Unblocked += jid + "\n";
                         }
                     }
		     psiOptions->setPluginOption(constUnblocked, QVariant(Unblocked));
                 }
             }


             QString from = stanza.attribute("from");
             QString to = stanza.attribute("to");
             QString valF = from.split("/").takeFirst();
             QString valT = to.split("/").takeFirst();

	     if(valF.toLower() == valT.toLower()
		     || valF.toLower() == AccInfoHost->getJid(account).toLower())  return false;

             if(!from.contains("@")) return false;

	     if(contactInfo->isConference(account, valF)
		     || contactInfo->isPrivate(account, from)
		     || findMucNS(stanza))
	    {
                 if(UseMuc)
                         return processMuc(account, stanza);
                 else
                     return false;
             }

             QStringList Roster = AccInfoHost->getRoster(account);
	     if(Roster.isEmpty() || Roster.contains("-1")) return false;
             if(Roster.contains(valF, Qt::CaseInsensitive)) return false;

             QStringList UnblockedJids = Unblocked.split("\n");
             if(UnblockedJids.contains(valF, Qt::CaseInsensitive)) return false;

             bool findRule = false;
             for(int i = 0; i < Jids.size(); i++) {
                 QString jid_ = Jids.at(i);
                 if(jid_.isEmpty())
                     continue;
                 if(from.contains(jid_, Qt::CaseInsensitive)) {
                     findRule = true;
                     if(!selected[i].toBool())
                         return false;
                     break;
                 }
             }
             if(!findRule && DefaultAct)
                 return false;

	  if (stanza.tagName() == "message") {
	     QString subj = stanza.firstChildElement("subject").text();
             QString type = "";
             type = stanza.attribute("type");
	     if(type == "error" && subj == "StopSpam Question") {
                 updateCounter(stanza, false);
                 return true;
             }

	     if (subj == "AutoReply" || subj == "StopSpam" || subj == "StopSpam Question") return false;

             if(type == "groupchat" || type == "error")  return false;


	     bool findInvite = false;
	     QString invFrom;
	     QDomElement x = stanza.firstChildElement("x");
	     while(!x.isNull()) {
		     QDomElement invite = x.firstChildElement("invite");
		     if(!invite.isNull()) {
			     findInvite = true;
			     invFrom = invite.attribute("from");
			     break;
		     }
		     x = x.nextSiblingElement("x");
	     }
	     if(findInvite) {  // invite to MUC
		     if(Roster.contains(invFrom.split("/").first(), Qt::CaseInsensitive))
			     return false;
		     else {
			     bool findRule = false;
			     for(int i = 0; i < Jids.size(); i++) {
				 QString jid_ = Jids.at(i);
				 if(jid_.isEmpty())
				     continue;
				 if(invFrom.contains(jid_, Qt::CaseInsensitive)) {
				     findRule = true;
				     if(!selected[i].toBool())
					 return false;
				     break;
				 }
			     }
			     if(!findRule && DefaultAct)
				     return false;
			     else {
				     updateCounter(stanza, true);
				     return true;
			     }
		     }
	     }

	     QDomElement captcha = stanza.firstChildElement("captcha");
	     if(!captcha.isNull() && captcha.attribute("xmlns") == "urn:xmpp:captcha")
		     return false; // CAPTCHA

             QDomElement Body = stanza.firstChildElement("body");
             if(!Body.isNull()) {
                 QString BodyText = Body.text();
                 if(BodyText == Answer) {
                     Unblocked += valF + "\n";
		     psiOptions->setPluginOption(constUnblocked, QVariant(Unblocked));
		     psiOptions->setPluginOption(constLastUnblock, QVariant(QDate::currentDate().toString("yyyyMMdd")));
                     StanzaHost->sendMessage(account, from,  Qt::escape(Congratulation), "StopSpam", "chat");
                     updateCounter(stanza, true);
		     if(LogHistory) logHistory(stanza);
                     return true;
                     } else {
                         int i = BlockedJids.size();
                         if(FindAcc(account, valF, i)) {
                             Blocked &B = BlockedJids[i];
                             if(B.count < Times) {
                                 StanzaHost->sendMessage(account, from,  Qt::escape(Question), "StopSpam Question", "chat");
                                 updateCounter(stanza, false);
				 if(LogHistory) logHistory(stanza);
                                 B.count++;
                                 B.LastMes = QDateTime::currentDateTime();
                                 return true;
                             } else {
                                 if(QDateTime::currentDateTime().secsTo(B.LastMes) >= -ResetTime*60) {
                                     updateCounter(stanza, false);
                                     return true;
                                 } else {
                                     B.count = 1;
                                     B.LastMes = QDateTime::currentDateTime();
                                     StanzaHost->sendMessage(account, from,  Qt::escape(Question), "StopSpam Question", "chat");
                                     updateCounter(stanza, false);
				     if(LogHistory) logHistory(stanza);
                                     return true;
                                 }
                             }
                         }
                         else {
                             Blocked B = { account, valF, 1, QDateTime::currentDateTime() };
                             BlockedJids << B;
                             StanzaHost->sendMessage(account, from,  Qt::escape(Question), "StopSpam Question", "chat");
                             updateCounter(stanza, false);
			     if(LogHistory) logHistory(stanza);
                             return true;
                         }
                    }
                 }
             updateCounter(stanza, false);
             return true;
         }

	  if (stanza.tagName() == "presence") {
		QString type = stanza.attribute("type");
		if(type == "subscribe") {
			StanzaHost->sendMessage(account, from,  Qt::escape(Question), "StopSpam Question", "chat");
			StanzaHost->sendStanza(account, "<presence type=\"unsubscribed\" to=\"" + valF + "\" />");
			updateCounter(stanza, false);
			if(LogHistory)
				logHistory(stanza);
			return true;
		}
		else
			return false;
          }

	  if (stanza.tagName() == "iq" && stanza.attribute("type") == "set") {
		  QString msg = QString("<iq type=\"error\" id=\"%1\" ").arg(stanza.attribute("id"));
		  if(!from.isEmpty())
			  msg += QString("to=\"%1\"").arg(from);
		  msg += " />";
		  StanzaHost->sendStanza(account, msg);
		  updateCounter(stanza, false);
		  return true;
	  }

          return false;
      }
     return false;
 }

bool StopSpam::findMucNS(const QDomElement &stanza)
{
	bool find = false;
	QDomNodeList nodeList = stanza.elementsByTagName("x");
	for(int i = 0; i < nodeList.size(); i++) {
		QDomElement item = nodeList.at(i).toElement();
		if(!item.isNull() && item.attribute("xmlns").contains("http://jabber.org/protocol/muc")) {
			find = true;
			break;
		}
	}

	return find;
}

bool StopSpam::outgoingStanza(int /*account*/, QDomElement& /*xml*/)
{
	return false;
}

bool StopSpam::processOutgoingMessage(int acc, const QString &fromJid, QString &body, const QString &type, QString &/*subject*/) {
	if(enabled && type != "groupchat" && !body.isEmpty()) {
		QString bareJid;
		if(contactInfo->isPrivate(acc, fromJid)) {
			bareJid = fromJid;
		}
		else {
			bareJid =  fromJid.split("/").first();
			if(contactInfo->inList(acc, bareJid))
				return false;
		}
		if(!Unblocked.split("\n").contains(bareJid, Qt::CaseInsensitive)) {
			Unblocked += bareJid + "\n";
			psiOptions->setPluginOption(constUnblocked, QVariant(Unblocked));
			psiOptions->setPluginOption(constLastUnblock, QVariant(QDate::currentDate().toString("yyyyMMdd")));
		}
	}
	return false;
}

 void StopSpam::updateCounter(QDomElement stanza, bool b) {
     Counter = psiOptions->getPluginOption(constCounter, QVariant(Counter)).toInt();
     Counter++;
     psiOptions->setPluginOption(constCounter, QVariant(Counter));
     QString path = AppInfoHost->appProfilesDir();
     QFile file(path + QDir::separator() + "Blockedstanzas.log");
     if(file.open(QIODevice::WriteOnly | QIODevice::Append)) {
	 QString date = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
         QTextStream out(&file);
	 //out.seek(file.size());
         out.setGenerateByteOrderMark(false);
	 out << date << endl << stanza << endl;
     }     

     if(!PopupNotify) return;

     QVariant delay(Interval*1000);
     int delay_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.delays.status").toInt();
     psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

     bool enbl_ = psiOptions->getGlobalOption("options.ui.notifications.passive-popups.enabled").toBool();
     QVariant enbl(true);
     psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);


     if(!b) {
         QString popupText = tr("Block stanza from ") + stanza.attribute("from");
         popup->initPopup(popupText, "Stop Spam Plugin");
     } else {
         QString popupText =  stanza.attribute("from") + tr(" pass the test");
         popup->initPopup(popupText, "Stop Spam Plugin");
     }

     delay = QVariant(delay_);
     psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", delay);

     enbl = QVariant(enbl_);
     psiOptions->setGlobalOption("options.ui.notifications.passive-popups.enabled", enbl);
 }

 bool StopSpam::FindAcc(int account, QString Jid, int &i) {
     for(; i > 0;) {
         Blocked Block =  BlockedJids[--i];
         if(Block.Acc == account && Block.Jid == Jid) {
             return true;
         }
     }
     return false;
 }

 void StopSpam::resetCounter() {
     Counter = 0;
     psiOptions->setPluginOption(constCounter, QVariant(Counter));
     countWid->setText("0");
 }

 void StopSpam::view() {
     if(viewer)
          viewer->raise();
     else {
         QString path = AppInfoHost->appProfilesDir() + QDir::separator() + "Blockedstanzas.log";
         viewer = new ViewLog(path, icoHost);
         connect(viewer, SIGNAL(onClose(int, int)), this, SLOT(close(int,int)));
         if(!viewer->init()) return;
         viewer->resize(Width, Height);
         viewer->show();
     }
  }

 void StopSpam::close(int width, int height) {
    Height = height;
    Width = width;
    psiOptions->setPluginOption(constHeight, QVariant(Height));
    psiOptions->setPluginOption(constWidth, QVariant(Width));
}

 void StopSpam::logHistory(QDomElement stanza) {
     QString folder = AppInfoHost->appHistoryDir();
     QString filename = stanza.attribute("from").split("/").takeFirst() + QString::fromUtf8(".history");
     filename.replace("%", "%25");
     filename.replace("_", "%5f");
     filename.replace("-", "%2d");
     filename.replace("@", "_at_");
     QFile file(folder + QDir::separator() + filename);
     if(!file.open(QIODevice::WriteOnly | QIODevice::Append))
	     return;

     QString time = QDateTime::currentDateTime().toString("|yyyy-MM-ddThh:mm:ss|");
     QString type;
     if(stanza.tagName() == "presence")
	     type = "3|";
     else
	     type = "1|";
     QString body;
     if((body = stanza.firstChildElement("body").text()).isEmpty())
	     body = "subscribe";
     QString outText = time + type + QString::fromUtf8("from|N---|") + body;
     QTextStream out(&file);
     //out.seek(file.size());
     out.setGenerateByteOrderMark(false);
     out << outText << endl;
 }

 bool StopSpam::processMuc(int account, const QDomElement& stanza) {
     if(stanza.tagName() == "presence") {
         QStringList jidList = stanza.attribute("from").split("/");
         int i =  mucUsers_.size();
         if(FindMuc(jidList.first(),jidList.last(), i)) {
             MucUser &mu = mucUsers_[i];
             QDomNodeList nodeList = stanza.elementsByTagName("x");
             for(int i = nodeList.size(); i> 0;) {
                 QDomNode node = nodeList.at(--i).firstChild();
                 while(!node.isNull()) {
                     QDomElement item = node.toElement();
                     if(item.tagName() == "item") {
                         mu.affiliation = item.attribute("affiliation");
                         mu.role = item.attribute("role");
                         mu.jid = item.attribute("jid");
                         break;
                     }
                     node = node.nextSibling();
                 }
             }
         } else {
             MucUser mu;
             mu.mucJid = jidList.first();
             mu.nick = jidList.last();
             QDomNodeList nodeList = stanza.elementsByTagName("x");
             for(int i = nodeList.size(); i> 0;) {
                 QDomNode node = nodeList.at(--i).firstChild();
                 while(!node.isNull()) {
                     QDomElement item = node.toElement();
                     if(item.tagName() == "item") {
                         mu.affiliation = item.attribute("affiliation");
                         mu.role = item.attribute("role");
                         mu.jid = item.attribute("jid");
                         break;
                     }
                     node = node.nextSibling();
                 }
             }
             mucUsers_ << mu;
         }
     }
     else if(stanza.tagName() == "message" && stanza.attribute("type") == "chat") {
         QDomElement subj = stanza.firstChildElement("subject");
         if (subj.text() == "StopSpam" || subj.text() == "StopSpam Question") return false;

	 QString valF = stanza.attribute("from");
	 if (contactInfo->isConference(account, valF)) return false;

         MucUser mu;
	 QStringList jidList = valF.split("/");
         int i =  mucUsers_.size();
         if(FindMuc(jidList.first(),jidList.last(), i)) {
             mu = mucUsers_[i];
         } else {
             mu.affiliation = "";
             mu.jid = "";
             mu.mucJid = "";
             mu.nick = "";
             mu.role = "";
         }

         bool find = false;

         if(mu.affiliation == "owner" && !Owner)
             find = true;
         else if( mu.affiliation == "admin" && !Admin)
             find = true;
         else if(mu.affiliation == "none" && !None)
             find = true;
         else if(mu.affiliation == "member" && !Member)
             find = true;
         if(find)
             return false;

         if(mu.role == "moderator" && !Moder)
             find = true;
         else if(mu.role == "participant" && !Participant)
             find = true;
         else if(mu.role == "visitor" && !Visitor)
             find = true;
         if(find)
             return false;

         QStringList UnblockedJids = Unblocked.split("\n");
         if(UnblockedJids.contains(valF, Qt::CaseInsensitive)) return false;

         for(int i = 0; i < Jids.size(); i++) {
             QString jid_ = Jids.at(i);
             if(jid_.isEmpty())
                     continue;
             if(mu.jid.contains(jid_, Qt::CaseInsensitive)
                    || mu.nick.contains(jid_, Qt::CaseInsensitive)
                    || mu.mucJid.contains(jid_, Qt::CaseInsensitive)) {
                     if(!selected[i].toBool())
                         return false;
                     break;
                 }
          }

         QDomElement Body = stanza.firstChildElement("body");
         if(Body.isNull()) return false;

         if(BlockAll) {
             updateCounter(stanza, false);

             if(EnableBlockAllMes)
                 StanzaHost->sendMessage(account, valF,  Qt::escape(BlockAllMes), "StopSpam", "chat");

             return true;
         }

                     QString BodyText = Body.text();
                     if(BodyText == Answer) {
			  Unblocked += valF + "\n";
			  QVariant vUnblocked(Unblocked);
			  psiOptions->setPluginOption(constUnblocked, vUnblocked);
			  psiOptions->setPluginOption(constLastUnblock, QVariant(QDate::currentDate().toString("yyyyMMdd")));
			  StanzaHost->sendMessage(account, valF,  Qt::escape(Congratulation), "StopSpam", "chat");
			  updateCounter(stanza, true);
			  return true;
                     } else {
                         int i = BlockedJids.size();
                         if(FindAcc(account, valF, i)) {
                             Blocked &B = BlockedJids[i];
                             if(B.count < Times) {
                                 StanzaHost->sendMessage(account, valF,  Qt::escape(Question), "StopSpam Question", "chat");
                                 updateCounter(stanza, false);
                                 B.count++;
                                 B.LastMes = QDateTime::currentDateTime();
                                 return true;
                             } else {
                                 if(QDateTime::currentDateTime().secsTo(B.LastMes) >= -ResetTime*60) {
                                     updateCounter(stanza, false);
                                     return true;
                                 } else {
                                     B.count = 1;
                                     B.LastMes = QDateTime::currentDateTime();
                                     StanzaHost->sendMessage(account, valF,  Qt::escape(Question), "StopSpam Question", "chat");
                                     updateCounter(stanza, false);
                                     return true;
                                 }
                             }
                         }
                         else {
                             Blocked B = { account, valF, 1, QDateTime::currentDateTime() };
                             BlockedJids << B;
                             StanzaHost->sendMessage(account, valF,  Qt::escape(Question), "StopSpam Question", "chat");
                             updateCounter(stanza, false);
                             return true;
                         }
                    }
     }

     return false;
 }

 void StopSpam::changeWidgetsState() {
     affBox->setEnabled(UseMucWidget->isChecked());
     roleBox->setEnabled(UseMucWidget->isChecked());
     BlockAllWidget->setEnabled(UseMucWidget->isChecked());
     EnableBlockAllMesWidget->setEnabled(UseMucWidget->isChecked() && BlockAllWidget->isChecked());
     BlockAllMesWidget->setEnabled(UseMucWidget->isChecked() && BlockAllWidget->isChecked() && EnableBlockAllMesWidget->isChecked());
 }

 void StopSpam::addRow() {
         model_->addRow();
         hack();
 }

 void StopSpam::removeRow() {
     if(model_->rowCount() > 1) {
         model_->deleteRow();
         hack();
     }
 }

 void StopSpam::hack() {
     AdminWidget->toggle();
     AdminWidget->toggle();
 }

 bool StopSpam::FindMuc(QString mucJid, QString nick, int &i) {
     for(; i > 0;) {
         MucUser mu =  mucUsers_[--i];
         if(mu.mucJid == mucJid && mu.nick == nick) {
             return true;
         }
     }
     return false;
 }

 void StopSpam::onOptionsClose() {
    model_->reset();
}

 QString StopSpam::pluginInfo() {
	 return tr("Author: ") +  "Dealer_WeARE\n"
		+ tr("Email: ") + "wadealer@gmail.com\n\n"
		+ trUtf8("This plugin is designed to block spam messages and other unwanted information from Psi+ users."
		"The functionality of the plugin is based on the principle of \"question - answer\".\n"
		"With the plugin settings you can:\n"
		"* Define a security question and the answer\n"
		"* Define the set of rules that define whether to the trigger plugin for a contact\n"
		"* Define the text messages sent in the case of the correct answer\n"
		"* Enable notification through popups\n"
		"* Enable the saving of blocked messages in the history of the contact\n"
		"* Define the number of subject parcels\n"
		"* Set the time interval after which to reset the number of how many questions will be sent\n"
		"* Enable blocking of private messages in conferences\n"
		"* Choose for which ranks and roles of conference participants blocking messages will be disabled\n"
		"* Enable deadlocks in private messages to participants who do not fall into the exceptions list for the roles and ranks which include blocking.\n\n"

		"The rules are checked from top to bottom. If the rule is Enabled - stopspam is triggered, otherwise - stopspam is not triggered."
		" In the case where none of the rules triggered stopspam for roster messages, you can specify whether the plugin will activate or not."
		" For private messages from the same conference, it will always work.\n"
		"Question and answer as well as a list of rules is common for ordinary messages and for private messages in conferences.\n"
		"When a user has passed, the test will send a re-authorization request. It should be noted in the messages that are sent back"
		" the security question was correctly answered.\n"
		"The plugin keeps a log of blocked messages, which you can view through the plugin settings. The \"Reset\" button deletes the log"
		" and resets the counter of blocked messages.\n\n"
		"WARNING!!! Before registering a new transport, it is recommended to add its jid to transport exceptions. This is due to the fact"
		" that after the transport registration, authorization requests for all contacts will be sent and if the transport was not added to"
		" as an exception, the plugin will block all the requests.");
 }

#include "stopspamplugin.moc"
