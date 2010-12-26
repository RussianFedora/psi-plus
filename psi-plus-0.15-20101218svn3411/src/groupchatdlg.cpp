/*
 * groupchatdlg.cpp - dialogs for handling groupchat
 * Copyright (C) 2001, 2002  Justin Karneges
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

// TODO: Move all the 'logic' of groupchats into MUCManager. See MUCManager
// for more details.

#include "groupchatdlg.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QToolBar>
#include <QMessageBox>
#include <QColorGroup>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QInputDialog>
#include <QPointer>
#include <QAction>
#include <QObject>
#include <QCursor>
#include <QCloseEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QFrame>
#include <QList>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QTextCursor>
#include <QTextDocument> // for Qt::escape()
#include <QToolTip>
#include <QScrollBar>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QDesktopWidget>

#include "psicon.h"
#include "psiaccount.h"
#include "capsmanager.h"
#include "userlist.h"
#include "mucconfigdlg.h"
#include "textutil.h"
#include "statusdlg.h"
#include "xmpp_message.h"
#include "psiiconset.h"
#include "stretchwidget.h"
#include "mucmanager.h"
#include "busywidget.h"
#include "msgmle.h"
#include "messageview.h"
#include "iconwidget.h"
#include "iconselect.h"
#include "xmpp_tasks.h"
#include "iconaction.h"
#include "psitooltip.h"
#include "psioptions.h"
#include "urlobject.h"
#include "shortcutmanager.h"
#include "psicontactlist.h"
#include "accountlabel.h"
#include "gcuserview.h"
#include "bookmarkmanager.h"
#include "mucreasonseditor.h"
#include "mcmdmanager.h"
#include "lastactivitytask.h"
#include "pluginmanager.h"
#include "psirichtext.h"
#include "psiwindowheader.h"

#include "mcmdsimplesite.h"

#include "tabcompletion.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include "psipopup.h"
#include "psievent.h"

#define MCMDMUC		"http://psi-im.org/ids/mcmd#mucmain"
#define MCMDMUCNICK	"http://psi-im.org/ids/mcmd#mucnick"




#include "groupchattopicdlg.h"
#include "sendbuttonmenu.h"
#include "typeaheadfind.h"
//----------------------------------------------------------------------------
// StatusPingTask
//----------------------------------------------------------------------------
#include "xmpp_xmlcommon.h"

class StatusPingTask : public Task
{
	Q_OBJECT
public:
	StatusPingTask(const Jid& myjid, Task* parent) : Task(parent), myjid_(myjid)
	{
	}


	void onGo() {
		iq_ = createIQ(doc(), "get", myjid_.full(), id());

		QDomElement ping = doc()->createElement("ping");
		ping.setAttribute("xmlns", "urn:xmpp:ping");
		iq_.appendChild(ping);
		timeout.setSingleShot ( true );
		timeout.setInterval( 1000 * 60 * 10 );
		connect(&timeout, SIGNAL(timeout()), SLOT(timeout_triggered()));
		send(iq_);
	}

	bool take(const QDomElement& x) {
		if(!iqVerify(x, myjid_, id()))  // , "urn:xmpp:ping"
			return false;

		if(x.attribute("type") == "result") {
			// something bad, we never reply to this stanza so someone
			// else got it.
			// FIXME seems to be no longer true
			//emit result(NotUs, id());
			emit result(LoggedIn, id());
			setSuccess();
		} else if(x.attribute("type") == "get") {
			// All went well!
			emit result(LoggedIn, id());
			setSuccess();
		} else {
			bool found;
			QDomElement tag = findSubTag(x, "error", &found);
			if(!found) {
				emit result(OtherErr, id());
			} else {
				XMPP::Stanza::Error err;
				err.fromXml(tag, client()->stream().baseNS());
				if (err.condition == XMPP::Stanza::Error::ItemNotFound) {
					emit result(NoSuch, id());
				} else if (err.condition == XMPP::Stanza::Error::NotAcceptable ) {
					emit result(NotOccupant, id());
				} else {
					emit result(OtherErr, id());
				}
				setSuccess();
			}
		}
		return true;
	}

	enum Result { NotOccupant, Timeout, NotUs, NoSuch, LoggedIn, OtherErr};

signals:
	void result(StatusPingTask::Result res, QString id);

private slots:
	void timeout_triggered() {
		emit result(Timeout, id());
		setSuccess();
	}
private:
	QDomElement iq_;
	Jid myjid_;
	QString xid;
	QTimer timeout;
};


//----------------------------------------------------------------------------
// GCMainDlg
//----------------------------------------------------------------------------
class GCMainDlg::Private : public QObject, public MCmdProviderIface
{
	Q_OBJECT
public:
	enum { Connecting, Connected, Idle, ForcedLeave };
	Private(GCMainDlg *d) : mCmdManager(&mCmdSite), tabCompletion(this) {
		dlg = d;
		nickSeparator = ":";
		nonAnonymous = false;
		
		trackBar = false;
		mCmdManager.registerProvider(this);
	}

	GCMainDlg *dlg;
	int state;
	MUCManager *mucManager;
	QString self, prev_self;
	QString password;
	QString topic;
	bool nonAnonymous;		 // got status code 100 ?
	IconAction *act_find, *act_clear, *act_icon, *act_configure, *act_ps, *act_templates, *act_bookmark, *act_html_text;
	TypeAheadFindBar *typeahead;
#ifdef WHITEBOARDING
	IconAction *act_whiteboard;
#endif
	QAction *act_send, *act_scrollup, *act_scrolldown, *act_close;

	QAction *act_mini_cmd, *act_nick, *act_hide, *act_minimize;

	MCmdSimpleSite mCmdSite;
	MCmdManager mCmdManager;

	QString nickSeparator; // equals ":"

	QMenu *pm_settings;
	SendButtonTemplatesMenu *template_menu;	
	int pending;
	int hPending; // highlight pending
	bool connecting;

	QStringList hist;
	int histAt;

	//QPointer<GCFindDlg> findDlg;
	//QString lastSearch;

	QPointer<MUCConfigDlg> configDlg;
	QPointer<GroupchatTopicDlg> topicDlg;

	int logSize;
	int rosterSize;

	int logHeight;
	int chateditHeight;

public:
	bool trackBar;
	
	bool tabmode;
	PsiWindowHeader *winHeader_;

public:
	ChatEdit* mle() const { return dlg->ui_.mle->chatEdit(); }
	ChatView* te_log() const { return dlg->ui_.log; }

public slots:
	void addEmoticon(const PsiIcon *icon) {
		addEmoticon(icon->defaultText());
	}

	void addEmoticon(QString text) {
		if ( !dlg->isActiveTab() )
			return;

		PsiRichText::addEmoticon(mle(), text);
	}

	void sp_result(StatusPingTask::Result res, QString id)
	{
		//qDebug() << res;
		QString base = QString("Done Status ping (id=%1) ").arg(id);
		switch (res) {
			case StatusPingTask::NotOccupant:
				dlg->appendSysMsg(base + "NotOccupant", false);
				break;
			case StatusPingTask::Timeout:
				dlg->appendSysMsg(base + "Timeout", false);
				break;
			case StatusPingTask::NotUs:
				dlg->appendSysMsg(base + "NotUs", false);
				break;
			case StatusPingTask::NoSuch:
				dlg->appendSysMsg(base + "NoSuch", false);
				break;
			case StatusPingTask::LoggedIn:
				dlg->appendSysMsg(base + "LoggedIn", false);
				break;
			case StatusPingTask::OtherErr:
				dlg->appendSysMsg(base + "OtherErr", false);
				break;
		}
	}

	void version_finished()
	{
		JT_ClientVersion *version = qobject_cast<JT_ClientVersion*>(sender());
		if (!version) {
			dlg->appendSysMsg(QString("No version information available for %1.").arg(version->jid().resource()) , false);
			return;
		}
		dlg->appendSysMsg(QString("Version response from %1: N: %2 V: %3 OS: %4")
			.arg(version->jid().resource(), version->name(), version->version(), version->os()), false);
	}

	void lastactivity_finished()
	{
		LastActivityTask *idle = (LastActivityTask *)sender();

		if (!idle->success()) {
			dlg->appendSysMsg(QString("Can't determine last activity time for %1.").arg(idle->jid().resource()), false);
			return;
		}

		if (idle->status().isEmpty()) {
			dlg->appendSysMsg(QString("Last activity from %1 at %2")
				.arg(idle->jid().resource(), idle->time().toString()), false);
		} else {
			dlg->appendSysMsg(QString("Last activity from %1 at %2 (%3)")
				.arg(idle->jid().resource(), idle->time().toString(), idle->status()), false);
		}
	}

	void doSPing()
	{
		Jid full = dlg->jid().withResource(self);
		StatusPingTask *sp = new StatusPingTask(full, dlg->account()->client()->rootTask());
		connect(sp, SIGNAL(result(StatusPingTask::Result, QString)), SLOT(sp_result(StatusPingTask::Result, QString)));
		sp->go(true);
		dlg->appendSysMsg(QString("Doing Status ping (id=%1)").arg(sp->id()), false);
	}

	void doNick()
	{
		MCmdSimpleState *state = new MCmdSimpleState(MCMDMUCNICK, tr("new nick") + '=', MCMDSTATE_UNPARSED);
		connect(state, SIGNAL(unhandled(QStringList)), SLOT(NickComplete(QStringList)));
		mCmdManager.open(state, QStringList() << self);
	}

	bool NickComplete(QStringList command)
	{
		if (command.count() > 0) {
			QString nick = command[0].trimmed();
			if ( !nick.isEmpty() ) {
				prev_self = self;
				self = nick;
				dlg->account()->groupChatChangeNick(dlg->jid().domain(), dlg->jid().node(), self, dlg->account()->status());
			}
		}
		return true;
	}

	void doMiniCmd()
	{
		mCmdManager.open(new MCmdSimpleState(MCMDMUC, tr("Command") + '>'), QStringList() );
	}

public:
	virtual bool mCmdTryStateTransit(MCmdStateIface *oldstate, QStringList command, MCmdStateIface *&newstate, QStringList &preset) {
		if (oldstate->getName() == MCMDMUC) {
			QString cmd;
			if (command.count() > 0) cmd = command[0].toLower();
	/*
TODO:
topic <topic>
invite <jid>
part [message]
kick <jid|nickname> [comment]
ban <jid|nickname>

Maybe?:
join <channel>{,<channel>}
query <user>
join <channel>{,<channel>} [pass{,<pass>}
	*/

			if(cmd == "clear") {
				dlg->doClear();
				histAt = 0;
				newstate = 0;
			} else if(cmd == "nick") {
				if (command.count() > 1) {
					QString nick = command[1].trimmed();
					// FIXME nick can't be empty....
					prev_self = self;
					self = nick;
					dlg->account()->groupChatChangeNick(dlg->jid().domain(), dlg->jid().node(), self, dlg->account()->status());
					newstate = 0;
				} else {
					// FIXME DRY with doNick
					MCmdSimpleState *state = new MCmdSimpleState("nick", tr("new nick") + '=', MCMDSTATE_UNPARSED);
					connect(state, SIGNAL(unhandled(QStringList)), SLOT(NickComplete(QStringList)));
					newstate = state;
					preset = QStringList() << self;
				}
			} else if(cmd == "sping") {
				doSPing();
				newstate = 0;
			} else if (cmd == "version" && command.count() > 1) {
				QString nick = command[1].trimmed();
				Jid target = dlg->jid().withResource(nick);
				JT_ClientVersion *version = new JT_ClientVersion(dlg->account()->client()->rootTask());
				connect(version, SIGNAL(finished()), SLOT(version_finished()));
				version->get(target);
				version->go();
				newstate = 0;
			} else if (cmd == "idle" && command.count() > 1) {
				QString nick = command[1].trimmed();
				Jid target = dlg->jid().withResource(nick);
				LastActivityTask *idle = new LastActivityTask(target, dlg->account()->client()->rootTask());
				connect(idle, SIGNAL(finished()), SLOT(lastactivity_finished()));
				idle->go();
				newstate = 0;
			} else if (cmd == "quote") {
				dlg->appendSysMsg(command.join("|"), false);
				preset = command;
				newstate = oldstate;
				return true;
			} else if (!cmd.isEmpty()) {
				return false;
			}
		} else {
			return false;
		}

		return true;
	}

	virtual QStringList mCmdTryCompleteCommand(MCmdStateIface *state, QString query, QStringList partcommand, int item) {
		//qDebug() << "mCmdTryCompleteCommand " << item << ":" << query;
		QStringList all;
		if (state->getName() == MCMDMUC) {
			QString spaceAtEnd = QString(QChar(0));
			if (item == 0) {
				all << "clear" + spaceAtEnd << "nick" + spaceAtEnd << "sping" + spaceAtEnd << "version" + spaceAtEnd << "idle" + spaceAtEnd << "quote" + spaceAtEnd;
			} else if (item == 1 && (partcommand[0] == "version" || partcommand[0] == "idle")) {
				all = dlg->ui_.lv_users->nickList();
			}
		}
		QStringList res;
		foreach(QString cmd, all) {
			if (cmd.startsWith(query, Qt::CaseInsensitive)) {
				res << cmd;
			}
		}
		return res;
	}

	virtual void mCmdSiteDestroyed() {
	}

public:		
	void doTrackBar()
	{
		trackBar = false;
		te_log()->doTrackBar();
	}

public:
	QString lastReferrer;  // contains nick of last person, who have said "yourNick: ..."

public slots:
	/** Insert a nick FIXME called from mini roster.
	 */
	void insertNick(const QString& nick)
	{
		if (nick.isEmpty())
			return;

		QTextCursor cursor(mle()->textCursor());
	
		mle()->setUpdatesEnabled(false);
		cursor.beginEditBlock();

		int index = cursor.position();
		cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
		QString prev = cursor.selectedText();
		cursor.setPosition(index, QTextCursor::KeepAnchor);

		if (index > 0) {
			if (!prev.isEmpty() && !prev[0].isSpace())
				mle()->insertPlainText(" ");
			mle()->insertPlainText(nick);
		}
		else {
			mle()->insertPlainText(nick);
			mle()->insertPlainText(nickSeparator);
		}
		mle()->insertPlainText(" ");
		mle()->setFocus();

		cursor.endEditBlock();
		mle()->setUpdatesEnabled(true);
		mle()->viewport()->update();
	}

public:

	bool eventFilter( QObject *obj, QEvent *ev ) {
		if (te_log()->handleCopyEvent(obj, ev, mle()))
			return true;
	
		if ( obj == mle() && ev->type() == QEvent::KeyPress ) {
			QKeyEvent *e = (QKeyEvent *)ev;

			if ( e->key() == Qt::Key_Tab ) {
				tabCompletion.tryComplete();
				return true;
			}
			
			tabCompletion.reset();
			return false;
		}

		return QObject::eventFilter( obj, ev );
	}

	class TabCompletionMUC : public TabCompletion {
		public:
		GCMainDlg::Private *p_;
		TabCompletionMUC(GCMainDlg::Private *p) : p_(p), nickSeparator(":") {};

		virtual void setup(QString str, int pos, int &start, int &end) {
			if (p_->mCmdSite.isActive()) {
				mCmdList_ = p_->mCmdManager.completeCommand(str, pos, start, end);
			} else {
				TabCompletion::setup(str, pos, start, end);
			}
		}

		virtual QStringList possibleCompletions() {
			if (p_->mCmdSite.isActive()) {
				return mCmdList_;
			}
			QStringList suggestedNicks;
			QStringList nicks = allNicks();

			QString postAdd = atStart_ ? nickSeparator + " " : "";

			foreach(QString nick, nicks) {
				if (nick.left(toComplete_.length()).toLower() == toComplete_.toLower()) {
					suggestedNicks << nick + postAdd;
				}
			}
			return suggestedNicks;
		};

		virtual QStringList allChoices(QString &guess) {
			if (p_->mCmdSite.isActive()) {
				guess = QString();
				return mCmdList_;
			}
			guess = p_->lastReferrer;
			if (!guess.isEmpty() && atStart_) {
				guess += nickSeparator + " ";
			}

			QStringList all = allNicks();

			if (atStart_) {
				QStringList::Iterator it = all.begin();
				for ( ; it != all.end(); ++it) {
					*it = *it + nickSeparator + " ";
				}
			}
			return all;
		};

		QStringList allNicks() {
			return p_->dlg->ui_.lv_users->nickList();
		}

		QStringList mCmdList_;
		
		// FIXME where to move this?
		QString nickSeparator; // equals ":"
	};

	TabCompletionMUC tabCompletion;

};

void GCMainDlg::openURL(QString url)
{
	if (url.startsWith("addnick://") && isActiveTab()) {
		const QString nick = QUrl::fromPercentEncoding(url.mid(14).toAscii());
		if (ui_.mle->chatEdit()->text().length() == 0) {
			ui_.mle->chatEdit()->insertPlainText(nick + QString(": "));
		} else {
			ui_.mle->chatEdit()->insertPlainText(QString(" %1 ").arg(nick));
		}
		ui_.mle->chatEdit()->setFocus();
	}
}

void GCMainDlg::showNM(QString nick){
	//ui_.lv_users->findItem(nick, 0).itemPos ()
	Q3ListViewItem* itm = ui_.lv_users->findItem(nick, 0);
	if (itm != 0){
		ui_.lv_users->qlv_contextMenuRequested(itm, QCursor::pos (), 0);
	}
}

GCMainDlg::GCMainDlg(PsiAccount *pa, const Jid &j, TabManager *tabManager)
	: TabbableWidget(j.bare(), pa, tabManager)
{
	setAttribute(Qt::WA_DeleteOnClose);
	d = new Private(this);
	d->self = d->prev_self = j.resource();
	account()->dialogRegister(this, jid());
	connect(account(), SIGNAL(updatedActivity()), SLOT(pa_updatedActivity()));
	d->mucManager = new MUCManager(account()->client(), jid());

	options_ = PsiOptions::instance();

	d->pending = 0;
	d->hPending = 0;
	d->connecting = false;

	d->histAt = 0;
	//d->findDlg = 0;
	d->configDlg = 0;

	d->state = Private::Connected;
	d->template_menu = new SendButtonTemplatesMenu(this);

	setAcceptDrops(true);

#ifndef Q_WS_MAC
	setWindowIcon(IconsetFactory::icon("psi/groupChat").icon());
#endif

	ui_.setupUi(this);
	ui_.lb_ident->setAccount(account());
	ui_.lb_ident->setShowJid(false);
	ui_.log->setSessionData(true, jid().full(), jid().full()); //FIXME change conference name
	d->tabmode = PsiOptions::instance()->getOption("options.ui.tabs.use-tabs").toBool();
	setWindowBorder(PsiOptions::instance()->getOption("options.ui.decorate-windows").toBool());
	if (!d->tabmode && !isBorder()) {
		d->winHeader_ = new PsiWindowHeader(this);
		ui_.vboxLayout1->insertWidget(0, d->winHeader_);
	}
	setMargins();

#ifdef WEBKIT
	ui_.log->setAccount(account());
#endif

	connect(ui_.log, SIGNAL(showNM(QString)), this, SLOT(showNM(QString)));
	connect(URLObject::getInstance(), SIGNAL(openURL(QString)), SLOT(openURL(QString)));
	
	connect(ui_.pb_topic, SIGNAL(clicked()), SLOT(openTopic()));
	PsiToolTip::install(ui_.le_topic);

	connect(account()->psi(), SIGNAL(accountCountChanged()), this, SLOT(updateIdentityVisibility()));
	updateIdentityVisibility();
/*
	d->act_find = new IconAction(tr("Find"), "psi/search", tr("&Find"), 0, this);
	connect(d->act_find, SIGNAL(triggered()), SLOT(openFind()));
	ui_.tb_find->setDefaultAction(d->act_find);
*/
	ui_.tb_emoticons->setIcon(IconsetFactory::icon("psi/smile").icon());

#ifdef Q_WS_MAC
	// seems its useless hack
	//connect(ui_.log, SIGNAL(selectionChanged()), SLOT(logSelectionChanged()));
#endif

	ui_.lv_users->setMainDlg(this);
	if ( PsiOptions::instance()->getOption("options.ui.contactlist.disable-scrollbar").toBool() ) {
		ui_.lv_users->setVScrollBarMode(Q3ScrollView::AlwaysOff);
	}
	connect(ui_.lv_users, SIGNAL(action(const QString &, const Status &, int)), SLOT(lv_action(const QString &, const Status &, int)));
	connect(ui_.lv_users, SIGNAL(insertNick(const QString&)), d, SLOT(insertNick(const QString&)));

	// typeahead find bar
	QHBoxLayout *hb3a = new QHBoxLayout(ui_.vboxLayout1);
	d->typeahead = new TypeAheadFindBar(ui_.log->textWidget(), tr("Find toolbar"), 0);
	hb3a->addWidget( d->typeahead );

	d->act_clear = new IconAction (tr("Clear Chat Window"), "psi/clearChat", tr("Clear Chat Window"), 0, this);
	connect( d->act_clear, SIGNAL( activated() ), SLOT( doClearButton() ) );
	
	d->act_find = new IconAction(tr("Find"), "psi/search", tr("&Find"), 0, this, "", true);
	connect(d->act_find, SIGNAL(activated()), d->typeahead, SLOT(toggleVisibility()));

	d->act_configure = new IconAction(tr("Configure Room"), "psi/configure-room", tr("&Configure Room"), 0, this);
	connect(d->act_configure, SIGNAL(triggered()), SLOT(configureRoom()));

	d->act_html_text = new IconAction(tr("Set Text Format"), "psi/text", tr("Set Text Format"), 0, this);
	connect(d->act_html_text, SIGNAL(triggered()), d->mle(), SLOT(doHTMLTextMenu()));

#ifdef WHITEBOARDING
	d->act_whiteboard = new IconAction(tr("Open a Whiteboard"), "psi/whiteboard", tr("Open a &Whiteboard"), 0, this);
	connect(d->act_whiteboard, SIGNAL(triggered()), SLOT(openWhiteboard()));
#endif

	connect(pa->psi()->iconSelectPopup(), SIGNAL(textSelected(QString)), d, SLOT(addEmoticon(QString)));
	d->act_icon = new IconAction( tr( "Select Icon" ), "psi/smile", tr( "Select Icon" ), 0, this );
	d->act_icon->setMenu( pa->psi()->iconSelectPopup() );
	ui_.tb_emoticons->setMenu(pa->psi()->iconSelectPopup());

	d->act_nick = new QAction(this);
	d->act_nick->setText(tr("Change Nickname..."));
	connect(d->act_nick, SIGNAL(triggered()), d, SLOT(doNick()));

	d->act_mini_cmd = new QAction(this);
	d->act_mini_cmd->setText(tr("Enter Command..."));
	connect(d->act_mini_cmd, SIGNAL(triggered()), d, SLOT(doMiniCmd()));
	addAction(d->act_mini_cmd);

	d->act_minimize = new QAction(this);
	connect(d->act_minimize, SIGNAL(triggered()), SLOT(doMinimize()));
	addAction(d->act_minimize);

	d->act_ps = new IconAction(tr("Paste and Send"), "psi/action_paste_and_send", tr("Paste and Send"), 0, this);
	connect(d->act_ps, SIGNAL(activated()), SLOT(doPasteAndSend()));

	d->act_templates = new IconAction(tr("Templates"), "psi/action_templates", tr("Templates"), 0, this);
	d->act_templates->setMenu(d->template_menu);
	connect(d->act_templates, SIGNAL(hovered()), d->template_menu, SLOT(update()));

	d->act_bookmark = new IconAction(this);
	connect(d->act_bookmark, SIGNAL(activated()), SLOT(doBookmark()));
	ui_.le_topic->addAction(d->act_bookmark);
	BookmarkManager *bm = account()->bookmarkManager();
	d->act_bookmark->setVisible(bm->isAvailable());
	if (bm->isAvailable()) {
		updateBookmarkIcon();
	}
	connect(bm, SIGNAL(availabilityChanged()), SLOT(updateBookmarkIcon()));
	connect(bm, SIGNAL(conferencesChanged(QList<ConferenceBookmark>)), SLOT(updateBookmarkIcon()));
	connect(bm, SIGNAL(bookmarksSaved()), SLOT(updateBookmarkIcon()));

	int s = PsiIconset::instance()->system().iconSize();
	ui_.toolbar->setIconSize(QSize(s,s));
	ui_.toolbar->addAction(d->act_clear);
	ui_.toolbar->addAction(d->act_find);
	ui_.toolbar->addAction(d->act_configure);
	ui_.toolbar->addAction(d->act_html_text);
#ifdef WHITEBOARDING
	ui_.toolbar->addAction(d->act_whiteboard);
#endif
#ifdef PSI_PLUGINS
	PluginManager::instance()->addGCToolBarButton(this, ui_.toolbar, account(), jid().full());
#endif
	ui_.toolbar->addWidget(new StretchWidget(ui_.toolbar));
	ui_.toolbar->addAction(d->act_icon);
	((QToolButton *)ui_.toolbar->widgetForAction(d->act_icon))->setPopupMode(QToolButton::InstantPopup);
	ui_.toolbar->addAction(d->act_templates);
	((QToolButton *)ui_.toolbar->widgetForAction(d->act_templates))->setPopupMode(QToolButton::InstantPopup);
	if (!PsiOptions::instance()->getOption("options.ui.chat.disable-paste-send").toBool())
		ui_.toolbar->addAction(d->act_ps);
	ui_.toolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

	// Common actions
	d->act_send = new QAction(this);
	addAction(d->act_send);
	connect(d->act_send,SIGNAL(triggered()), SLOT(mle_returnPressed()));
 	ui_.pb_send->setIcon(IconsetFactory::icon("psi/action_button_send").icon());
	connect(ui_.pb_send, SIGNAL(clicked()), SLOT(mle_returnPressed()));
	connect(ui_.pb_send, SIGNAL(customContextMenuRequested(const QPoint)), SLOT(sendButtonMenu()));
	d->act_close = new QAction(this);
	addAction(d->act_close);
	connect(d->act_close,SIGNAL(triggered()), SLOT(close()));
	d->act_hide = new QAction(this);
	addAction(d->act_hide);
	connect(d->act_hide,SIGNAL(triggered()), SLOT(hideTab()));
	d->act_scrollup = new QAction(this);
	addAction(d->act_scrollup);
	connect(d->act_scrollup,SIGNAL(triggered()), SLOT(scrollUp()));
	d->act_scrolldown = new QAction(this);
	addAction(d->act_scrolldown);
	connect(d->act_scrolldown,SIGNAL(triggered()), SLOT(scrollDown()));

	ui_.mini_prompt->hide();
	connect(ui_.mle, SIGNAL(textEditCreated(QTextEdit*)), SLOT(chatEditCreated()));
	chatEditCreated();
	ui_.log->init(); //here is we ready to do that

	d->pm_settings = new QMenu(this);
	connect(d->pm_settings, SIGNAL(aboutToShow()), SLOT(buildMenu()));
	ui_.tb_actions->setMenu(d->pm_settings);
	ui_.tb_actions->setIcon(IconsetFactory::icon("psi/select").icon());
	ui_.tb_actions->setStyleSheet(" QToolButton::menu-indicator { image:none } ");

	connect(ui_.hsplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(horizSplitterMoved(int,int)));
	connect(ui_.vsplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(verticalSplitterMoved(int,int)));

	// resize the horizontal splitter
	d->logSize = PsiOptions::instance()->getOption("options.ui.muc.log-width").toInt();
	d->rosterSize = PsiOptions::instance()->getOption("options.ui.muc.roster-width").toInt();
	QList<int> list;
	bool leftRoster = PsiOptions::instance()->getOption("options.ui.muc.roster-at-left").toBool();
	if(leftRoster)
		list << d->rosterSize << d->logSize;
	else
		list <<  d->logSize << d->rosterSize;

	ui_.hsplitter->setSizes(list);

	if (leftRoster)
		ui_.hsplitter->insertWidget(0, ui_.lv_users);  // Swap widgets

	// resize the vertical splitter
	d->logHeight = PsiOptions::instance()->getOption("options.ui.chat.log-height").toInt();
	d->chateditHeight = PsiOptions::instance()->getOption("options.ui.chat.chatedit-height").toInt();
	setVSplitterPosition(d->logHeight, d->chateditHeight);

	X11WM_CLASS("groupchat");

	ui_.mle->chatEdit()->setFocus();
	if(PsiOptions::instance()->getOption("options.ui.remember-window-sizes").toBool())
		setInitSize(PsiOptions::instance()->getOption("options.ui.muc.size").toSize(),
			   PsiOptions::instance()->getOption("options.ui.muc.x").toInt(),
			    PsiOptions::instance()->getOption("options.ui.muc.y").toInt());

	// Connect signals from MUC manager
	connect(d->mucManager,SIGNAL(action_error(MUCManager::Action, int, const QString&)), SLOT(action_error(MUCManager::Action, int, const QString&)));
	connect(d->mucManager, SIGNAL(action_success(MUCManager::Action)), ui_.lv_users, SLOT(update()));

	setLooks();
	setShortcuts();
	invalidateTab();
	setConnecting();
	connect(URLObject::getInstance(), SIGNAL(vcardsig(QString)), SLOT(vcard(QString)));

	connect(ui_.log->textWidget(), SIGNAL(quote(const QString &)), ui_.mle->chatEdit(), SLOT(insertPlainText(const QString &)));
}

void GCMainDlg::vcard(QString str){
	str.remove(0, str.indexOf(":")+1);
	XMPP::Jid jd(str);
	account()->actionInfo(jd);
}
GCMainDlg::~GCMainDlg()
{
	//  Save splitters size
	PsiOptions::instance()->setOption("options.ui.muc.log-width", d->logSize);
	PsiOptions::instance()->setOption("options.ui.muc.roster-width", d->rosterSize);

	if(d->state != Private::Idle && d->state != Private::ForcedLeave) {
		account()->groupChatLeave(jid().domain(), jid().node());
	}

	//QMimeSourceFactory *m = ui_.log->mimeSourceFactory();
	//ui_.log->setMimeSourceFactory(0);
	//delete m;

	account()->dialogUnregister(this);
	delete d->mucManager;
	delete d;
}

void GCMainDlg::horizSplitterMoved(int/* pos*/, int /*index*/)
{
	QList<int> list = ui_.hsplitter->sizes();
	bool leftRoster = PsiOptions::instance()->getOption("options.ui.muc.roster-at-left").toBool();
	d->rosterSize = !leftRoster ? list.last() : list.first();
	d->logSize = leftRoster ? list.last() : list.first();

	PsiOptions::instance()->setOption("options.ui.muc.log-width", d->logSize);
	PsiOptions::instance()->setOption("options.ui.muc.roster-width", d->rosterSize);
}

void GCMainDlg::verticalSplitterMoved(int, int)
{
	QList<int> list = ui_.vsplitter->sizes();
	d->logHeight = list.first();
	d->chateditHeight = list.last();
	PsiOptions::instance()->setOption("options.ui.chat.log-height", d->logHeight);
	PsiOptions::instance()->setOption("options.ui.chat.chatedit-height", d->chateditHeight);

	emit vSplitterMoved(d->logHeight, d->chateditHeight);
}

void GCMainDlg::setVSplitterPosition(int log,int chat)
{
	QList<int> list;
	d->logHeight = log;
	d->chateditHeight = chat;
	list << log << chat;
	ui_.vsplitter->setSizes(list);
}

void GCMainDlg::doMinimize()
{
	window()->showMinimized();
}

void GCMainDlg::ensureTabbedCorrectly() {
	TabbableWidget::ensureTabbedCorrectly();
	setShortcuts();
	// QSplitter is broken again, force resize so that
	// lv_users gets initizalised properly and context menu
	// works in tabs too.
	d->logSize = PsiOptions::instance()->getOption("options.ui.muc.log-width").toInt();
	d->rosterSize = PsiOptions::instance()->getOption("options.ui.muc.roster-width").toInt();
	QList<int> list;
	if(PsiOptions::instance()->getOption("options.ui.muc.roster-at-left").toBool())
		list << d->rosterSize << d->logSize;
	else
		list <<  d->logSize << d->rosterSize;
	ui_.hsplitter->setSizes(QList<int>() << 0);
	ui_.hsplitter->setSizes(list);
	setStatusTabIcon((d->state == Private::Connected ? STATUS_ONLINE : STATUS_OFFLINE));
}

void GCMainDlg::setShortcuts()
{
	d->act_clear->setShortcuts(ShortcutManager::instance()->shortcuts("chat.clear"));
	d->act_find->setShortcuts(ShortcutManager::instance()->shortcuts("chat.find"));
	d->act_send->setShortcuts(ShortcutManager::instance()->shortcuts("chat.send"));
	if (!isTabbed()) {
		d->act_close->setShortcuts(ShortcutManager::instance()->shortcuts("common.close"));
	} else {
		d->act_close->QAction::setShortcuts (QList<QKeySequence>());
	}
	d->act_hide->setShortcuts(ShortcutManager::instance()->shortcuts("common.hide"));
	d->act_scrollup->setShortcuts(ShortcutManager::instance()->shortcuts("common.scroll-up"));
	d->act_scrolldown->setShortcuts(ShortcutManager::instance()->shortcuts("common.scroll-down"));
	d->act_mini_cmd->setShortcuts(ShortcutManager::instance()->shortcuts("chat.quick-command"));
	d->act_minimize->setShortcuts(ShortcutManager::instance()->shortcuts("chat.minimize"));
}

void GCMainDlg::scrollUp() {
	ui_.log->scrollUp();
}

void GCMainDlg::scrollDown() {
	ui_.log->scrollDown();
}

void GCMainDlg::closeEvent(QCloseEvent *e)
{
	PsiOptions::instance()->setOption("options.ui.muc.x", window()->x());
	PsiOptions::instance()->setOption("options.ui.muc.y", window()->y());
	PsiOptions::instance()->setOption("options.ui.muc.size", window()->size());
	if (isBorder()) {
		PsiOptions::instance()->setOption("options.ui.muc.maximized",
					  (bool(window()->windowState() & Qt::WindowFullScreen)
					   || bool(window()->windowState() & Qt::WindowMaximized)));
	}
	else {
		PsiOptions::instance()->setOption("options.ui.muc.maximized", bool(window()->geometry() == qApp->desktop()->availableGeometry(-1)));
	}
	e->accept();
	if (d->state != Private::Connected)
		account()->groupChatLeave(d->dlg->jid().domain(),d->dlg->jid().node());
}

void GCMainDlg::resizeEvent(QResizeEvent* e)
{
	if (!(bool(window()->windowState() & Qt::WindowFullScreen)
		|| bool(window()->windowState() & Qt::WindowMaximized))){
		if (PsiOptions::instance()->getOption("options.ui.remember-window-sizes").toBool())
			PsiOptions::instance()->setOption("options.ui.muc.size", e->size());
	}
}

void GCMainDlg::deactivated()
{
	TabbableWidget::deactivated();

	d->trackBar = true;
}

void GCMainDlg::activated()
{
	TabbableWidget::activated();
	setState(StateNone);

	if(d->pending > 0) {
		d->pending = 0;
		d->hPending = 0;
		UserListItem* u = account()->find(d->dlg->jid().bare());
		if (u) {
			u->setPending(d->pending, d->hPending);
			account()->updateEntry(*u);
		}
		invalidateTab();
	}
	doFlash(false);

	ui_.mle->chatEdit()->setFocus();
	d->trackBar = false;
}

void GCMainDlg::mucInfoDialog(const QString& title, const QString& message, const Jid& actor, const QString& reason)
{
	QString m = message;
	
	if (!actor.isEmpty())
		m += tr(" by %1").arg(actor.full());
	m += ".";
	
	if (!reason.isEmpty())
		m += tr("\nReason: %1").arg(reason);

	// FIXME maybe this should be queued in the future?
	QMessageBox* msg = new QMessageBox(QMessageBox::Information, title, m, QMessageBox::Ok, this);
	msg->setAttribute(Qt::WA_DeleteOnClose, true);
	msg->setModal(false);
	msg->show();
}

void GCMainDlg::logSelectionChanged()
{
#ifdef Q_WS_MAC
	// A hack to only give the message log focus when text is selected
// seems its already useless. at least copy works w/o this hack
//	if (ui_.log->textCursor().hasSelection())
//		ui_.log->setFocus();
//	else
//		ui_.mle->chatEdit()->setFocus();
#endif
}

void GCMainDlg::setConnecting()
{
	d->connecting = true;
	QTimer::singleShot(5000,this,SLOT(unsetConnecting()));
}

void GCMainDlg::updateIdentityVisibility()
{
	ui_.lb_ident->setVisible(account()->psi()->contactList()->enabledAccounts().count() > 1);
}

void GCMainDlg::updateBookmarkIcon()
{
	BookmarkManager *bm = account()->bookmarkManager();
	d->act_bookmark->setVisible(bm->isAvailable());
	if (bm->isAvailable()) {
		QString text;
		if (bm->isBookmarked(jid())) {
			text = tr("Edit bookmark");
			d->act_bookmark->setPsiIcon("psi/bookmark_remove");
		} else {
			text = tr("Add to bookmarks");
			d->act_bookmark->setPsiIcon("psi/bookmark_add");
		}
		d->act_bookmark->setToolTip(text);
		d->act_bookmark->setText(text);
		d->act_bookmark->setStatusTip(text);
	}
}

#ifdef WHITEBOARDING
void GCMainDlg::openWhiteboard()
{
	account()->actionOpenWhiteboardSpecific(jid(), jid().withResource(d->self), true);
}
#endif

void GCMainDlg::unsetConnecting()
{
	d->connecting = false;
}

void GCMainDlg::action_error(MUCManager::Action, int, const QString& err) 
{
	appendSysMsg(err, false);
}


void MiniCommand_Depreciation_Message(const QString &old,const QString &newCmd, QString &line1, QString &line2) {
	line1 = QObject::tr("Warning: %1 is deprecated and will be removed in the future").arg(old);
	QList<QKeySequence> keys = ShortcutManager::instance()->shortcuts("chat.quick-command");
	if (keys.isEmpty()) {
		line2 = QObject::tr("Please set a shortcut for 'Change to quick command mode', use that shortcut and enter '%1'.").arg(newCmd);
	} else {
		line2 = QObject::tr("Please instead press %1 and enter '%2'.").arg(keys.at(0).toString(), newCmd);
	}
}


void GCMainDlg::mle_returnPressed()
{
	d->tabCompletion.reset();
	QString str = d->mle()->toPlainText();

	if (d->mCmdSite.isActive()) {
		if (!d->mCmdManager.processCommand(str)) {
			appendSysMsg(tr("Error: Cannot parse command: ") + str, false);
		}
		return;
	}

	if(str.isEmpty())
		return;

	if(str == "/clear") {
		doClear();

		d->histAt = 0;
		d->hist.prepend(str);
		ui_.mle->chatEdit()->setText("");

		QString line1,line2;
		MiniCommand_Depreciation_Message("/clear", "clear", line1, line2);
		appendSysMsg(line1, false);
		appendSysMsg(line2, false);
		return;
	}

	if(str.toLower().startsWith("/nick ")) {
		QString nick = str.mid(6).trimmed();
    XMPP::Jid newJid = jid().withResource(nick);
		if (!nick.isEmpty() && newJid.isValid()) {
			d->prev_self = d->self;
			d->self = newJid.resource();
			account()->groupChatChangeNick(jid().domain(), jid().node(), d->self, account()->status());
		}
		ui_.mle->chatEdit()->setText("");
		QString line1,line2;
		MiniCommand_Depreciation_Message("/nick", "nick", line1, line2);
		appendSysMsg(line1, false);
		appendSysMsg(line2, false);
		return;
	}

	if(d->state != Private::Connected)
		return;

	Message m(jid());
	m.setType("groupchat");
	m.setBody(str);

	HTMLElement html = d->mle()->toHTMLElement();
	if(!html.body().isNull())
		m.setHTML(html);

	m.setTimeStamp(QDateTime::currentDateTime());
	emit d->mle()->appendMessageHistory(m.body());

	aSend(m);

	d->histAt = 0;
	d->hist.prepend(str);
	ui_.mle->chatEdit()->setText("");
}

/*void GCMainDlg::le_upPressed()
{
	if(d->histAt < (int)d->hist.count()) {
		++d->histAt;
		d->le_input->setText(d->hist[d->histAt-1]);
	}
}

void GCMainDlg::le_downPressed()
{
	if(d->histAt > 0) {
		--d->histAt;
		if(d->histAt == 0)
			d->le_input->setText("");
		else
			d->le_input->setText(d->hist[d->histAt-1]);
	}
}*/

void GCMainDlg::openTopic()
{
	if(d->topicDlg) {
		::bringToFront(d->topicDlg);
	} else {
		d->topicDlg = new GroupchatTopicDlg(this);
		d->topicDlg->setAttribute(Qt::WA_DeleteOnClose);
		d->topicDlg->show();
		connect(d->topicDlg, SIGNAL(topicAccepted(const QString)), this, SLOT(setTopic(const QString)));
	}
}

void GCMainDlg::setTopic(const QString &topic)
{
	Message m(jid());
	m.setType("groupchat");
	m.setSubject(topic);
	m.setTimeStamp(QDateTime::currentDateTime());
	aSend(m);
}

void GCMainDlg::doClear()
{
	ui_.log->clear();
}

void GCMainDlg::doClearButton()
{
	if (PsiOptions::instance()->getOption("options.ui.chat.warn-before-clear").toBool()) {
		switch (
			QMessageBox::warning(
				this,
				tr("Warning"),
				tr("Are you sure you want to clear the chat window?\n(note: does not affect saved history)"),
				QMessageBox::Yes, QMessageBox::YesAll, QMessageBox::No
			)
		) {
		case QMessageBox::No:
		break;
		case QMessageBox::YesAll:
			PsiOptions::instance()->setOption("options.ui.chat.warn-before-clear", false);
			// fall-through
		case QMessageBox::Yes:
			doClear();
		}
	} else {
		doClear();
	}
}
/*
void GCMainDlg::openFind()
{
	if(d->findDlg)
		::bringToFront(d->findDlg);
	else {
		d->findDlg = new GCFindDlg(d->lastSearch, this);
		connect(d->findDlg, SIGNAL(find(const QString &)), SLOT(doFind(const QString &)));
		d->findDlg->show();
	}
}
*/
void GCMainDlg::configureRoom()
{
	if(d->configDlg)
		::bringToFront(d->configDlg);
	else {
		GCUserViewItem* c = (GCUserViewItem*)ui_.lv_users->findEntry(d->self);
		MUCItem::Role role = c ? c->s.mucItem().role() : MUCItem::UnknownRole;
		MUCItem::Affiliation affiliation = c ? c->s.mucItem().affiliation() : MUCItem::UnknownAffiliation;
		d->configDlg = new MUCConfigDlg(d->mucManager, this);
		d->configDlg->setRoleAffiliation(role, affiliation);
		d->configDlg->show();
	}
}
/*
void GCMainDlg::doFind(const QString &str)
{
	d->lastSearch = str;
	if (d->te_log()->internalFind(str))
		d->findDlg->found();
	else
		d->findDlg->error(str);
}
*/
void GCMainDlg::goDisc()
{
	if(d->state != Private::Idle && d->state != Private::ForcedLeave) {
		d->state = Private::Idle;
		ui_.pb_topic->setEnabled(false);
		setStatusTabIcon(STATUS_OFFLINE);
		appendSysMsg(tr("Disconnected."), true);
		ui_.mle->chatEdit()->setEnabled(false);
	}
}

// kick, ban, removed muc, etc
void GCMainDlg::goForcedLeave() {
	if(d->state != Private::Idle && d->state != Private::ForcedLeave) {
		goDisc();
		account()->groupChatLeave(jid().domain(), jid().node());
		d->state = Private::ForcedLeave;
	}
}

bool GCMainDlg::isInactive() const {
	return d->state == Private::ForcedLeave;
}

void GCMainDlg::reactivate() {
	d->state = Private::Idle;
	goConn();
}


void GCMainDlg::goConn()
{
	if(d->state == Private::Idle) {
		d->state = Private::Connecting;
		appendSysMsg(tr("Reconnecting..."), true);

		QString host = jid().domain();
		QString room = jid().node();
		QString nick = d->self;

		if(!account()->groupChatJoin(host, room, nick, d->password)) {
			appendSysMsg(tr("Error: You are in or joining this room already!"), true);
			d->state = Private::Idle;
		}
	}
}

void GCMainDlg::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasText())
		e->accept();
}

void GCMainDlg::dropEvent(QDropEvent *e)
{
	Jid jid(e->mimeData()->text());
	if (jid.isValid() && !ui_.lv_users->hasJid(jid)) {
		Message m;
		m.setTo(this->jid());
		m.addMUCInvite(MUCInvite(jid));
		if (!d->password.isEmpty())
			m.setMUCPassword(d->password);
		m.setTimeStamp(QDateTime::currentDateTime());
		account()->dj_sendMessage(m);
	}
}


void GCMainDlg::pa_updatedActivity()
{
	if(!account()->loggedIn()) {
		goDisc();
	}
	else {
		if(d->state == Private::Idle) {
			goConn();
		}
		else if(d->state == Private::Connected) {
			Status s = account()->status();
			s.setXSigned("");
			account()->groupChatSetStatus(jid().domain(), jid().node(), s);
		}
	}
}

PsiAccount* GCMainDlg::account() const
{
	return TabbableWidget::account();
}

void GCMainDlg::error(int, const QString &str)
{
	ui_.pb_topic->setEnabled(false);
	setStatusTabIcon(STATUS_ERROR);

	if(d->state == Private::Connecting)
		appendSysMsg(tr("Unable to join groupchat.	Reason: %1").arg(str), true);
	else
		appendSysMsg(tr("Unexpected groupchat error: %1").arg(str), true);

	d->state = Private::Idle;
}


void GCMainDlg::mucKickMsgHelper(const QString &nick, const Status &s, const QString &nickJid, const QString &title,
			const QString &youSimple, const QString &youBy, const QString &someoneSimple,
			const QString &someoneBy) {
	QString message;
	if (nick == d->self) {
		message = youSimple;
		mucInfoDialog(title, message, s.mucItem().actor(), s.mucItem().reason());
		if (!s.mucItem().actor().isEmpty()) {
			message = youBy.arg(s.mucItem().actor().full());
		}
		goForcedLeave();
	} else if (!s.mucItem().actor().isEmpty()) {
		message = someoneBy.arg(nickJid, s.mucItem().actor().full());
	} else {
		message = someoneSimple.arg(nickJid);
	}

	if (!s.mucItem().reason().isEmpty()) {
		message += QString(" (%1)").arg(s.mucItem().reason());
	}
	appendSysMsg(message);
}

void GCMainDlg::presence(const QString &nick, const Status &s)
{
	if(s.hasError()) {
		QString message;
		if (s.errorCode() == 409) {
			message = tr("Please choose a different nickname");
			d->self = d->prev_self;
		}
		else {
			message = tr("An error occurred (errorcode: %1)").arg(s.errorCode());
		}
		appendSysMsg(message);
		return;
	}

	if ((nick.isEmpty()) && (s.getMUCStatuses().contains(100))) {
		d->nonAnonymous = true;
	}

	if (nick == d->self) {
		if(s.isAvailable()) setStatusTabIcon(s.type());
		UserListItem* u = account()->find(d->dlg->jid().bare());
		if(u) {
			Resource r;
			r.setName("Muc");
			r.setStatus(s);
			u->userResourceList().replace(0, r);
			account()->updateEntry(*u);
		}
		// Update configuration dialog
		if (d->configDlg) {
			d->configDlg->setRoleAffiliation(s.mucItem().role(),s.mucItem().affiliation());
		}
		d->act_configure->setEnabled(s.mucItem().affiliation() >= MUCItem::Member);
	}
	

	if(s.isAvailable()) {
		// Available
		if (s.getMUCStatuses().contains(201)) {
			appendSysMsg(tr("New room created"));
			if (options_->getOption("options.muc.accept-defaults").toBool()) {
				d->mucManager->setDefaultConfiguration();
			} else if (options_->getOption("options.muc.auto-configure").toBool()) {
				QTimer::singleShot(0, this, SLOT(configureRoom()));
			}
		}

		GCUserViewItem* contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		if (contact == NULL) {
			//contact joining
			if ((!d->connecting || options_->getOption("options.ui.muc.show-initial-joins").toBool()) && options_->getOption("options.muc.show-joins").toBool() ) {
				QString message = tr("%1 has joined the room");
				if ( options_->getOption("options.muc.show-role-affiliation").toBool() ) {
					if (s.mucItem().role() != MUCItem::NoRole) {
						if (s.mucItem().affiliation() != MUCItem::NoAffiliation) {
							message = tr("%3 has joined the room as %1 and %2").arg(MUCManager::roleToString(s.mucItem().role(),true)).arg(MUCManager::affiliationToString(s.mucItem().affiliation(),true));
						}
						else {
							message = tr("%2 has joined the room as %1").arg(MUCManager::roleToString(s.mucItem().role(),true));
						}
					}
					else if (s.mucItem().affiliation() != MUCItem::NoAffiliation) {
						message = tr("%2 has joined the room as %1").arg(MUCManager::affiliationToString(s.mucItem().affiliation(),true));
					}
				}
				if (!s.mucItem().jid().isEmpty()) {
					message = message.arg(QString("%1 (%2)").arg(nick).arg(s.mucItem().jid().full()));
				} else {
					message = message.arg(nick);
				}
				if (options_->getOption("options.muc.show-status-changes").toBool()) {
					message += tr(" and now is %1").arg(status2txt(s.type()));
					if (!s.status().isEmpty()) {
						message += QString(" (%1)").arg(s.status());
					}
					if (options_->getOption("options.ui.muc.status-with-priority").toBool() && s.priority() != 0) {
						message += QString(" [%1]").arg(s.priority());
					}
				}
				appendSysMsg(message);
			}
		}
		else {
			// Status change
			if ( !d->connecting && options_->getOption("options.muc.show-role-affiliation").toBool() ) {
				QString message;
				if (contact->s.mucItem().role() != s.mucItem().role() && s.mucItem().role() != MUCItem::NoRole) {
					if (contact->s.mucItem().affiliation() != s.mucItem().affiliation()) {
						message = tr("%1 is now %2 and %3").arg(nick).arg(MUCManager::roleToString(s.mucItem().role(),true)).arg(MUCManager::affiliationToString(s.mucItem().affiliation(),true));
					}
					else {
						message = tr("%1 is now %2").arg(nick).arg(MUCManager::roleToString(s.mucItem().role(),true));
					}
				}
				else if (contact->s.mucItem().affiliation() != s.mucItem().affiliation()) {
					message += tr("%1 is now %2").arg(nick).arg(MUCManager::affiliationToString(s.mucItem().affiliation(),true));
				}

				if (!message.isEmpty()) {
					appendSysMsg(message);
				}
			}
			if ( !d->connecting && options_->getOption("options.muc.show-status-changes").toBool() ) {
				bool statusWithPriority = PsiOptions::instance()->getOption("options.ui.muc.status-with-priority").toBool();
				if (s.status() != contact->s.status() || s.show() != contact->s.show() || (statusWithPriority && s.priority() != contact->s.priority())) {
					QString message;
					message = tr("%1 is now %2").arg(nick).arg(status2txt(s.type()));
					if (!s.status().isEmpty()) {
						message += QString(" (%1)").arg(s.status());
					}
					if (statusWithPriority && s.priority() != 0) {
						message += QString(" [%1]").arg(s.priority());
					}
					appendSysMsg(message);
				}
			}
		}
		ui_.lv_users->updateEntry(nick, s);
	} 
	else {
		// Unavailable
		if (s.hasMUCDestroy()) {
			// Room was destroyed
			QString message = tr("This room has been destroyed.");
			QString log = message;
			if (!s.mucDestroy().reason().isEmpty()) {
				message += "\n";
				QString reason = tr("Reason: %1").arg(s.mucDestroy().reason());
				message += reason;
				log += " " + reason;
			}
			if (!s.mucDestroy().jid().isEmpty()) {
				message += "\n";
				message += tr("Do you want to join the alternate venue '%1'?").arg(s.mucDestroy().jid().full());
				int ret = QMessageBox::information(this, tr("Room Destroyed"), message, QMessageBox::Yes, QMessageBox::No);
				if (ret == QMessageBox::Yes) {
					account()->actionJoin(s.mucDestroy().jid().full());
				}
			}
			else {
				QMessageBox::information(this,tr("Room Destroyed"), message);
			}
			appendSysMsg(log);
			goForcedLeave();
		}

		QString message;
		QString nickJid;
		GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		if (contact && !contact->s.mucItem().jid().isEmpty()) {
			nickJid = QString("%1 (%2)").arg(nick).arg(contact->s.mucItem().jid().full());
		} else {
			nickJid = nick;
		}

		bool suppressDefault = false;

		if (s.getMUCStatuses().contains(301)) {
			// Ban
			mucKickMsgHelper(nick, s, nickJid, tr("Banned"), tr("You have been banned from the room"),
						 tr("You have been banned from the room by %1"),
						 tr("%1 has been banned"),
						 tr("%1 has been banned by %2"));
			suppressDefault = true;
		}
		if (s.getMUCStatuses().contains(307)) {
			// Kick
			mucKickMsgHelper(nick, s, nickJid, tr("Kicked"), tr("You have been kicked from the room"),
						  tr("You have been kicked from the room by %1"),
						  tr("%1 has been kicked"),
						  tr("%1 has been kicked by %2"));
			suppressDefault = true;
		}
		if (s.getMUCStatuses().contains(321)) {
			// Remove due to affiliation change
			mucKickMsgHelper(nick, s, nickJid, tr("Removed"),
						 tr("You have been removed from the room due to an affiliation change"),
						 tr("You have been removed from the room by %1 due to an affiliation change"),
						 tr("%1 has been removed from the room due to an affilliation change"),
						 tr("%1 has been removed from the room by %2 due to an affilliation change"));
			suppressDefault = true;
		}
		if (s.getMUCStatuses().contains(322)) {
			mucKickMsgHelper(nick, s, nickJid, tr("Removed"),
						 tr("You have been removed from the room because the room was made members only"),
						 tr("You have been removed from the room by %1 because the room was made members only"),
						 tr("%1 has been removed from the room because the room was made members-only"),
						 tr("%1 has been removed from the room by %2 because the room was made members-only"));
			suppressDefault = true;
		}

		if ( !d->connecting && !suppressDefault && options_->getOption("options.muc.show-joins").toBool() ) {
			if (s.getMUCStatuses().contains(303)) {
				message = tr("%1 is now known as %2").arg(nick).arg(s.mucItem().nick());
				ui_.lv_users->updateEntry(s.mucItem().nick(), s);
			} else {
				//contact leaving
				message = tr("%1 has left the room").arg(nickJid);
				if (!s.status().isEmpty()) {
					message += QString(" (%1)").arg(s.status());
				}
			}
			appendSysMsg(message);
		}
		ui_.lv_users->removeEntry(nick);
	}
	
	if (!s.capsNode().isEmpty()) {
		Jid caps_jid(s.mucItem().jid().isEmpty() || !d->nonAnonymous ? Jid(jid()).withResource(nick) : s.mucItem().jid());
		account()->capsManager()->updateCaps(caps_jid,s.capsNode(),s.capsVersion(),s.capsExt());
	}

}

void GCMainDlg::message(const Message &_m)
{
	Message m = _m;
	QString from = m.from().resource();
	bool alert = false;

	if (m.getMUCStatuses().contains(100)) {
		d->nonAnonymous = true;
	}
	if (m.getMUCStatuses().contains(172)) {
		d->nonAnonymous = true;
	}
	if (m.getMUCStatuses().contains(173)) {
		d->nonAnonymous = false;
	}
	if (m.getMUCStatuses().contains(174)) {
		d->nonAnonymous = false;
	}

	if(!m.subject().isNull()) {
		QString subject = m.subject();
		d->topic = subject;
		QString subjectTooltip = TextUtil::plain2rich(subject);
		subjectTooltip = TextUtil::linkify(subjectTooltip);
		if(PsiOptions::instance()->getOption("options.ui.emoticons.use-emoticons").toBool()) {
			subjectTooltip = TextUtil::emoticonify(subjectTooltip);
		}
		ui_.le_topic->setText(subject.replace("\n\n", " || ").replace("\n", " | ").replace("\t", " ").replace(QRegExp("\\s{2,}"), " "));
		ui_.le_topic->setCursorPosition(0);
		ui_.le_topic->setToolTip(QString("<qt><p>%1</p></qt>").arg(subjectTooltip));

		QString sysMsg;
		if (from.isEmpty()) {
			// The topic was set by the server
			// ugly trick
			int btStart = m.body().indexOf(d->topic);
			sysMsg = btStart > 0?m.body().left(btStart).remove(": "):tr("The topic has been set to");
		} else {
			sysMsg = QString(from) + (d->topic.isEmpty()?
						 tr(" has unset the topic"):tr(" has set the topic to"));
		}
		MessageView tv = MessageView::subjectMessage(d->topic, sysMsg);
		tv.setDateTime(m.timeStamp());
		appendSysMsg(tv);
		//ui_.log->dispatchMessage(tv);
		return;
	}

	if(m.body().isEmpty())
		return;

	// code to determine if the speaker was addressing this client in chat
	if(m.body().contains(d->self))
		alert = true;

	if (m.body().left(d->self.length()) == d->self)
		d->lastReferrer = m.from().resource();

	if(PsiOptions::instance()->getOption("options.ui.muc.use-highlighting").toBool()) {
		QStringList highlightWords = PsiOptions::instance()->getOption("options.ui.muc.highlight-words").toStringList();
		foreach (QString word, highlightWords) {
			if(m.body().contains((word), Qt::CaseInsensitive)) {
				alert = true;
			}
		}
	}

	// play sound?
	if(from == d->self) {
		if(!m.spooled())
			account()->playSound(PsiAccount::eSend);
	}
	else {
		if(alert || (PsiOptions::instance()->getOption("options.ui.notifications.sounds.notify-every-muc-message").toBool() && !m.spooled() && !from.isEmpty()) )
			account()->playSound(PsiAccount::eChat2);
		if(alert || (PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.notify-every-muc-message").toBool() && !m.spooled() && !from.isEmpty()) )
			if (!m.spooled() && !isActiveTab() && !m.from().resource().isEmpty()) {
				PsiPopup *popup = new PsiPopup(PsiPopup::AlertGcHighlight, account());
				XMPP::Jid jid = m.from();
				jid.setDomain("");
				const UserListItem u;
				const MessageEvent *e = new MessageEvent(m, account());
				popup->setData(jid, m.from().resource(), &u, (PsiEvent *)e);
			}
	}

	if(from.isEmpty())
		appendSysMsg(m.body(), alert, m.timeStamp());
	else
		appendMessage(m, alert);
}

void GCMainDlg::joined()
{
	if(d->state == Private::Connecting) {
		ui_.lv_users->clear();
		d->state = Private::Connected;
		ui_.pb_topic->setEnabled(true);
		setStatusTabIcon(STATUS_ONLINE);
		ui_.mle->chatEdit()->setEnabled(true);
		setConnecting();
		appendSysMsg(tr("Connected."), true);
	}
	account()->addMucItem(d->dlg->jid().bare());
}

void GCMainDlg::setPassword(const QString& p)
{
	d->password = p;
}

const QString& GCMainDlg::nick() const
{
	return d->self;
}

const QString& GCMainDlg::topic() const
{
	return d->topic;
}

void GCMainDlg::appendSysMsg(const QString &str, bool alert, const QDateTime &ts)
{
	MessageView mv = MessageView::fromPlainText(str, MessageView::System);
	if (!PsiOptions::instance()->getOption("options.ui.muc.use-highlighting").toBool()) {
		alert = false;
	}
	mv.setAlert(alert);
	if(!ts.isNull()) {
		mv.setDateTime(ts);
	}
	appendSysMsg(mv);
}

void GCMainDlg::appendSysMsg(const MessageView &mv)
{
	if (d->trackBar)
		d->doTrackBar();

	ui_.log->dispatchMessage(mv);

	if(mv.isAlert())
		doAlert();
}

void GCMainDlg::appendMessage(const Message &m, bool alert)
{
	MessageView mv(MessageView::Message);
	if (m.containsHTML() && PsiOptions::instance()
							->getOption("options.html.muc.render").toBool() &&
							!m.html().text().isEmpty()) {
		mv.setHtml(m.html().toString("span"));
	} else {
		mv.setPlainText(m.body());
	}
	if (!PsiOptions::instance()->getOption("options.ui.muc.use-highlighting").toBool())
		alert=false;
	mv.setAlert(alert);
	mv.setUserId(m.from().full());
	mv.setNick(m.from().resource());
	mv.setLocal(mv.nick() == d->self);
	mv.setSpooled(m.spooled());
	mv.setDateTime(m.timeStamp());

	if (d->trackBar && !mv.isLocal() && !mv.isSpooled()) {
		d->doTrackBar();
	}

	ui_.log->dispatchMessage(mv);

	// if we're not active, notify the user by changing the title
	if(!isActiveTab() && !mv.isLocal()) {
		setState(alert ? TabbableWidget::StateUnreadToMe : TabbableWidget::StateUnread);

		++d->pending;
		if(alert) ++d->hPending;
		UserListItem* u = account()->find(d->dlg->jid().bare());
		if (u) {
			u->setPending(d->pending, d->hPending);
			account()->updateEntry(*u);
		}
		invalidateTab();
	}

	//if someone directed their comments to us, notify the user
	if(alert)
		doAlert();

	//if the message spoke to us, alert the user before closing this window
	//except that keepopen doesn't seem to be implemented for this class yet.
	/*if(alert) {
		d->keepOpen = true;
		QTimer::singleShot(1000, this, SLOT(setKeepOpenFalse()));
				}*/
}

void GCMainDlg::doAlert()
{
	if(!isActiveTab())
		if (PsiOptions::instance()->getOption("options.ui.flash-windows").toBool())
			doFlash(true);
}

QString GCMainDlg::desiredCaption() const
{
	QString cap = "";

	if (d->pending > 0) {
		cap += "* ";
		if (d->pending > 1) {
			cap += QString("[%1] ").arg(d->pending);
		}
	}
	cap += jid().full();

	return cap;
}

void GCMainDlg::setLooks()
{
	const QString css = PsiOptions::instance()->getOption("options.ui.chat.css").toString();
	if (!css.isEmpty()) {
		setStyleSheet(css);
		d->mle()->setCssString(css);
	}
	ui_.vsplitter->optionsChanged();
	ui_.mle->optionsChanged();

	// update the fonts
	QFont f;
	f.fromString(PsiOptions::instance()->getOption("options.ui.look.font.chat").toString());
	ui_.log->setFont(f);
	ui_.mle->chatEdit()->setFont(f);

	f.fromString(PsiOptions::instance()->getOption("options.ui.look.font.contactlist").toString());
	ui_.lv_users->setFont(f);

	if (PsiOptions::instance()->getOption("options.ui.chat.central-toolbar").toBool()) {
		ui_.toolbar->show();
		ui_.tb_actions->hide();
		ui_.tb_emoticons->hide();
	}
	else {
		ui_.toolbar->hide();
		ui_.tb_emoticons->setVisible(PsiOptions::instance()->getOption("options.ui.emoticons.use-emoticons").toBool());
		ui_.tb_actions->show();
	}

	if (PsiOptions::instance()->getOption("options.ui.disable-send-button").toBool()) {
		ui_.pb_send->hide();
	}
	else {
		ui_.pb_send->show();
	}

	setWindowOpacity(double(qMax(MINIMUM_OPACITY,PsiOptions::instance()->getOption("options.ui.chat.opacity").toInt()))/100);

	// update the widget icon
#ifndef Q_WS_MAC
	setWindowIcon(IconsetFactory::icon("psi/groupChat").icon());
#endif
}

void GCMainDlg::optionsUpdate()
{
	/*QMimeSourceFactory *m = ui_.log->mimeSourceFactory();
	ui_.log->setMimeSourceFactory(PsiIconset::instance()->emoticons.generateFactory());
	delete m;*/

	setLooks();
	setShortcuts();
	d->typeahead->optionsUpdate();
	// update status icons
	ui_.lv_users->updateAll();
}

void GCMainDlg::lv_action(const QString &nick, const Status &s, int x)
{
	if(x == 0) {
		account()->invokeGCMessage(jid().withResource(nick));
	}
	else if(x == 1) {
		account()->invokeGCChat(jid().withResource(nick));
	}
	else if(x == 2) {
		UserListItem u;
		u.setJid(jid().withResource(nick));
		u.setName(nick);

		// make a resource so the contact appears online
		UserResource ur;
		ur.setName(nick);
		ur.setStatus(s);
		u.userResourceList().append(ur);

		StatusShowDlg *w = new StatusShowDlg(u);
		w->show();
	}
	else if(x == 3) {
		account()->invokeGCInfo(jid().withResource(nick));
	}
	else if(x == 4) {
		account()->invokeGCFile(jid().withResource(nick));
	}
	else if(x == 5) {
		account()->actionVoice(jid().withResource(nick));
	}
	else if(x == 10) {
		d->mucManager->kick(nick);
	}
	else if(x == 11) {
		GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		d->mucManager->ban(contact->s.mucItem().jid());
	}
	else if(x == 12) {
		GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		if (contact->s.mucItem().role() != MUCItem::Visitor)
			d->mucManager->setRole(nick, MUCItem::Visitor);
	}
	else if(x == 13) {
		GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		if (contact->s.mucItem().role() != MUCItem::Participant)
			d->mucManager->setRole(nick, MUCItem::Participant);
	}
	else if(x == 14) {
		GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		if (contact->s.mucItem().role() != MUCItem::Moderator)
			d->mucManager->setRole(nick, MUCItem::Moderator);
	}
	else if(x >= 100 && x<300) {
		// Kick || Ban with reason
		QString reason;
		QStringList reasons = PsiOptions::instance()->getOption("options.muc.reasons").toStringList();
		if (x==100 || x==200) {
			// Show custom reason dialog
			MUCReasonsEditor *editor=new MUCReasonsEditor(this);
			if (editor->exec())
				reason=editor->reason();
			delete editor;
		} else {
			int idx = (x<200) ? x-101 : x-201;
			if (idx<reasons.count())
				reason=reasons[idx];
		}
		if (!reason.isEmpty()) {
			if (x<200)
				d->mucManager->kick(nick, reason);
			else {
				GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
				if (!contact) return;
				d->mucManager->ban(contact->s.mucItem().jid(), reason);
			}
		}

	}
	else if(x == 15) {
		GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		if (contact->s.mucItem().affiliation() != MUCItem::NoAffiliation)
			d->mucManager->setAffiliation(contact->s.mucItem().jid(), MUCItem::NoAffiliation);
	}
	else if(x == 16) {
		GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		if (contact->s.mucItem().affiliation() != MUCItem::Member)
			d->mucManager->setAffiliation(contact->s.mucItem().jid(), MUCItem::Member);
	}
	else if(x == 17) {
		GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		if (contact->s.mucItem().affiliation() != MUCItem::Admin)
			d->mucManager->setAffiliation(contact->s.mucItem().jid(), MUCItem::Admin);
	}
	else if(x == 18) {
		GCUserViewItem *contact = (GCUserViewItem*) ui_.lv_users->findEntry(nick);
		if (contact->s.mucItem().affiliation() != MUCItem::Owner)
			d->mucManager->setAffiliation(contact->s.mucItem().jid(), MUCItem::Owner);
	}
}

void GCMainDlg::contextMenuEvent(QContextMenuEvent *)
{
	d->pm_settings->exec(QCursor::pos());
}

void GCMainDlg::buildMenu()
{
	// Dialog menu
	d->pm_settings->clear();

	d->act_clear->addTo( d->pm_settings );
	d->act_configure->addTo( d->pm_settings );
#ifdef WHITEBOARDING
	d->act_whiteboard->addTo( d->pm_settings );
#endif
	d->pm_settings->addSeparator();

	d->pm_settings->addAction(d->act_icon);
	d->pm_settings->addAction(d->act_templates);
	if (!PsiOptions::instance()->getOption("options.ui.chat.disable-paste-send").toBool())
		d->pm_settings->addAction(d->act_ps);
	d->pm_settings->addAction(d->act_nick);
	d->pm_settings->addAction(d->act_bookmark);
#ifdef PSI_PLUGINS
	if(!PsiOptions::instance()->getOption("options.ui.chat.central-toolbar").toBool()) {
		d->pm_settings->addSeparator();
		PluginManager::instance()->addGCToolBarButton(this, d->pm_settings, account(), jid().full());
	}
#endif
}

void GCMainDlg::chatEditCreated()
{
	d->mCmdSite.setInput(ui_.mle->chatEdit());
	d->mCmdSite.setPrompt(ui_.mini_prompt);
	d->tabCompletion.setTextEdit(d->mle());


	ui_.log->setDialog(this);
	ui_.mle->chatEdit()->setDialog(this);

	ui_.mle->chatEdit()->installEventFilter(d);
}

int GCMainDlg::unreadMessageCount() const
{
	return d->pending;
}

void GCMainDlg::setStatusTabIcon(int status)
{
	setTabIcon(PsiIconset::instance()->statusPtr(jid(), status)->icon());
}

void GCMainDlg::sendButtonMenu()
{
	SendButtonTemplatesMenu* menu = new SendButtonTemplatesMenu(this, true);
	menu->exec(QCursor::pos());
	d->mle()->setFocus();
}

void GCMainDlg::doPasteAndSend()
{
	d->mle()->paste();
	mle_returnPressed();
	d->act_ps->setEnabled(false);
	QTimer::singleShot(2000, this, SLOT(psButtonEnabled()));
}

void GCMainDlg::doBookmark()
{
	BookmarkManager *bm = account()->bookmarkManager();
	if (!bm->isAvailable()) {
		return;
	}
	QList<ConferenceBookmark> confs =  bm->conferences();
	int confInd = bm->indexOfConference(jid());
	if (confInd < 0) { // not found
		ConferenceBookmark conf(jid().bare(), jid(), false, nick(), d->password);
		confs.push_back(conf);
		bm->setBookmarks(confs);
		return;
	}
	ConferenceBookmark &b = confs[confInd];
	QDialog *dlg = new QDialog(this);
	QVBoxLayout *layout = new QVBoxLayout;
	QHBoxLayout *blayout = new QHBoxLayout;
	QFormLayout *formLayout = new QFormLayout;
	QLineEdit *txtName = new QLineEdit;
	QLineEdit *txtNick = new QLineEdit;
	QCheckBox *chkAJoin = new QCheckBox;
	QPushButton *saveBtn = new QPushButton(dlg->style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Save"), dlg);
	QPushButton *deleteBtn = new QPushButton(dlg->style()->standardIcon(QStyle::SP_DialogDiscardButton), tr("Delete"), dlg);
	QPushButton *cancelBtn = new QPushButton(dlg->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Cancel"), dlg);

	blayout->insertStretch(0);
	blayout->addWidget(saveBtn);
	blayout->addWidget(deleteBtn);
	blayout->addWidget(cancelBtn);
	txtName->setText(b.name());
	txtNick->setText(b.nick());
	chkAJoin->setChecked(b.autoJoin());
	formLayout->addRow(tr("&Name:"), txtName);
	formLayout->addRow(tr("N&ick:"), txtNick);
	formLayout->addRow(tr("&Auto join:"), chkAJoin);
	layout->addLayout(formLayout);
	layout->addLayout(blayout);
	dlg->setLayout(layout);
	dlg->setMinimumWidth(300);
	dlg->connect(saveBtn, SIGNAL(clicked()), dlg, SLOT(accept()));
	dlg->connect(deleteBtn, SIGNAL(clicked()), dlg, SLOT(reject()));
	dlg->connect(cancelBtn, SIGNAL(clicked()), dlg, SLOT(reject()));
	connect(deleteBtn, SIGNAL(clicked()), this, SLOT(doRemoveBookmark()));

	dlg->setWindowTitle(tr("Bookmark conference"));
	dlg->adjustSize();
	dlg->move(ui_.le_topic->mapToGlobal(QPoint(
			ui_.le_topic->width() - dlg->width(), ui_.le_topic->height())));
	if (dlg->exec() == QDialog::Accepted) {
		ConferenceBookmark conf(txtName->text(), jid(), chkAJoin->checkState() == Qt::Checked, txtNick->text(), d->password);
		confs[confInd] = conf;
		bm->setBookmarks(confs);
	}
	delete dlg;
}

void GCMainDlg::doRemoveBookmark()
{
	BookmarkManager *bm = account()->bookmarkManager();
	if (bm->isAvailable()) {
		bm->removeConference(jid());
	}
}

void GCMainDlg::psButtonEnabled()
{
	d->act_ps->setEnabled(true);
}

void GCMainDlg::sendTemp(QAction * action)
{
	if (action)
		d->mle()->textCursor().insertText(action->text().replace("&&", "&"));
	if (!PsiOptions::instance()->getOption("options.ui.chat.only-paste-template").toBool())
		mle_returnPressed();
}

void GCMainDlg::setMargins()
{
	ui_.vboxLayout->setContentsMargins(0,0,0,0);
	ui_.vboxLayout2->setContentsMargins(4,0,4,4);
	if (!d->tabmode) {
		ui_.hboxLayout->setContentsMargins(4,0,4,0);
		if (!isBorder()) {
			ui_.vboxLayout1->setContentsMargins(0,0,0,0);
		}
		else {
			ui_.vboxLayout1->setContentsMargins(0,4,0,0);
		}
	}
	else {
		ui_.vboxLayout1->setContentsMargins(4,4,4,0);
		ui_.hboxLayout->setContentsMargins(2,0,4,0);
	}
}

void GCMainDlg::setInitSize(QSize size, int x, int y)
{
	if (PsiOptions::instance()->getOption("options.ui.muc.maximized").toBool()) {
		if (isBorder()) {
#ifdef Q_WS_WIN
			window()->setWindowState(window()->windowState() | Qt::WindowFullScreen | Qt::WindowMaximized);
#else
			window()->setWindowState(window()->windowState() | Qt::WindowMaximized);
#endif
		}
		else {
			window()->setGeometry(qApp->desktop()->availableGeometry(-1));
		}
	}
	else {
		window()->resize(size);
		window()->move(x ,y);
	}
}

//----------------------------------------------------------------------------
// GCFindDlg
//----------------------------------------------------------------------------
GCFindDlg::GCFindDlg(const QString& str, QWidget* parent)
	: QDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
#if QT_VERSION >= 0x040500
        setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
#endif
	setWindowTitle(tr("Find"));
	QVBoxLayout *vb = new QVBoxLayout(this);
	vb->setMargin(4);
	QHBoxLayout *hb = new QHBoxLayout;
	vb->addLayout(hb);
	QLabel *l = new QLabel(tr("Find:"), this);
	hb->addWidget(l);
	le_input = new QLineEdit(this);
	hb->addWidget(le_input);
	vb->addStretch(1);

	QFrame *Line1 = new QFrame(this);
	Line1->setFrameShape( QFrame::HLine );
	Line1->setFrameShadow( QFrame::Sunken );
	Line1->setFrameShape( QFrame::HLine );
	vb->addWidget(Line1);

	hb = new QHBoxLayout;
	vb->addLayout(hb);
	hb->addStretch(1);
	QPushButton *pb_close = new QPushButton(tr("&Close"), this);
	connect(pb_close, SIGNAL(clicked()), SLOT(close()));
	QPushButton *pb_find = new QPushButton(tr("&Find"), this);
	pb_find->setDefault(true);
	connect(pb_find, SIGNAL(clicked()), SLOT(doFind()));
	hb->addWidget(pb_find);
	hb->addWidget(pb_close);
	pb_find->setAutoDefault(true);

	resize(200, minimumSizeHint().height());

	le_input->setText(str);
	le_input->setFocus();
}

GCFindDlg::~GCFindDlg()
{
}

void GCFindDlg::found()
{
	// nothing here to do...
}

void GCFindDlg::error(const QString &str)
{
	QMessageBox::warning(this, tr("Find"), tr("Search string '%1' not found.").arg(str));
	le_input->setFocus();
}

void GCFindDlg::doFind()
{
	emit find(le_input->text());
}

#include "groupchatdlg.moc"
