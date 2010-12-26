/*
 * extendedoptionsplugin.cpp - plugin
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

#include "psiplugin.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "applicationinfoaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "plugininfoprovider.h"


#define constVersion "0.3.1"

class ExtendedOptions : public QObject, public PsiPlugin, public OptionAccessor, public ApplicationInfoAccessor, public PluginInfoProvider
{
        Q_OBJECT
	Q_INTERFACES(PsiPlugin OptionAccessor ApplicationInfoAccessor PluginInfoProvider)
public:
        ExtendedOptions();
        virtual QString name() const;
        virtual QString shortName() const;
        virtual QString version() const;
        virtual QWidget* options();
        virtual bool enable();
        virtual bool disable();
        virtual void optionChanged(const QString& option);
        virtual void applyOptions();
        virtual void restoreOptions();
        virtual void setOptionAccessingHost(OptionAccessingHost* host);
        virtual void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host);
	virtual QString pluginInfo();

private slots:
        void chooseColor(QAbstractButton*);

private:
        OptionAccessingHost *psiOptions;
        ApplicationInfoAccessingHost* appInfo;
	bool enabled;
        QString readFile();
        void saveFile(QString text);
        QString profileDir();
	QPointer<QWidget> options_;

        //Chats-----
        QCheckBox *htmlRender;
        QCheckBox *centralToolbar;
        QCheckBox *confirmClearing;
        QCheckBox *messageIcons;
        //QCheckBox *altnSwitch;
        QCheckBox *disableScroll;
        QCheckBox *bottomTabs;
        QCheckBox *closeButton;
        QComboBox *middleButton;
        QCheckBox *showAvatar;
        QSpinBox *avatarSize;
        QCheckBox *disablePastSend;
        QCheckBox *sayMode;
        QCheckBox *showTabIcons;
	QCheckBox *disableSend;
	QCheckBox *hideWhenClose;

        //MUC-----
        QCheckBox *showJoins;
        QCheckBox *showRole;
        QCheckBox *showStatus;
        QCheckBox *leftMucRoster;
        QCheckBox *showGroups;
        QCheckBox *showAffIcons;
        QCheckBox *skipAutojoin;
        QTextEdit *bookmarksListSkip;
        QCheckBox *mucClientIcons;
	QCheckBox *rosterNickColors;
	QCheckBox *mucHtml;
	QCheckBox *hideAutoJoin;

        //Roster
        QCheckBox *resolveNicks;
        QCheckBox *lockRoster;
        QCheckBox *leftRoster;
        QCheckBox *singleLineStatus;
        QCheckBox *avatarTip;
        QCheckBox *statusTip;
        QCheckBox *geoTip;
        QCheckBox *pgpTip;
        QCheckBox *clientTip;
        QComboBox *sortContacts;
       // QComboBox *sortGroups;
       // QComboBox *sortAccs;
        QCheckBox *leftAvatars;
        QCheckBox *defaultAvatar;
        QCheckBox *showStatusIcons;
        QCheckBox *statusIconsOverAvatars;


        //Menu------
        QCheckBox *admin;
        QCheckBox *activeChats;
        QCheckBox *pgpKey;
        QCheckBox *picture;
        QCheckBox *changeProfile;
        QCheckBox *chat;
        QCheckBox *invis;
        QCheckBox *xa;

        //Popup----
        QSpinBox *jidLenght;
        QSpinBox *statusLenght;
        QSpinBox *textLenght;
        QSpinBox *chatDur;
        QSpinBox *file;
        QSpinBox *message;
        QSpinBox *status;
	QCheckBox *topToBottom;
	QCheckBox *atLeftCorner;
	QCheckBox *everyMucMessage;

        //Look-----
        QToolButton *popupBorder;
        QToolButton *linkColor;
        QToolButton *mailtoColor;
        QToolButton *moderColor;
        QToolButton *visitorColor;
        QToolButton *parcColor;
        QToolButton *noroleColor;
        QToolButton *tipText;
        QToolButton *tipBase;
	QToolButton *composingBut;
	QToolButton *unreadBut;

        //CSS----------------
        QTextEdit *chatCss;
        QTextEdit *rosterCss;
        QTextEdit *popupCss;
        QTextEdit *tooltipCss;
        
};

Q_EXPORT_PLUGIN(ExtendedOptions);

ExtendedOptions::ExtendedOptions() {
    psiOptions = 0;
    appInfo = 0;
    enabled = 0;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    //Chats-----
    htmlRender = 0;
    centralToolbar = 0;
    confirmClearing = 0;
    messageIcons = 0;
   // altnSwitch = 0;
    disableScroll = 0;
    bottomTabs = 0;
    closeButton = 0;
    middleButton = 0;
    showAvatar = 0;
    avatarSize = 0;
    disablePastSend = 0;
    sayMode = 0;
    showTabIcons = 0;
    disableSend = 0;
    hideWhenClose = 0;

    //MUC-----
    showJoins = 0;
    showRole = 0;
    showStatus = 0;
    leftMucRoster = 0;
    showGroups = 0;
    showAffIcons = 0;
    skipAutojoin = 0;
    bookmarksListSkip = 0;
    mucClientIcons = 0;
    rosterNickColors = 0;
    mucHtml = 0;
    hideAutoJoin = 0;

    //Roster
    resolveNicks = 0;
    lockRoster = 0;
    sortContacts = 0;
    leftRoster = 0;
    singleLineStatus = 0;
    avatarTip = 0;
    statusTip = 0;
    geoTip = 0;
    pgpTip = 0;
    clientTip = 0;
    sortContacts = 0;
   // sortGroups = 0;
   // sortAccs = 0;
    leftAvatars = 0;
    defaultAvatar = 0;
    showStatusIcons = 0;
    statusIconsOverAvatars = 0;


    //Menu------
    admin = 0;
    activeChats = 0;
    pgpKey = 0;
    picture = 0;
    changeProfile = 0;
    chat = 0;
    invis = 0;
    xa = 0;

    //Popup----
    jidLenght = 0;
    statusLenght = 0;
    textLenght = 0;
    chatDur = 0;
    file = 0;
    message = 0;
    status = 0;
    topToBottom = 0;
    atLeftCorner = 0;
    everyMucMessage = 0;

    //Look----
    popupBorder = 0;
    linkColor = 0;
    mailtoColor = 0;
    moderColor = 0;
    parcColor = 0;
    visitorColor = 0;
    noroleColor = 0;
    tipText = 0;
    tipBase = 0;
    composingBut = 0;
    unreadBut = 0;

    //CSS----------------
    chatCss = 0;
    rosterCss = 0;
    popupCss = 0;
    tooltipCss = 0;

}

QString ExtendedOptions::name() const {
        return "Extended Options Plugin";
}

QString ExtendedOptions::shortName() const {
        return "extopt";
}

QString ExtendedOptions::version() const {
        return constVersion;
}

bool ExtendedOptions::enable(){
    if(psiOptions) {
        enabled = true;   
    }
    return enabled;
}

bool ExtendedOptions::disable(){
	enabled = false;
        return true;
}

QWidget* ExtendedOptions::options(){
        if (!enabled) {
		return 0;
	}
	options_ = new QWidget();
	QVBoxLayout *mainLayout = new QVBoxLayout(options_);
        QTabWidget *tabs = new QTabWidget;
        QWidget *tab1 = new QWidget;
        QWidget *tab2 = new QWidget;
        QWidget *tab3 = new QWidget;
        QWidget *tab4 = new QWidget;
        QWidget *tab5 = new QWidget;
        QWidget *tab6 = new QWidget;
        QWidget *tab7 = new QWidget;
        QVBoxLayout *tab1Layout = new QVBoxLayout(tab1);
        QVBoxLayout *tab2Layout = new QVBoxLayout(tab2);
        QVBoxLayout *tab3Layout = new QVBoxLayout(tab3);
        QVBoxLayout *tab4Layout = new QVBoxLayout(tab4);
        QVBoxLayout *tab5Layout = new QVBoxLayout(tab5);
        QVBoxLayout *tab6Layout = new QVBoxLayout(tab6);
        QVBoxLayout *tab7Layout = new QVBoxLayout(tab7);
        tabs->addTab(tab1, tr("Chat"));
        tabs->addTab(tab2, tr("Conference"));
        tabs->addTab(tab3, tr("Roster"));
        tabs->addTab(tab4, tr("Menu"));
        tabs->addTab(tab5, tr("Popups"));
        tabs->addTab(tab6, tr("Look"));
        tabs->addTab(tab7, tr("CSS"));

        //Chats-----
        htmlRender = new QCheckBox(tr("Enable HTML rendering in chat window"));
        centralToolbar = new QCheckBox(tr("Enable central toolbar"));
        confirmClearing = new QCheckBox(tr("Ask for confirmation before clearing chat window"));
        messageIcons = new QCheckBox(tr("Enable icons in chat"));

       /* altnSwitch = new QCheckBox(tr("Switch tabs with \"ALT+(1-9)\""));
        altnSwitch->setChecked(psiOptions->getGlobalOption("options.ui.tabs.alt-n-switch").toBool());*/

        disableScroll = new QCheckBox(tr("Disable wheel scroll"));
        bottomTabs = new QCheckBox(tr("Put tabs at bottom of chat window"));
        closeButton = new QCheckBox(tr("Show Close Button on tabs"));
        showAvatar = new QCheckBox(tr("Show Avatar"));
        disablePastSend = new QCheckBox(tr("Disable \"Paste and Send\" button"));
        sayMode = new QCheckBox(tr("Enable \"Says style\""));
        showTabIcons = new QCheckBox(tr("Show status icons on tabs"));
	disableSend = new QCheckBox(tr("Hide \"Send\" button"));
	hideWhenClose = new QCheckBox(tr("Hide tab when close"));

	avatarSize = new QSpinBox;
        QHBoxLayout *asLayout = new QHBoxLayout;
        asLayout->addWidget(new QLabel(tr("Avatar size:")));
        asLayout->addWidget(avatarSize);
        asLayout->addStretch();

        middleButton = new QComboBox;
        QHBoxLayout *mbLayout = new QHBoxLayout;
        middleButton->addItem("hide");
        middleButton->addItem("close");
        mbLayout->addWidget(new QLabel(tr("Action for mouse middle click on tabs:")));
        mbLayout->addWidget(middleButton);
        mbLayout->addStretch();

        tab1Layout->addWidget(htmlRender);
        tab1Layout->addWidget(centralToolbar);
        tab1Layout->addWidget(confirmClearing);
        tab1Layout->addWidget(messageIcons);
        //tab1Layout->addWidget(altnSwitch);
        tab1Layout->addWidget(disableScroll);
        tab1Layout->addWidget(bottomTabs);
        tab1Layout->addWidget(closeButton);
        tab1Layout->addWidget(showTabIcons);
        tab1Layout->addWidget(disablePastSend);
	tab1Layout->addWidget(disableSend);
        tab1Layout->addWidget(sayMode);
        tab1Layout->addWidget(showAvatar);
	tab1Layout->addWidget(hideWhenClose);
        tab1Layout->addLayout(asLayout);
        tab1Layout->addLayout(mbLayout);
        tab1Layout->addStretch();

        //MUC-----
        showJoins = new QCheckBox(tr("Show joins"));
        showRole = new QCheckBox(tr("Show role affiliation"));
        showStatus = new QCheckBox(tr("Show status changes"));
        leftMucRoster = new QCheckBox(tr("Place MUC roster at left"));
        showGroups = new QCheckBox(tr("Show groups in MUC roster"));
        showAffIcons = new QCheckBox(tr("Show affiliation icons in MUC roster"));
        mucClientIcons = new QCheckBox(tr("Show client icons in MUC roster"));
	rosterNickColors = new QCheckBox(tr("Enable nick coloring in roster"));
        skipAutojoin = new QCheckBox(tr("Enable autojoin for bookmarked conferences"));
	hideAutoJoin = new QCheckBox(tr("Hide auto-join conferences"));
	mucHtml = new QCheckBox(tr("Enable HTML rendering in MUC chat window"));

        bookmarksListSkip = new QTextEdit();
        bookmarksListSkip->setMaximumWidth(300);
        bookmarksListSkip->setPlainText(readFile());

	tab2Layout->addWidget(mucHtml);
        tab2Layout->addWidget(showJoins);
        tab2Layout->addWidget(showRole);
        tab2Layout->addWidget(showStatus);
        tab2Layout->addWidget(leftMucRoster);
        tab2Layout->addWidget(showGroups);
        tab2Layout->addWidget(showAffIcons);
        tab2Layout->addWidget(mucClientIcons);
	tab2Layout->addWidget(rosterNickColors);
        tab2Layout->addWidget(skipAutojoin);
	tab2Layout->addWidget(hideAutoJoin);
        tab2Layout->addWidget(new QLabel(tr("Disable autojoin to folowing conferences:\n(specify JIDs)")));
        tab2Layout->addWidget(bookmarksListSkip);
        tab2Layout->addStretch();

        //Roster
        resolveNicks = new QCheckBox(tr("Resolve nicks on contact add"));
        lockRoster = new QCheckBox(tr("Lockdown roster"));
        leftRoster = new QCheckBox(tr("Place roster at left in \"all-in-one-window\" mode"));
        singleLineStatus = new QCheckBox(tr("Contact name and status message in a row"));
        leftAvatars = new QCheckBox(tr("Place avatars at left"));
        defaultAvatar = new QCheckBox(tr("If contact does not have avatar, use default avatar"));
        showStatusIcons = new QCheckBox(tr("Show status icons"));
        statusIconsOverAvatars = new QCheckBox(tr("Place status icon over avatar"));

        QGroupBox *groupBox = new QGroupBox(tr("Tooltips:"));
        QVBoxLayout *boxLayout = new QVBoxLayout(groupBox);

        avatarTip = new QCheckBox(tr("Show avatar"));
        statusTip = new QCheckBox(tr("Show last status"));
        pgpTip = new QCheckBox(tr("Show PGP"));
        clientTip = new QCheckBox(tr("Show client version"));
        geoTip = new QCheckBox(tr("Show geolocation"));

        boxLayout->addWidget(avatarTip);
        boxLayout->addWidget(statusTip);
        boxLayout->addWidget(pgpTip);
        boxLayout->addWidget(clientTip);
        boxLayout->addWidget(geoTip);

        sortContacts = new QComboBox;
        sortContacts->addItem("alpha");
        sortContacts->addItem("status");

	QHBoxLayout *sortLayout = new QHBoxLayout();
	sortLayout->addWidget(new QLabel(tr("Sort style for contacts:")));
	sortLayout->addWidget(sortContacts);
	sortLayout->addStretch();

        tab3Layout->addWidget(resolveNicks);
        tab3Layout->addWidget(lockRoster);
        tab3Layout->addWidget(leftRoster);
        tab3Layout->addWidget(singleLineStatus);
        tab3Layout->addWidget(showStatusIcons);
        tab3Layout->addWidget(statusIconsOverAvatars);
        tab3Layout->addWidget(leftAvatars);
        tab3Layout->addWidget(defaultAvatar);
	tab3Layout->addLayout(sortLayout);
	tab3Layout->addWidget(groupBox);
	tab3Layout->addStretch();


        //Menu------
        admin = new QCheckBox(tr("Show \"Admin\" option in account menu"));
        activeChats = new QCheckBox(tr("Show \"Active Chats\" option in contact menu"));
        pgpKey = new QCheckBox(tr("Show \"Assign OpenPGP Key\" option in contact menu"));
        picture = new QCheckBox(tr("Show \"Picture\" option in contact menu"));
        changeProfile = new QCheckBox(tr("Show \"Change Profile\" option in main menu"));
        chat = new QCheckBox(tr("Show \"Chat\" option in status menu"));
        invis = new QCheckBox(tr("Show \"Invisible\" option in status menu"));
        xa = new QCheckBox(tr("Show \"XA\" option in status menu"));

        tab4Layout->addWidget(admin);
        tab4Layout->addWidget(activeChats);
        tab4Layout->addWidget(pgpKey);
        tab4Layout->addWidget(picture);
        tab4Layout->addWidget(changeProfile);
        tab4Layout->addWidget(chat);
        tab4Layout->addWidget(invis);
        tab4Layout->addWidget(xa);
        tab4Layout->addStretch();


        //Popup----
	topToBottom = new QCheckBox(tr("Show popups from top to bottom"));
	atLeftCorner = new QCheckBox(tr("Place popups at left"));
	everyMucMessage = new QCheckBox(tr("Enable popup notifications for every muc message"));

        jidLenght = new QSpinBox;
        jidLenght->setMinimum(-1);
        jidLenght->setMaximum(9999);

        QHBoxLayout *jlLayout = new QHBoxLayout;
        jlLayout->addWidget(new QLabel(tr("Maximum JID length:")));
        jlLayout->addStretch();
        jlLayout->addWidget(jidLenght);

        statusLenght = new QSpinBox;
        statusLenght->setMinimum(-1);
        statusLenght->setMaximum(9999);

        QHBoxLayout *slLayout = new QHBoxLayout;
        slLayout->addWidget(new QLabel(tr("Maximum status length:")));
        slLayout->addStretch();
        slLayout->addWidget(statusLenght);

        textLenght = new QSpinBox;
        textLenght->setMinimum(-1);
        textLenght->setMaximum(9999);

        QHBoxLayout *tlLayout = new QHBoxLayout;
        tlLayout->addWidget(new QLabel(tr("Maximum text length:")));
        tlLayout->addStretch();
        tlLayout->addWidget(textLenght);

        QGroupBox *group2Box = new QGroupBox(tr("Delays (sec):"));
        QVBoxLayout *box2Layout = new QVBoxLayout(group2Box);

        chatDur = new QSpinBox;
        chatDur->setMinimum(-1);

        QHBoxLayout *cLayout = new QHBoxLayout;
        cLayout->addWidget(new QLabel(tr("Chat:")));
        cLayout->addStretch();
        cLayout->addWidget(chatDur);

        file = new QSpinBox;
        file->setMinimum(-1);

        QHBoxLayout *fLayout = new QHBoxLayout;
        fLayout->addWidget(new QLabel(tr("File:")));
        fLayout->addStretch();
        fLayout->addWidget(file);

        message = new QSpinBox;
        message->setMinimum(-1);

        QHBoxLayout *mLayout = new QHBoxLayout;
        mLayout->addWidget(new QLabel(tr("Message:")));
        mLayout->addStretch();
        mLayout->addWidget(message);

        status = new QSpinBox;
        status->setMinimum(-1);

        QHBoxLayout *sLayout = new QHBoxLayout;
        sLayout->addWidget(new QLabel(tr("Status:")));
        sLayout->addStretch();
        sLayout->addWidget(status);

        box2Layout->addLayout(cLayout);
        box2Layout->addLayout(fLayout);
        box2Layout->addLayout(mLayout);
        box2Layout->addLayout(sLayout);
        box2Layout->addStretch();

	tab5Layout->addWidget(topToBottom);
	tab5Layout->addWidget(atLeftCorner);
	tab5Layout->addWidget(everyMucMessage);
        tab5Layout->addLayout(jlLayout);
        tab5Layout->addLayout(slLayout);
        tab5Layout->addLayout(tlLayout);
        tab5Layout->addWidget(group2Box);
        tab5Layout->addStretch();


        //Look----
        popupBorder = new QToolButton;
        QHBoxLayout *pbLayout = new QHBoxLayout;
        pbLayout->addWidget(new QLabel(tr("Popup border color:")));
        pbLayout->addStretch();
        pbLayout->addWidget(popupBorder);

        linkColor = new QToolButton;
        QHBoxLayout *lcLayout = new QHBoxLayout;
        lcLayout->addWidget(new QLabel(tr("Link color:")));
        lcLayout->addStretch();
        lcLayout->addWidget(linkColor);

        mailtoColor = new QToolButton;
        QHBoxLayout *mcLayout = new QHBoxLayout;
        mcLayout->addWidget(new QLabel(tr("Mailto color:")));
        mcLayout->addStretch();
        mcLayout->addWidget(mailtoColor);

        moderColor = new QToolButton;
        QHBoxLayout *modcLayout = new QHBoxLayout;
        modcLayout->addWidget(new QLabel(tr("Moderators color:")));
        modcLayout->addStretch();
        modcLayout->addWidget(moderColor);

        parcColor = new QToolButton;
        QHBoxLayout *parcLayout = new QHBoxLayout;
        parcLayout->addWidget(new QLabel(tr("Participants color:")));
        parcLayout->addStretch();
        parcLayout->addWidget(parcColor);

        visitorColor  = new QToolButton;
        QHBoxLayout *vscLayout = new QHBoxLayout;
        vscLayout->addWidget(new QLabel(tr("Visitors color:")));
        vscLayout->addStretch();
        vscLayout->addWidget(visitorColor);

        noroleColor  = new QToolButton;
        QHBoxLayout *nrcLayout = new QHBoxLayout;
        nrcLayout->addWidget(new QLabel(tr("No Role color:")));
        nrcLayout->addStretch();
        nrcLayout->addWidget(noroleColor);

        tipText = new QToolButton;
        QHBoxLayout *ttLayout = new QHBoxLayout;
        ttLayout->addWidget(new QLabel(tr("ToolTip text color:")));
        ttLayout->addStretch();
        ttLayout->addWidget(tipText);

        tipBase = new QToolButton;
        QHBoxLayout *tbLayout = new QHBoxLayout;
        tbLayout->addWidget(new QLabel(tr("ToolTip background color:")));
        tbLayout->addStretch();
        tbLayout->addWidget(tipBase);

	composingBut = new QToolButton;
	QHBoxLayout *composingLayout = new QHBoxLayout;
	composingLayout->addWidget(new QLabel(tr("Text color for \"composing\" events on tabs:")));
	composingLayout->addStretch();
	composingLayout->addWidget(composingBut);

	unreadBut = new QToolButton;
	QHBoxLayout *unreadLayout = new QHBoxLayout;
	unreadLayout->addWidget(new QLabel(tr("Text color for \"unread\" events on tabs:")));
	unreadLayout->addStretch();
	unreadLayout->addWidget(unreadBut);

        QButtonGroup *b_color = new QButtonGroup;
        b_color->addButton(popupBorder);
        b_color->addButton(linkColor);
        b_color->addButton(mailtoColor);
        b_color->addButton(moderColor);
        b_color->addButton(parcColor);
        b_color->addButton(visitorColor);
        b_color->addButton(noroleColor);
        b_color->addButton(tipText);
        b_color->addButton(tipBase);
	b_color->addButton(composingBut);
	b_color->addButton(unreadBut);
        connect(b_color, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(chooseColor(QAbstractButton*)));

        QGroupBox *group3Box = new QGroupBox(tr("Colors:"));
        QVBoxLayout *box3Layout = new QVBoxLayout(group3Box);
        box3Layout->addLayout(pbLayout);
        box3Layout->addLayout(lcLayout);
        box3Layout->addLayout(mcLayout);
        box3Layout->addLayout(modcLayout);
        box3Layout->addLayout(parcLayout);
        box3Layout->addLayout(vscLayout);
        box3Layout->addLayout(nrcLayout);
        box3Layout->addLayout(ttLayout);
        box3Layout->addLayout(tbLayout);
	box3Layout->addLayout(composingLayout);
	box3Layout->addLayout(unreadLayout);
        box3Layout->addStretch();

        tab6Layout->addWidget(group3Box);

        //CSS----------------
        QTabWidget *cssTab = new QTabWidget;

        chatCss = new QTextEdit;
        chatCss->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        rosterCss = new QTextEdit;
        rosterCss->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        popupCss = new QTextEdit;
        popupCss->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        tooltipCss = new QTextEdit;
        tooltipCss->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        cssTab->addTab(chatCss, tr("Chat"));
        cssTab->addTab(rosterCss, tr("Roster"));
        cssTab->addTab(popupCss, tr("Popup"));
        cssTab->addTab(tooltipCss, tr("Tooltip"));

	QLabel *cssLabel = new QLabel(tr("<a href=\"http://psi-plus.com/wiki/skins_css\">CSS for Psi+</a>"));
	cssLabel->setOpenExternalLinks(true);

        tab7Layout->addWidget(cssTab);
	tab7Layout->addWidget(cssLabel);


        QLabel *wikiLink = new QLabel(tr("<a href=\"http://psi-plus.com/wiki/plugins#extended_options_plugin\">Wiki (Online)</a>"));
	wikiLink->setOpenExternalLinks(true);

        mainLayout->addWidget(tabs);
        mainLayout->addWidget(wikiLink);

	restoreOptions();

	return options_;
}

void ExtendedOptions::applyOptions() {
    if(!options_)
	    return;

     //Chats-----
    psiOptions->setGlobalOption("options.html.chat.render",QVariant(htmlRender->isChecked()));
    psiOptions->setGlobalOption("options.ui.chat.central-toolbar",QVariant(centralToolbar->isChecked()));
    psiOptions->setGlobalOption("options.ui.chat.warn-before-clear",QVariant(confirmClearing->isChecked()));
    psiOptions->setGlobalOption("options.ui.chat.use-message-icons",QVariant(messageIcons->isChecked()));
    //psiOptions->setGlobalOption("options.ui.tabs.alt-n-switch",QVariant(altnSwitch->isChecked()));
    psiOptions->setGlobalOption("options.ui.tabs.disable-wheel-scroll",QVariant(disableScroll->isChecked()));
    psiOptions->setGlobalOption("options.ui.tabs.put-tabs-at-bottom",QVariant(bottomTabs->isChecked()));
    psiOptions->setGlobalOption("options.ui.tabs.show-tab-close-buttons",QVariant(closeButton->isChecked()));
    psiOptions->setGlobalOption("options.ui.tabs.mouse-middle-button",QVariant(middleButton->currentText()));
    psiOptions->setGlobalOption("options.ui.chat.avatars.show",QVariant(showAvatar->isChecked()));
    psiOptions->setGlobalOption("options.ui.chat.disable-paste-send",QVariant(disablePastSend->isChecked()));
    psiOptions->setGlobalOption("options.ui.chat.use-chat-says-style",QVariant(sayMode->isChecked()));
    psiOptions->setGlobalOption("options.ui.chat.avatars.size", QVariant(avatarSize->value()));
    psiOptions->setGlobalOption("options.ui.tabs.show-tab-icons",QVariant(showTabIcons->isChecked()));
    psiOptions->setGlobalOption("options.ui.disable-send-button",QVariant(disableSend->isChecked()));
    psiOptions->setGlobalOption("options.ui.chat.hide-when-closing",QVariant(hideWhenClose->isChecked()));

     //MUC-----
    psiOptions->setGlobalOption("options.muc.show-joins",QVariant(showJoins->isChecked()));
    psiOptions->setGlobalOption("options.muc.show-role-affiliation",QVariant(showRole->isChecked()));
    psiOptions->setGlobalOption("options.muc.show-status-changes",QVariant(showStatus->isChecked()));
    psiOptions->setGlobalOption("options.ui.muc.roster-at-left",QVariant(leftMucRoster->isChecked()));
    psiOptions->setGlobalOption("options.ui.muc.show-groups",QVariant(showGroups->isChecked()));
    psiOptions->setGlobalOption("options.ui.muc.show-affiliation-icons",QVariant(showAffIcons->isChecked()));
    psiOptions->setGlobalOption("options.ui.muc.show-client-icons",QVariant(mucClientIcons->isChecked()));
    psiOptions->setGlobalOption("options.muc.bookmarks.auto-join",QVariant(skipAutojoin->isChecked()));
    psiOptions->setGlobalOption("options.ui.muc.roster-nick-coloring",QVariant(rosterNickColors->isChecked()));
    psiOptions->setGlobalOption("options.html.muc.render",QVariant(mucHtml->isChecked()));
    psiOptions->setGlobalOption("options.ui.muc.hide-on-autojoin",QVariant(hideAutoJoin->isChecked()));
    saveFile(bookmarksListSkip->toPlainText());    

    //Roster-----
    psiOptions->setGlobalOption("options.contactlist.resolve-nicks-on-contact-add",QVariant(resolveNicks->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.lockdown-roster",QVariant(lockRoster->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.roster-at-left-when-all-in-one-window",QVariant(leftRoster->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.status-messages.single-line",QVariant(singleLineStatus->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.tooltip.avatar",QVariant(avatarTip->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.tooltip.last-status",QVariant(statusTip->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.tooltip.pgp",QVariant(pgpTip->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.tooltip.geolocation",QVariant(geoTip->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.tooltip.client-version",QVariant(clientTip->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.contact-sort-style",QVariant(sortContacts->currentText()));
   // psiOptions->setGlobalOption("options.ui.contactlist.group-sort-style",QVariant(sortGroups->currentText()));
   // psiOptions->setGlobalOption("options.ui.contactlist.account-sort-style",QVariant(sortAccs->currentText()));
    psiOptions->setGlobalOption("options.ui.contactlist.avatars.avatars-at-left",QVariant(leftAvatars->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.avatars.use-default-avatar",QVariant(defaultAvatar->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.show-status-icons",QVariant(showStatusIcons->isChecked()));
    psiOptions->setGlobalOption("options.ui.contactlist.status-icon-over-avatar",QVariant(statusIconsOverAvatars->isChecked()));

    //Menu------
    psiOptions->setGlobalOption("options.ui.menu.account.admin",QVariant(admin->isChecked()));
    psiOptions->setGlobalOption("options.ui.menu.contact.active-chats",QVariant(activeChats->isChecked()));
    psiOptions->setGlobalOption("options.ui.menu.contact.custom-pgp-key",QVariant(pgpKey->isChecked()));
    psiOptions->setGlobalOption("options.ui.menu.contact.custom-picture",QVariant(picture->isChecked()));
    psiOptions->setGlobalOption("options.ui.menu.main.change-profile",QVariant(changeProfile->isChecked()));
    psiOptions->setGlobalOption("options.ui.menu.status.chat",QVariant(chat->isChecked()));
    psiOptions->setGlobalOption("options.ui.menu.status.invisible",QVariant(invis->isChecked()));
    psiOptions->setGlobalOption("options.ui.menu.status.xa",QVariant(xa->isChecked()));

    //Popup----
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.maximum-jid-length", QVariant(jidLenght->value()));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.maximum-status-length", QVariant(statusLenght->value()));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.maximum-text-length", QVariant(textLenght->value()));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.chat", QVariant((chatDur->value())*1000));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.file", QVariant((file->value())*1000));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.message", QVariant((message->value())*1000));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.delays.status", QVariant((status->value())*1000));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.top-to-bottom",QVariant(topToBottom->isChecked()));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.at-left-corner",QVariant(atLeftCorner->isChecked()));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.notify-every-muc-message",QVariant(everyMucMessage->isChecked()));

    //Look----
    psiOptions->setGlobalOption("options.ui.look.colors.passive-popup.border", QVariant(popupBorder->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.chat.link-color", QVariant(linkColor->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.chat.mailto-color", QVariant(mailtoColor->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.muc.role-moderator", QVariant(moderColor->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.muc.role-participant", QVariant(parcColor->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.muc.role-visitor", QVariant(visitorColor->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.muc.role-norole", QVariant(noroleColor->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.tooltip.text", QVariant(tipText->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.tooltip.background", QVariant(tipBase->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.chat.unread-message-color", QVariant(unreadBut->property("psi_color").value<QColor>()));
    psiOptions->setGlobalOption("options.ui.look.colors.chat.composing-color", QVariant(composingBut->property("psi_color").value<QColor>()));

    //CSS----------------
    psiOptions->setGlobalOption("options.ui.chat.css", QVariant(chatCss->toPlainText()));
    psiOptions->setGlobalOption("options.ui.contactlist.css", QVariant(rosterCss->toPlainText()));
    psiOptions->setGlobalOption("options.ui.notifications.passive-popups.css", QVariant(popupCss->toPlainText()));
    psiOptions->setGlobalOption("options.ui.contactlist.tooltip.css", QVariant(tooltipCss->toPlainText()));

}

void ExtendedOptions::restoreOptions() {
	if(!options_)
		return;

     //Chats-----
        htmlRender->setChecked(psiOptions->getGlobalOption("options.html.chat.render").toBool());
        centralToolbar->setChecked(psiOptions->getGlobalOption("options.ui.chat.central-toolbar").toBool());
        confirmClearing->setChecked(psiOptions->getGlobalOption("options.ui.chat.warn-before-clear").toBool());
        messageIcons->setChecked(psiOptions->getGlobalOption("options.ui.chat.use-message-icons").toBool());
       // altnSwitch->setChecked(psiOptions->getGlobalOption("options.ui.tabs.alt-n-switch").toBool());
        disableScroll->setChecked(psiOptions->getGlobalOption("options.ui.tabs.disable-wheel-scroll").toBool());
        bottomTabs->setChecked(psiOptions->getGlobalOption("options.ui.tabs.put-tabs-at-bottom").toBool());
        closeButton->setChecked(psiOptions->getGlobalOption("options.ui.tabs.show-tab-close-buttons").toBool());
        middleButton->setCurrentIndex(middleButton->findText(psiOptions->getGlobalOption("options.ui.tabs.mouse-middle-button").toString()));
        showAvatar->setChecked(psiOptions->getGlobalOption("options.ui.chat.avatars.show").toBool());
        avatarSize->setValue(psiOptions->getGlobalOption("options.ui.chat.avatars.size").toInt());
        disablePastSend->setChecked(psiOptions->getGlobalOption("options.ui.chat.disable-paste-send").toBool());
        sayMode->setChecked(psiOptions->getGlobalOption("options.ui.chat.use-chat-says-style").toBool());
        showTabIcons->setChecked(psiOptions->getGlobalOption("options.ui.tabs.show-tab-icons").toBool());
	disableSend->setChecked(psiOptions->getGlobalOption("options.ui.disable-send-button").toBool());
	hideWhenClose->setChecked(psiOptions->getGlobalOption("options.ui.chat.hide-when-closing").toBool());

        //MUC-----
        showJoins->setChecked(psiOptions->getGlobalOption("options.muc.show-joins").toBool());
        showRole->setChecked(psiOptions->getGlobalOption("options.muc.show-role-affiliation").toBool());
        showStatus->setChecked(psiOptions->getGlobalOption("options.muc.show-status-changes").toBool());
        leftMucRoster->setChecked(psiOptions->getGlobalOption("options.ui.muc.roster-at-left").toBool());
        showGroups->setChecked(psiOptions->getGlobalOption("options.ui.muc.show-groups").toBool());
        showAffIcons->setChecked(psiOptions->getGlobalOption("options.ui.muc.show-affiliation-icons").toBool());
        skipAutojoin->setChecked(psiOptions->getGlobalOption("options.muc.bookmarks.auto-join").toBool());
        bookmarksListSkip->setPlainText(readFile());
        mucClientIcons->setChecked(psiOptions->getGlobalOption("options.ui.muc.show-client-icons").toBool());
	rosterNickColors->setChecked(psiOptions->getGlobalOption("options.ui.muc.roster-nick-coloring").toBool());
	mucHtml->setChecked(psiOptions->getGlobalOption("options.html.muc.render").toBool());
	hideAutoJoin->setChecked(psiOptions->getGlobalOption("options.ui.muc.hide-on-autojoin").toBool());

        //Roster
        resolveNicks->setChecked(psiOptions->getGlobalOption("options.contactlist.resolve-nicks-on-contact-add").toBool());
        lockRoster->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.lockdown-roster").toBool());
        leftRoster->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.roster-at-left-when-all-in-one-window").toBool());
        singleLineStatus->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.status-messages.single-line").toBool());
        avatarTip->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.tooltip.avatar").toBool());
        statusTip->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.tooltip.last-status").toBool());
        geoTip->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.tooltip.geolocation").toBool());
        pgpTip->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.tooltip.pgp").toBool());
        clientTip->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.tooltip.client-version").toBool());
        sortContacts->setCurrentIndex(sortContacts->findText(psiOptions->getGlobalOption("options.ui.contactlist.contact-sort-style").toString()));
        leftAvatars->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.avatars.avatars-at-left").toBool());
        defaultAvatar->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.avatars.use-default-avatar").toBool());
        showStatusIcons->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.show-status-icons").toBool());
        statusIconsOverAvatars->setChecked(psiOptions->getGlobalOption("options.ui.contactlist.status-icon-over-avatar").toBool());

        //Menu------
        admin->setChecked(psiOptions->getGlobalOption("options.ui.menu.account.admin").toBool());
        activeChats->setChecked(psiOptions->getGlobalOption("options.ui.menu.contact.active-chats").toBool());
        pgpKey->setChecked(psiOptions->getGlobalOption("options.ui.menu.contact.custom-pgp-key").toBool());
        picture->setChecked(psiOptions->getGlobalOption("options.ui.menu.contact.custom-picture").toBool());
        changeProfile->setChecked(psiOptions->getGlobalOption("options.ui.menu.main.change-profile").toBool());
        chat->setChecked(psiOptions->getGlobalOption("options.ui.menu.status.chat").toBool());
        invis->setChecked(psiOptions->getGlobalOption("options.ui.menu.status.invisible").toBool());
        xa->setChecked(psiOptions->getGlobalOption("options.ui.menu.status.xa").toBool());

        //Popup----
        jidLenght->setValue(psiOptions->getGlobalOption("options.ui.notifications.passive-popups.maximum-jid-length").toInt());
        statusLenght->setValue(psiOptions->getGlobalOption("options.ui.notifications.passive-popups.maximum-status-length").toInt());
        textLenght->setValue(psiOptions->getGlobalOption("options.ui.notifications.passive-popups.maximum-text-length").toInt());
        chatDur->setValue((psiOptions->getGlobalOption("options.ui.notifications.passive-popups.delays.chat").toInt())/1000);
        file->setValue((psiOptions->getGlobalOption("options.ui.notifications.passive-popups.delays.file").toInt())/1000);
        message->setValue((psiOptions->getGlobalOption("options.ui.notifications.passive-popups.delays.message").toInt())/1000);
        status->setValue((psiOptions->getGlobalOption("options.ui.notifications.passive-popups.delays.status").toInt())/1000);
	topToBottom->setChecked(psiOptions->getGlobalOption("options.ui.notifications.passive-popups.top-to-bottom").toBool());
	atLeftCorner->setChecked(psiOptions->getGlobalOption("options.ui.notifications.passive-popups.at-left-corner").toBool());
	everyMucMessage->setChecked(psiOptions->getGlobalOption("options.ui.notifications.passive-popups.notify-every-muc-message").toBool());

        //Look----
        QColor color;
        color = psiOptions->getGlobalOption("options.ui.look.colors.passive-popup.border").toString();
        popupBorder->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        popupBorder->setProperty("psi_color", color);
        color = psiOptions->getGlobalOption("options.ui.look.colors.chat.link-color").toString();
        linkColor->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        linkColor->setProperty("psi_color", color);
        color = psiOptions->getGlobalOption("options.ui.look.colors.chat.mailto-color").toString();
        mailtoColor->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        mailtoColor->setProperty("psi_color", color);
        color = psiOptions->getGlobalOption("options.ui.look.colors.muc.role-moderator").toString();
        moderColor->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        moderColor->setProperty("psi_color", color);
        color = psiOptions->getGlobalOption("options.ui.look.colors.muc.role-participant").toString();
        parcColor->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        parcColor->setProperty("psi_color", color);
        color = psiOptions->getGlobalOption("options.ui.look.colors.muc.role-visitor").toString();
        visitorColor->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        visitorColor->setProperty("psi_color", color);
        color = psiOptions->getGlobalOption("options.ui.look.colors.muc.role-norole").toString();
        noroleColor->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        noroleColor->setProperty("psi_color", color);
        color = psiOptions->getGlobalOption("options.ui.look.colors.tooltip.text").toString();
        tipText->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        tipText->setProperty("psi_color", color);
        color = psiOptions->getGlobalOption("options.ui.look.colors.tooltip.background").toString();
        tipBase->setStyleSheet(QString("background-color: %1;").arg(color.name()));
	tipBase->setProperty("psi_color", color);
	color = psiOptions->getGlobalOption("options.ui.look.colors.chat.unread-message-color").toString();
	unreadBut->setStyleSheet(QString("background-color: %1;").arg(color.name()));
	unreadBut->setProperty("psi_color", color);
	color = psiOptions->getGlobalOption("options.ui.look.colors.chat.composing-color").toString();
	composingBut->setStyleSheet(QString("background-color: %1;").arg(color.name()));
	composingBut->setProperty("psi_color", color);

        //CSS----------------
        chatCss->setText(psiOptions->getGlobalOption("options.ui.chat.css").toString());
        rosterCss->setText(psiOptions->getGlobalOption("options.ui.contactlist.css").toString());
        popupCss->setText(psiOptions->getGlobalOption("options.ui.notifications.passive-popups.css").toString());
        tooltipCss->setText(psiOptions->getGlobalOption("options.ui.contactlist.tooltip.css").toString());


}


void ExtendedOptions::setOptionAccessingHost(OptionAccessingHost *host) {
    psiOptions = host;
}

void ExtendedOptions::optionChanged(const QString &option) {
    Q_UNUSED(option);
}

void ExtendedOptions::chooseColor(QAbstractButton* button) {
        QColor c;
        c = button->property("psi_color").value<QColor>();
        c = QColorDialog::getColor(c, new QWidget());
        if(c.isValid()) {
                button->setProperty("psi_color", c);
                button->setStyleSheet(QString("background-color: %1").arg(c.name()));
        }

        //Enable "Apply" button
        htmlRender->toggle();
        htmlRender->toggle();
}

void ExtendedOptions::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host) {
    appInfo = host;
}

QString ExtendedOptions::readFile() {
    QFile file(profileDir() + QDir::separator() + QString("mucskipautojoin.txt"));
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        return in.readAll();
    }
    return QString();
}

void ExtendedOptions::saveFile(QString text) {
    QFile file(profileDir() + QDir::separator() + QString("mucskipautojoin.txt"));
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream out(&file);
        out.setGenerateByteOrderMark(false);
        out << text << endl;
        file.close();
    }
}

QString ExtendedOptions::profileDir() {
    QString profileDir = appInfo->appHistoryDir();
    int index = profileDir.size() - profileDir.lastIndexOf("/");
    profileDir.chop(index);
    return profileDir;
}

QString ExtendedOptions::pluginInfo() {
	return tr("Author: ") +  "Dealer_WeARE\n"
			+ tr("Email: ") + "wadealer@gmail.com\n\n"
			+ trUtf8("This plugin is designed to allow easy configuration of some advanced options in Psi+.\n"
			 "This plugin gives you access to advanced application options, which do not have a graphical user interface.\n\n"
			 "Importantly: a large part of the options are important system settings. These require extra attention and proper"
			 "understanding of the results when changing the option.");
}

#include "extendedoptionsplugin.moc"

