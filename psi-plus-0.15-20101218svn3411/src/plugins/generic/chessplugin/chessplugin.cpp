/*
 * chessplugin.cpp - plugin
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

#include <QByteArray>
#include <QMessageBox>
#include <QSound>
#include <QFileDialog>

#include "psiplugin.h"
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
#include "menuaccessor.h"
#include "applicationinfoaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "stanzafilter.h"
#include "plugininfoprovider.h"
#include "eventcreatinghost.h"
#include "eventcreator.h"

#include "mainwindow.h"
#include "figure.h"
#include "ui_options.h"
#include "invitedialog.h"

#define cVer "0.1.9"

#define soundStartConst "soundstart"
#define soundFinishConst "soundfinish"
#define soundMoveConst "soundmove"
#define soundErrorConst "sounderror"
#define constDndDisable "dnddsbl"
#define constDefSoundSettings "defsndstngs"


class ChessPlugin: public QObject, public PsiPlugin, public OptionAccessor, public ActiveTabAccessor, public MenuAccessor, public ApplicationInfoAccessor,
		    public ToolbarIconAccessor, public IconFactoryAccessor, public StanzaSender, public AccountInfoAccessor, public StanzaFilter,
		    public PluginInfoProvider, public EventCreator
{
	Q_OBJECT
        Q_INTERFACES(PsiPlugin AccountInfoAccessor OptionAccessor ActiveTabAccessor MenuAccessor StanzaFilter
		     ToolbarIconAccessor IconFactoryAccessor StanzaSender ApplicationInfoAccessor PluginInfoProvider EventCreator)

public:
        ChessPlugin();
	virtual QString name() const;
	virtual QString shortName() const;
	virtual QString version() const;
        virtual QWidget* options();
	virtual bool enable();
        virtual bool disable();
        virtual void applyOptions();
        virtual void restoreOptions();
        virtual void optionChanged(const QString& /*option*/){};
        virtual void setAccountInfoAccessingHost(AccountInfoAccessingHost* host);
        virtual void setOptionAccessingHost(OptionAccessingHost* host);        
        virtual void setActiveTabAccessingHost(ActiveTabAccessingHost* host);
        virtual void setIconFactoryAccessingHost(IconFactoryAccessingHost* host);
        virtual void setStanzaSendingHost(StanzaSendingHost *host);
        virtual QList < QVariantHash >* getButtonParam();
	virtual QAction* getAction(QObject* , int , const QString& ) { return 0; };
        virtual QList < QVariantHash >* getAccountMenuParam();
        virtual QList < QVariantHash >* getContactMenuParam();
        virtual void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host);
        virtual bool incomingStanza(int account, const QDomElement& xml);
	virtual bool outgoingStanza(int account, QDomElement& xml);
	virtual void setEventCreatingHost(EventCreatingHost *host);
	virtual QString pluginInfo();

private:
        bool enabled;
        
        OptionAccessingHost* psiOptions;
        AccountInfoAccessingHost *AccInfoHost;
        ActiveTabAccessingHost* activeTab;       
        IconFactoryAccessingHost *IcoHost;
        ApplicationInfoAccessingHost* appInfo;
        StanzaSendingHost *stanzaSender;
	EventCreatingHost *psiEvent;
       // bool UseMenu, UseToolbutton;

        QString jid_, yourJid_;
        int account_;
        Figure::GameType type_;
        ChessWindow *board;
        bool game_, theEnd_;
        bool waitFor;
        QSet<QString> jidList;
        int id;
        QString tmpId;
        QString chessId;

        QString soundStart, soundFinish, soundMove, soundError;
        bool DndDisable, DefSoundSettings, enableSound;

        QString newId();
        bool checkId(QString id);
        void acceptGame();
        void rejectGame();
        void stopGame();
        void playSound(QString filename);
        void boardClosed();
        Ui::options ui_;

	struct Request {
		int account;
		QString jid;
		QString yourJid;
		Figure::GameType type;
		QString tmpId;
		QString chessId;
	};
	QList<Request> requests;

	int findRequest(QString jid);

public slots:
        void closeBoardEvent();
        void move(int oldX, int oldY, int newX, int newY, QString figure);
        void moveAccepted();
        void error();
        void load(QString settings);

private slots:
        void toolButtonPressed();
        void menuActivated();
        void invite();
        void sendInvite(QString resource, QString color);
        void accept();
        void reject();
        void youWin();
        void youLose();
        void draw();
        void getSound();
        void testSound();
        void toggleEnableSound(bool enable);
	void doInviteDialog(QString);
};

Q_EXPORT_PLUGIN(ChessPlugin);

ChessPlugin::ChessPlugin() {
        enabled = false;        
        AccInfoHost = 0;
        psiOptions = 0;
        IcoHost = 0;
        activeTab = 0;
        stanzaSender = 0;
        appInfo = 0;
	psiEvent = 0;
        board = 0;
        jidList.clear();        
        soundStart = "sound/chess_start.wav";
        soundFinish = "sound/chess_finish.wav";
        soundMove = "sound/chess_move.wav";
        soundError = "sound/chess_error.wav";
        DndDisable = true;
        DefSoundSettings = false;
        enableSound = true;
    }

QString ChessPlugin::name() const {
        return "Chess Plugin";
    }

QString ChessPlugin::shortName() const {
        return "chessplugin";
}

QString ChessPlugin::version() const {
        return cVer;
}

bool ChessPlugin::enable() {
	game_ = false;
	theEnd_ = false;
	waitFor = false;
	id = 111;
	chessId = "ch_111";
	requests.clear();

    if(!psiOptions) return false;
        enabled = true;
	QFile file(":/chessplugin/figures/Black queen 2d.png");
	if(file.open(QIODevice::ReadOnly)) {
	    QByteArray ico = file.readAll();
            IcoHost->addIcon("chessplugin/chess", ico);
	    file.close();
        }
        soundStart = psiOptions->getPluginOption(soundStartConst, QVariant(soundStart)).toString();
        soundFinish = psiOptions->getPluginOption(soundFinishConst, QVariant(soundFinish)).toString();
        soundMove = psiOptions->getPluginOption(soundMoveConst, QVariant(soundMove)).toString();
        soundError = psiOptions->getPluginOption(soundErrorConst, QVariant(soundError)).toString();
        DndDisable = psiOptions->getPluginOption(constDndDisable, QVariant(DndDisable)).toBool();
        DefSoundSettings = psiOptions->getPluginOption(constDefSoundSettings, QVariant(DefSoundSettings)).toBool();
        return enabled;
}

bool ChessPlugin::disable() {
    if(board) {
        delete (board);
        board = 0;
        game_ = false;
    }

    enabled = false;
    return true;
}

QWidget* ChessPlugin::options() {
    if(!enabled)
        return 0;

    QWidget *options = new QWidget;
    ui_.setupUi(options);
    ui_.wiki->setText(tr("<a href=\"http://psi-plus.com/wiki/plugins#chess_plugin\">Wiki (Online)</a>"));
    ui_.wiki->setOpenExternalLinks(true);

    ui_.le_error->setText(soundError);
    ui_.le_finish->setText(soundFinish);
    ui_.le_move->setText(soundMove);
    ui_.le_start->setText(soundStart);

    ui_.play_error->setIcon(IcoHost->getIcon("psi/play"));
    ui_.play_finish->setIcon(IcoHost->getIcon("psi/play"));
    ui_.play_move->setIcon(IcoHost->getIcon("psi/play"));;
    ui_.play_start->setIcon(IcoHost->getIcon("psi/play"));

    ui_.select_error->setIcon(IcoHost->getIcon("psi/browse"));
    ui_.select_finish->setIcon(IcoHost->getIcon("psi/browse"));
    ui_.select_move->setIcon(IcoHost->getIcon("psi/browse"));
    ui_.select_start->setIcon(IcoHost->getIcon("psi/browse"));

    ui_.cb_sound_override->setChecked(DefSoundSettings);
    ui_.cb_disable_dnd->setChecked(DndDisable);

    connect(ui_.play_error, SIGNAL(pressed()), this, SLOT(testSound()));
    connect(ui_.play_finish, SIGNAL(pressed()), this, SLOT(testSound()));
    connect(ui_.play_move, SIGNAL(pressed()), this, SLOT(testSound()));
    connect(ui_.play_start, SIGNAL(pressed()), this, SLOT(testSound()));

    connect(ui_.select_error, SIGNAL(pressed()), this, SLOT(getSound()));
    connect(ui_.select_finish, SIGNAL(pressed()), this, SLOT(getSound()));
    connect(ui_.select_start, SIGNAL(pressed()), this, SLOT(getSound()));
    connect(ui_.select_move, SIGNAL(pressed()), this, SLOT(getSound()));
    return options;
}

void ChessPlugin::applyOptions(){
    soundError = ui_.le_error->text();
    psiOptions->setPluginOption(soundErrorConst, QVariant(soundError));
    soundFinish = ui_.le_finish->text();
    psiOptions->setPluginOption(soundFinishConst, QVariant(soundFinish));
    soundMove = ui_.le_move->text();
    psiOptions->setPluginOption(soundMoveConst, QVariant(soundMove));
    soundStart = ui_.le_start->text();
    psiOptions->setPluginOption(soundStartConst, QVariant(soundStart));
    DndDisable = ui_.cb_disable_dnd->isChecked();
    psiOptions->setPluginOption(constDndDisable, QVariant(DndDisable));
    DefSoundSettings = ui_.cb_sound_override->isChecked();
    psiOptions->setPluginOption(constDefSoundSettings, QVariant(DefSoundSettings));
}

void ChessPlugin::restoreOptions(){
    ui_.le_error->setText(soundError);
    ui_.le_finish->setText(soundFinish);
    ui_.le_move->setText(soundMove);
    ui_.le_start->setText(soundStart);
    ui_.cb_disable_dnd->setChecked(DndDisable);
    ui_.cb_sound_override->setChecked(DefSoundSettings);
}

void ChessPlugin::setAccountInfoAccessingHost(AccountInfoAccessingHost* host) {
     AccInfoHost = host;
 }

void ChessPlugin::setOptionAccessingHost(OptionAccessingHost* host) {
    psiOptions = host;
}

void ChessPlugin::setActiveTabAccessingHost(ActiveTabAccessingHost* host) {
    activeTab = host;
}

void ChessPlugin::setIconFactoryAccessingHost(IconFactoryAccessingHost *host) {
    IcoHost = host;
}

void ChessPlugin::setStanzaSendingHost(StanzaSendingHost *host) {
    stanzaSender = host;
}

void ChessPlugin::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host) {
    appInfo = host;
}

void ChessPlugin::setEventCreatingHost(EventCreatingHost *host) {
	psiEvent = host;
}

QList < QVariantHash >* ChessPlugin::getButtonParam() {
    QList< QVariantHash > * l = new QList< QVariantHash >();    
    QVariantHash hash;
    hash["tooltip"] = QVariant(tr("Chess!"));
    hash["icon"] = QVariant(QString("chessplugin/chess"));
    hash["reciver"] = qVariantFromValue(qobject_cast<QObject *>(this));
    hash["slot"] = QVariant(SLOT(toolButtonPressed()));
    l->push_back(hash);    
    return l;
}

QList < QVariantHash >* ChessPlugin::getAccountMenuParam() {
    return 0;
}

QList < QVariantHash >* ChessPlugin::getContactMenuParam() {
    QList< QVariantHash > * l = new QList< QVariantHash >();
    QVariantHash hash;
    hash["name"] = QVariant(tr("Chess!"));
    hash["icon"] = QVariant(QString("chessplugin/chess"));
    hash["reciver"] = qVariantFromValue(qobject_cast<QObject *>(this));
    hash["slot"] = QVariant(SLOT(menuActivated()));
    l->push_back(hash);
    return l;
}

void ChessPlugin::toolButtonPressed() {
    if(!enabled) return;
    if(game_) {
        if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundError);
        return;
    }
    yourJid_ = activeTab->getYourJid();
    jid_ = activeTab->getJid().split("/").first();
    QString tmpJid("");
    account_ = 0;
    while (yourJid_ != (tmpJid = AccInfoHost->getJid(account_))){
        ++account_;
        if (tmpJid == "-1") return;
    }

    if(AccInfoHost->getStatus(account_) == "offline") return;

    invite();
}

void ChessPlugin::menuActivated() {
    if(!enabled) return;
    if(game_) {
        if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundError);
        return;
    }
    account_ = sender()->property("account").toInt();
    if(AccInfoHost->getStatus(account_) == "offline") return;
    jid_ = sender()->property("jid").toString();
    yourJid_ = AccInfoHost->getJid(account_);

    invite();
}

void ChessPlugin::invite() {
    QStringList list;
    foreach(QString fullJid, jidList) {
        if(fullJid.contains(jid_) && fullJid.contains("/"))
            list.append(fullJid.split("/").last());
    }

    InviteDialog *id = new InviteDialog(list);
    connect(id, SIGNAL(play(QString,QString)), this, SLOT(sendInvite(QString,QString)));
    id->show();
}

void ChessPlugin::sendInvite(QString resource, QString color) {
    jid_ += "/" + resource;   
    stanzaSender->sendStanza(account_, QString("<iq type='set' to='%1' id='%2'><create xmlns='games:board' id='%4' type='chess' color='%3'></create></iq>")
                             .arg(jid_)
                             .arg(newId())
                             .arg(color)
                             .arg(chessId));
    if(color == "white")
        type_ = Figure::WhitePlayer;
    else
        type_ = Figure::BlackPlayer;

    waitFor = true;
}

void ChessPlugin::accept() {
    stanzaSender->sendStanza(account_, QString("<iq type='result' to='%1' id='%2'><create xmlns='games:board' type='chess' id='%3'/></iq>")
                             .arg(jid_).arg(tmpId).arg(chessId));
    acceptGame();
}

void ChessPlugin::reject() {
    stanzaSender->sendStanza(account_, QString("<iq type='error' to='%1' id='%2'></iq>")
                             .arg(jid_).arg(tmpId));
    rejectGame();
}

void ChessPlugin::closeBoardEvent() {
    stanzaSender->sendStanza(account_, QString("<iq type='set' to='%1' id='%2'><close xmlns='games:board' id='%3' type='chess'></close></iq>")
                             .arg(jid_)
                             .arg(newId())
                             .arg(chessId));

    if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundFinish);
    stopGame();
}

void ChessPlugin::boardClosed() {
    if(theEnd_) return;
    QMessageBox::warning(board, tr("Chess Plugin"),
                         tr("Your opponent has closed the board!\n You can still save the game."), QMessageBox::Ok);
}

void ChessPlugin::stopGame() {
    delete (board);
    board = 0;
    game_ = false;
    theEnd_ = false;
}

void ChessPlugin::acceptGame() {
    if(game_) return;
    game_ = true;
    waitFor = false;
    theEnd_ = false;
    board = new ChessWindow(type_, enableSound);
    connect(board, SIGNAL(closeBoard()), this, SLOT(closeBoardEvent()));
    connect(board, SIGNAL(move(int,int,int,int, QString)), this, SLOT(move(int,int,int,int, QString)));
    connect(board, SIGNAL(moveAccepted()), this, SLOT(moveAccepted()));
    connect(board, SIGNAL(error()), this, SLOT(error()));
    connect(board, SIGNAL(load(QString)), this, SLOT(load(QString)));
    connect(board, SIGNAL(draw()), this, SLOT(draw()));
    connect(board, SIGNAL(lose()), this, SLOT(youLose()));
    connect(board, SIGNAL(toggleEnableSound(bool)), this, SLOT(toggleEnableSound(bool)));
    board->show();
    if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundStart);
}

void ChessPlugin::rejectGame() {
    game_ = false;
    waitFor = false;
    theEnd_ = false;
    if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundFinish);
}

void ChessPlugin::load(QString settings) {
    stanzaSender->sendStanza(account_, QString("<iq type='set' to='%1' id='%2'><load xmlns='games:board' id='%3' type='chess'>%4</load></iq>")
                             .arg(jid_)
                             .arg(newId())
                             .arg(chessId)
                             .arg(settings));
}

void ChessPlugin::move(int oldX, int oldY, int newX, int newY, QString figure) {
    QString stanza = QString("<iq type='set' to='%1' id='%2'><turn xmlns='games:board' type='chess' id='%7'><move pos='%3,%4;%5,%6'>")
                             .arg(jid_)
                             .arg(newId())
                             .arg(QString::number(oldX))
                             .arg(QString::number(oldY))
                             .arg(QString::number(newX))
                             .arg(QString::number(newY))
                             .arg(chessId);
    if(!figure.isEmpty())
        stanza += QString("<promotion>%1</promotion>").arg(figure);
    stanza += "</move></turn></iq>";
    stanzaSender->sendStanza(account_, stanza);
    if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundMove);
}

void ChessPlugin::moveAccepted() {
    stanzaSender->sendStanza(account_, QString("<iq type='result' to='%1' id='%2'><turn type='chess' id='%3' xmlns='games:board'/></iq>")
                             .arg(jid_)
                             .arg(tmpId)
                             .arg(chessId));
}

void ChessPlugin::youLose() {
	if(theEnd_) return;
    stanzaSender->sendStanza(account_, QString("<iq type='set' to='%1' id='%2'><turn xmlns='games:board' type='chess' id='%3'><resign/></turn></iq>")
                             .arg(jid_)
                             .arg(tmpId)
                             .arg(chessId));
    board->youLose();
    theEnd_ = true;
    QMessageBox::information(board, tr("Chess Plugin"),
                         tr("You Lose."), QMessageBox::Ok);    
}

void ChessPlugin::youWin() {
	if(theEnd_) return;
    if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundStart);
    board->youWin();
    theEnd_ = true;
    QMessageBox::information(board, tr("Chess Plugin"),
                         tr("You Win!"), QMessageBox::Ok);
}

void ChessPlugin::draw() {
    if(!theEnd_) {
        stanzaSender->sendStanza(account_, QString("<iq type='set' to='%1' id='%2'><turn xmlns='games:board' type='chess' id='%3'><draw/></turn></iq>")
                             .arg(jid_)
                             .arg(tmpId)
                             .arg(chessId));
        if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundStart);
        board->youDraw();
        theEnd_ = true;
        QMessageBox::information(board, tr("Chess Plugin"),
                         tr("Draw!"), QMessageBox::Ok);
    }
}

void ChessPlugin::error() {
    if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundError);
    QMessageBox::warning(board, tr("Chess Plugin"),
                         tr("Unknown error!"), QMessageBox::Ok);
    board->close();
}

bool ChessPlugin::incomingStanza(int account, const QDomElement& xml)
{
    if(!enabled) return false;;
    if(xml.tagName() == "presence") {
        jidList << xml.attribute("from");
    }
    else if(xml.tagName() == "iq"){
         if(xml.attribute("type") == "set") {
             QDomElement createElem = xml.firstChildElement("create");
             if(!createElem.isNull() && createElem.attribute("xmlns") == "games:board"
                && createElem.attribute("type") == "chess") {
                 if((DndDisable && AccInfoHost->getStatus(account) == "dnd") || game_) {
			 stanzaSender->sendStanza(account, QString("<iq type='error' to='%1' id='%2'></iq>")
						  .arg(xml.attribute("from")).arg(xml.attribute("id")));
			return true;
                 }
		 QString color = createElem.attribute("color");

		 Request r;
		 r.tmpId = xml.attribute("id");
		 r.chessId = createElem.attribute("id");
		 r.account = account;
		 r.jid = xml.attribute("from");
		 r.yourJid = AccInfoHost->getJid(account);
		 r.type = Figure::WhitePlayer;
		 if(color == "white")
		     r.type = Figure::BlackPlayer;
		 requests.append(r);

		 psiEvent->createNewEvent(account, r.jid, tr("Chess Plugin: Invitation from %1").arg(r.jid), this, SLOT(doInviteDialog(QString)));
                 return true;
             }
             QDomElement turn = xml.firstChildElement("turn");
             if(!turn.isNull() && turn.attribute("xmlns") == "games:board" && turn.attribute("type") == "chess" && game_) {
                 tmpId = xml.attribute("id");
                 QDomNode node = turn.firstChild();
                 while(!node.isNull()) {
                     QDomElement childElem = node.toElement();
                     if(childElem.tagName() == "move") {
                         QStringList tmpMove = childElem.attribute("pos").split(";");
                         int oldX = tmpMove.first().split(",").first().toInt();
                         int oldY = tmpMove.first().split(",").last().toInt();
                         int newX = tmpMove.last().split(",").first().toInt();
                         int newY = tmpMove.last().split(",").last().toInt();                         
                         QDomElement promElem = childElem.firstChildElement("promotion");
                         QString figure = "";
                         if(!promElem.isNull())
                             figure = promElem.text();
                         board->moveRequest(oldX,oldY,newX,newY,figure);
                         if((DefSoundSettings || psiOptions->getGlobalOption("options.ui.notifications.sounds.enable").toBool()) && enableSound)
                             playSound(soundMove);
                     }
                     else if(childElem.tagName() == "draw") {
                         draw();
                         return true;
                     }
                     else if(childElem.tagName() == "resign") {
                         youWin();
                         return true;
                     }
                     node = node.nextSibling();
                 }
                 return true;
             }
             QDomElement closeElem = xml.firstChildElement("close");
             if(!closeElem.isNull() && closeElem.attribute("xmlns") == "games:board" && closeElem.attribute("type") == "chess" && game_) {
                 boardClosed();
                 return true;
             }
             QDomElement loadElem = xml.firstChildElement("load");
             if(!loadElem.isNull() && loadElem.attribute("xmlns") == "games:board" && loadElem.attribute("type") == "chess" && game_) {
                 board->loadRequest(loadElem.text());
                 return true;
             }
         }
         else if(xml.attribute("type") == "result") {
             if(waitFor && checkId(xml.attribute("id"))) {
                 acceptGame();
                 return true;
             }
         } else if(xml.attribute("type") == "error") {
             if(waitFor && checkId(xml.attribute("id"))) {
                 rejectGame();
                 return true;
             }
         }
     }
    return false;
}

bool ChessPlugin::outgoingStanza(int /*account*/, QDomElement& /*xml*/)
{
	return false;
}

void ChessPlugin::doInviteDialog(QString jid) {
	if(!enabled || requests.isEmpty())
		return;

	int index = findRequest(jid);
	if(index == -1)
		return;

	Request rec = requests.takeAt(index);

	if(game_) {
		QMessageBox::information(0, tr("Chess Plugin"), tr("You are allready playing!"));
		stanzaSender->sendStanza(rec.account, QString("<iq type='error' to='%1' id='%2'></iq>")
					 .arg(rec.jid).arg(rec.tmpId));
		return;
	}

	chessId = rec.chessId;
	account_ = rec.account;
	jid_ = rec.jid;
	yourJid_ = rec.yourJid;
	tmpId = rec.tmpId;
	type_ = rec.type;

	QString color = "black";
	if(type_ == Figure::BlackPlayer)
		color = "white";

	InvitationDialog *id = new InvitationDialog(jid_, color);
	connect(id, SIGNAL(accept()), this, SLOT(accept()));
	connect(id, SIGNAL(reject()), this, SLOT(reject()));
	id->show();
}

int ChessPlugin::findRequest(QString jid)
{
	int index = -1;
	for(int i = 0; i < requests.size(); i++) {
		if(requests.at(i).jid == jid) {
			index = i; //не останавливаем поиск, т.к. нам нужно найти последний индекс
		}
	}
	return index;
}

void ChessPlugin::testSound() {
    if(ui_.play_error->isDown()) {
        playSound(ui_.le_error->text());
    } else if(ui_.play_finish->isDown()) {
        playSound(ui_.le_finish->text());
    } else if(ui_.play_move->isDown()) {
        playSound(ui_.le_move->text());
    } else if(ui_.play_start->isDown()) {
        playSound(ui_.le_start->text());
    }
}

void ChessPlugin::getSound() {
    QLineEdit *le = 0;
    if(ui_.select_error->isDown())
        le = ui_.le_error;
    if(ui_.select_finish->isDown())
        le = ui_.le_finish;
    if(ui_.select_move->isDown())
        le = ui_.le_move;
    if(ui_.select_start->isDown())
        le = ui_.le_start;

    if(!le) return;

    QString fileName = QFileDialog::getOpenFileName(0,tr("Choose a sound file"),"", tr("Sound (*.wav)"));
    if(fileName.isEmpty()) return;
    le->setText(fileName);
}

void ChessPlugin::playSound(QString f) {
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

QString ChessPlugin::newId() {
    id++;
    QString newid = "cp_"+QString::number(id);
    return newid;
}

bool ChessPlugin::checkId(QString checkId) {
    return checkId == "cp_"+QString::number(id);
}

void ChessPlugin::toggleEnableSound(bool enable) {
    enableSound = enable;
}

QString ChessPlugin::pluginInfo() {
	return tr("Author: ") +  "Dealer_WeARE\n"
			+ tr("Email: ") + "wadealer@gmail.com\n\n"
			+ trUtf8("This plugin allows you to play chess with your friends.\n"
			 "The plugin is compatible with a similar plugin for Tkabber.\n"
			 "For sending commands, normal messages are used, so this plugin will always work wherever you are able to log in."
			 "To invite a friend for a game, you can use contact menu item or the button on the toolbar in a chat window.");
}


#include "chessplugin.moc"
