/*
 * historydlg.cpp
 * Copyright (C) 2001-2010  Justin Karneges, Michail Pishchagin,
 * Piotr Okonski, Evgeny Khryukin
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

#include <QMessageBox>
#include <QScrollBar>
#include <QMenu>
#include <QProgressDialog>

#include "historydlg.h"
#include "psiaccount.h"
#include "psicon.h"
#include "psicontact.h"
#include "psiiconset.h"
#include "psioptions.h"
#include "textutil.h"
#include "jidutil.h"
#include "fileutil.h"
#include "userlist.h"
#include "common.h"

static QString getNext(QString *str)
{
	int n = 0;
	// skip leading spaces (but *do* return them later!)
	while(n < (int)str->length() && str->at(n).isSpace()) {
		++n;
	}
	if(n == (int)str->length()) {
		return QString::null;
	}
	// find end or next space
	while(n < (int)str->length() && !str->at(n).isSpace()) {
		++n;
	}
	QString result = str->mid(0, n);
	*str = str->mid(n);
	return result;
}

// wraps a string against a fixed width
static QStringList wrapString(const QString &str, int wid)
{
	QStringList lines;
	QString cur;
	QString tmp = str;
	//printf("parsing: [%s]\n", tmp.latin1());
	while(1) {
		QString word = getNext(&tmp);
		if(word == QString::null) {
			lines += cur;
			break;
		}
		//printf("word:[%s]\n", word.latin1());
		if(!cur.isEmpty()) {
			if((int)cur.length() + (int)word.length() > wid) {
				lines += cur;
				cur = "";
			}
		}
		if(cur.isEmpty()) {
			// trim the whitespace in front
			for(int n = 0; n < (int)word.length(); ++n) {
				if(!word.at(n).isSpace()) {
					if(n > 0) {
						word = word.mid(n);
					}
					break;
				}
			}
		}
		cur += word;
	}
	return lines;
}


class HistoryDlg::Private
{
public:
	Private()
	{
	}

	~Private()
	{
		delete h;
	}

	Jid jid;
	PsiAccount *pa;
	PsiCon *psi;
	EDBHandle *h;
	EDBHandle *exp;
	QString id_prev, id_begin, id_end, id_next;
	int reqtype;
	QString findStr;
	QDate date;
};

HistoryDlg::HistoryDlg(const Jid &jid, PsiAccount *pa)
	: QDialog(0, Qt::Window)
{
	ui_.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	setModal(false);
	d = new Private;
	d->reqtype = 99;
	d->pa = pa;
	d->psi = pa->psi();
	d->jid = jid;
	d->pa->dialogRegister(this, d->jid);
	d->h = new EDBHandle(d->pa->edb());

#ifndef Q_WS_MAC
	setWindowIcon(IconsetFactory::icon("psi/history").icon());
#endif
	ui_.tb_find->setIcon(IconsetFactory::icon("psi/search").icon());

	ui_.msgLog->setFont(fontForOption("options.ui.look.font.chat"));
	ui_.jidList->setFont(fontForOption("options.ui.look.font.contactlist"));

	ui_.calendar->setFirstDayOfWeek(firstDayOfWeekFromLocale());

	connect(d->h, SIGNAL(finished()), SLOT(edb_finished()));
	connect(ui_.searchField, SIGNAL(returnPressed()), SLOT(findMessages()));
	connect(ui_.searchField, SIGNAL(textChanged(const QString)), SLOT(highlightBlocks(const QString)));
	connect(ui_.buttonPrevious, SIGNAL(released()), SLOT(getPrevious()));
	connect(ui_.buttonNext, SIGNAL(released()), SLOT(getNext()));
	connect(ui_.buttonRefresh, SIGNAL(released()), SLOT(refresh()));
	connect(ui_.jidList, SIGNAL(itemSelectionChanged()), SLOT(openSelectedContact()));
	connect(ui_.tb_find, SIGNAL(clicked()), SLOT(findMessages()));
	connect(ui_.buttonLastest, SIGNAL(released()), SLOT(getLatest()));
	connect(ui_.buttonEarliest, SIGNAL(released()), SLOT(getEarliest()));
	connect(ui_.calendar, SIGNAL(selectionChanged()), SLOT(getDate()));
	connect(ui_.calendar, SIGNAL(activated(QDate)), SLOT(getDate()));

	connect(d->pa, SIGNAL(removedContact(PsiContact*)), SLOT(removedContact(PsiContact*)));

	ui_.jidList->installEventFilter(this);

	listAccounts();
	loadContacts();

	int height = PsiOptions::instance()->getOption("options.ui.save.history-height").toInt();
	int width = PsiOptions::instance()->getOption("options.ui.save.history-width").toInt();
	resize(width, height);

	ui_.jidList->setFocus();
}

HistoryDlg::~HistoryDlg()
{
	PsiOptions::instance()->setOption("options.ui.save.history-height", height());
	PsiOptions::instance()->setOption("options.ui.save.history-width", width());
	delete d;
}

bool HistoryDlg::eventFilter(QObject *obj, QEvent *e)
{
	if(obj == ui_.jidList && e->type() == QEvent::ContextMenu) {
		e->accept();
		QTimer::singleShot(0, this, SLOT(doMenu()));
		return true;
	}

	return QDialog::eventFilter(obj, e);
}

QFont HistoryDlg::fontForOption(const QString &option)
{
	QFont font;
	font.fromString(PsiOptions::instance()->getOption(option).toString());
	return font;
}

void HistoryDlg::changeAccount(const QString /*accountName*/)
{
	ui_.msgLog->clear();
	setButtons(false);
	d->jid = QString();
	d->pa = d->psi->contactList()->getAccountByJid(ui_.accountsBox->itemData(ui_.accountsBox->currentIndex()).toString());
	delete d->h;
	d->h = 0;
	d->h = new EDBHandle(d->pa->edb()); //set handle to new EDB
	connect(d->h, SIGNAL(finished()), SLOT(edb_finished()));
	loadContacts();
	ui_.jidList->setCurrentRow(0);
	openSelectedContact();
}

void HistoryDlg::listAccounts()
{
	if (d->psi)
	{
		foreach (PsiAccount* account, d->psi->contactList()->enabledAccounts())
			ui_.accountsBox->addItem(IconsetFactory::icon("psi/account").icon(), account->nameWithJid(), QVariant(account->jid().full()));
	}
	//select active account
	ui_.accountsBox->setCurrentIndex(ui_.accountsBox->findData(d->pa->jid().full()));
	//connect signal after the list is populated to prevent execution in the middle of the loop
	connect(ui_.accountsBox, SIGNAL(currentIndexChanged(const QString)), SLOT(changeAccount(const QString)));
}

void HistoryDlg::loadContacts()
{
	jids_.clear();
	ui_.jidList->clear();
	ui_.msgLog->clear();
	foreach (PsiContact* contact, d->pa->contactList())
	{
		if(contact->isConference()
		|| contact->isPrivate()
		|| jids_.contains(contact->jid().bare()))
			continue;

		QListWidgetItem *item = new QListWidgetItem(contact->name(), ui_.jidList);
		item->setToolTip(contact->jid().bare());
		item->setIcon(PsiIconset::instance()->statusPtr(contact->jid(),Status(Status::Online))->icon());
		//item->setIcon(PsiIconset::instance()->status(contact->status()).icon());
		ui_.jidList->addItem(item);
		jids_.append(item->toolTip());
	}
	PsiContact* self = d->pa->selfContact();
	if(!jids_.contains(self->jid().bare())) {
		QListWidgetItem *item = new QListWidgetItem(self->name(), ui_.jidList);
		item->setToolTip(self->jid().bare());
		//item->setIcon(PsiIconset::instance()->status(self->status()).icon());
		item->setIcon(PsiIconset::instance()->statusPtr(self->jid(),Status(Status::Online))->icon());
		ui_.jidList->addItem(item);
		jids_.append(item->toolTip());
	}

	ui_.jidList->sortItems();
	//set contact in jidList to selected jid
	for (int i = 0; i < ui_.jidList->count(); i++)
	{
		if (ui_.jidList->item(i)->toolTip() == d->jid.bare().toLower())
			ui_.jidList->setCurrentRow(i);  //triggers openSelectedContact()
	}
}

void HistoryDlg::openSelectedContact()
{
	ui_.msgLog->clear();
	UserListItem *u = currentUserListItem();
	if (!u)
		return;

	setWindowTitle(u->name() + " (" + u->jid().full() + ")");
	d->jid = u->jid();
	getLatest();
}

void HistoryDlg::highlightBlocks(const QString text)
{
	QTextCursor cur = ui_.msgLog->textCursor();
	cur.clearSelection();
	cur.movePosition(QTextCursor::Start);
	ui_.msgLog->setTextCursor(cur);	

	if (text.isEmpty()) {
		getLatest();
		return;
	}

	QList<QTextEdit::ExtraSelection> extras;
	QTextEdit::ExtraSelection highlight;
	highlight.format.setBackground(Qt::yellow);
	highlight.cursor = ui_.msgLog->textCursor();	

	bool found = ui_.msgLog->find(text);
	while (found)
	{
		highlight.cursor = ui_.msgLog->textCursor();
		extras << highlight;
		found = ui_.msgLog->find(text);
	}

	ui_.msgLog->setExtraSelections(extras);
}

void HistoryDlg::findMessages()
{
	//get the oldest event as a starting point
	startRequest();
	d->reqtype = 3;
	d->h->getOldest(d->jid, 1);
}

void HistoryDlg::removeHistory()
{
	int res = QMessageBox::question(this, tr("Remove history"),
					tr("Are you sure you want to completely remove history for a contact %1?").arg(d->jid.bare())
					,QMessageBox::Ok | QMessageBox::Cancel);
	if(res == QMessageBox::Ok) {
		d->h->erase(d->jid);
		openSelectedContact();
	}
}

void HistoryDlg::openChat()
{
	UserListItem *u = currentUserListItem();
	if(u) {
		d->pa->actionOpenChat2(u->jid().bare());
	}
}

void HistoryDlg::exportHistory()
{
	UserListItem *u = currentUserListItem();
	if(!u)
		return;
	QString them = JIDUtil::nickOrJid(u->name(), u->jid().full());
	QString s = JIDUtil::encode(them).toLower();
	QString fname = FileUtil::getSaveFileName(this,
						  tr("Export message history"),
						  s + ".txt",
						  tr("Text files (*.txt);;All files (*.*)"));
	if(fname.isEmpty())
		return;

	QFile f(fname);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		QMessageBox::information(this, tr("Error"), tr("Error writing to file."));
		return;
	}
	QTextStream stream(&f);

	QString us = d->pa->nick();

	d->exp = new EDBHandle(d->pa->edb());
	QString id;
	startRequest();
	while(1) {
		if(id.isEmpty()) {
			d->exp->getOldest(d->jid, 1000);
		}
		else {
			d->exp->get(d->jid, id, EDB::Forward, 1000);
		}
		while(d->exp->busy()) {
			qApp->processEvents();
		}

		const EDBResult *r = d->exp->result();
		if(!r) {
			break;
		}
		if(r->count() <= 0) {
			break;
		}

		// events are in forward order
		for(int i = 0; i < r->count(); ++i) {
			EDBItem* item = r->value(i);
			id = item->nextId();
			PsiEvent *e = item->event();
			QString txt;

			QString ts = e->timeStamp().toString(Qt::LocalDate);

			QString nick;
			if(e->originLocal()) {
				nick = us;
			}
			else {
				nick = them;
			}

			if(e->type() == PsiEvent::Message) {
				MessageEvent *me = (MessageEvent *)e;
				stream << QString("[%1] <%2>: ").arg(ts, nick)/* << endl*/;

				QStringList lines = me->message().body().split('\n', QString::KeepEmptyParts);
				foreach(const QString& str, lines) {
					QStringList sub = wrapString(str, 72);
					foreach(const QString& str2, sub) {
						txt += str2 + "\n" + QString("    ");
					}
				}
			}
			else {
				continue;
			}

			stream << txt << endl;
		}

		// done!
		if(id.isEmpty()) {
			break;
		}
	}
	delete d->exp;
	d->exp = 0;
	f.close();
	stopRequest();
}

void HistoryDlg::doMenu()
{
	QMenu *m = new QMenu(ui_.jidList);
	m->addAction(IconsetFactory::icon("psi/chat").icon(), tr("&Open chat"), this, SLOT(openChat()));
	m->addAction(IconsetFactory::icon("psi/save").icon(), tr("&Export history"), this, SLOT(exportHistory()));
	m->addAction(IconsetFactory::icon("psi/clearChat").icon(), tr("&Delete history"), this, SLOT(removeHistory()));
	m->exec(QCursor::pos());

}

void HistoryDlg::edb_finished()
{
	setButtons(false);
	stopRequest();

	const EDBResult *r = d->h->result();
	if (d->h->lastRequestType() == EDBHandle::Read && r)
	{
		if (r->count() > 0)
		{
			if (d->reqtype == 0 || d->reqtype == 1)
			{
				// events are in backward order
				// first entry is the end event
				EDBItem* it = r->first();
				d->id_end = it->id();
				d->id_next = it->nextId();
				// last entry is the begin event
				it = r->last();
				d->id_begin = it->id();
				d->id_prev = it->prevId();
				displayResult(r, EDB::Forward);
			}
			else if (d->reqtype == 2)
			{
				// events are in forward order
				// last entry is the end event
				EDBItem* it = r->last();
				d->id_end = it->id();
				d->id_next = it->nextId();
				// first entry is the begin event
				it = r->first();
				d->id_begin = it->id();
				d->id_prev = it->prevId();
				displayResult(r, EDB::Backward);
			}
			else if (d->reqtype == 3)
			{
				QString str = ui_.searchField->text();
				if (str.isEmpty())
				{
					getLatest();
					return;
				}
				d->reqtype = 4;
				d->findStr = str;
				EDBItem *ei = r->first();
				startRequest();
				d->h->find(str, d->jid, ei->id(), EDB::Forward);
			}
			else if (d->reqtype == 4)
			{
				displayResult(r, EDB::Forward);
				highlightBlocks(ui_.searchField->text());
				return;
			}

			setButtons();
		}
		else
		{
			ui_.msgLog->clear();

			if (d->reqtype == 4)
			{
				return;
			}
		}
	}
}

void HistoryDlg::setButtons()
{
	ui_.buttonPrevious->setEnabled(!d->id_prev.isEmpty());
	ui_.buttonNext->setEnabled(!d->id_next.isEmpty());
	ui_.buttonEarliest->setEnabled(!d->id_prev.isEmpty());
	ui_.buttonLastest->setEnabled(!d->id_next.isEmpty());
}

void HistoryDlg::setButtons(bool act)
{
	ui_.buttonPrevious->setEnabled(act);
	ui_.buttonNext->setEnabled(act);
	ui_.buttonEarliest->setEnabled(act);
	ui_.buttonLastest->setEnabled(act);
}

void HistoryDlg::refresh()
{
	ui_.calendar->setSelectedDate(QDate::currentDate());
	ui_.searchField->clear();
	getLatest();
}

void HistoryDlg::getLatest()
{
	d->reqtype = 0;
	startRequest();
	d->h->getLatest(d->jid, 50);
}

void HistoryDlg::getEarliest()
{
	d->reqtype = 2;
	startRequest();
	d->h->getOldest(d->jid, 50);
}

void HistoryDlg::getPrevious()
{
	d->reqtype = 1;
	ui_.buttonPrevious->setEnabled(false);
	d->h->get(d->jid, d->id_prev, EDB::Backward, 50);
}

void HistoryDlg::getNext()
{
	d->reqtype = 2;
	ui_.buttonNext->setEnabled(false);
	d->h->get(d->jid, d->id_next, EDB::Forward, 50);
}

void HistoryDlg::getDate()
{
	const QDate date = ui_.calendar->selectedDate();
	d->reqtype = 2;
	d->date = date;
	QDateTime first (d->date);
	QDateTime last = first.addDays(1);
	startRequest();
	d->h->getByDate(d->jid, first, last);
}

void HistoryDlg::removedContact(PsiContact *pc)
{
	QString jid = pc->jid().bare().toLower();
	QString curJid  = ui_.jidList->currentItem()->toolTip();
	for(int i = 0; i < ui_.jidList->count(); i++) {
		QListWidgetItem *it = ui_.jidList->item(i);
		if(it && it->toolTip() == jid) {
			ui_.jidList->removeItemWidget(it);
			if(jid == curJid) {
				ui_.jidList->setCurrentRow(0);
				openSelectedContact();
			}
			break;
		}
	}
}

void HistoryDlg::displayResult(const EDBResult *r, int direction, int max)
{
	int i  = (direction == EDB::Forward) ? r->count() - 1 : 0;
	int at = 0;
	ui_.msgLog->clear();
	while (i >= 0 && i <= r->count() - 1 && (max == -1 ? true : at < max))
	{
		EDBItem* item = r->value(i);
		PsiEvent* e = item->event();
		UserListItem *u = d->pa->findFirstRelevant(e->from().full());
		QString from = JIDUtil::nickOrJid(u->name(), u->jid().full());
		if (e->type() == PsiEvent::Message)
		{
			MessageEvent *me = (MessageEvent *) e;
			QString msg = me->message().body();
			msg = TextUtil::linkify(TextUtil::plain2rich(msg));

			if (PsiOptions::instance()->getOption("options.ui.emoticons.use-emoticons").toBool())
				msg = TextUtil::emoticonify(msg);
			if (PsiOptions::instance()->getOption("options.ui.chat.legacy-formatting").toBool())
				msg = TextUtil::legacyFormat(msg);

			if (me->originLocal())
				msg = "<span style='color:red'>" + me->timeStamp().toString("[dd.MM.yyyy hh:mm:ss]")+" &lt;"+ d->pa->nick() +"&gt; " + msg + "</span>";
			else
				msg = "<span style='color:blue'>" + me->timeStamp().toString("[dd.MM.yyyy hh:mm:ss]") + " &lt;" +  from + "&gt; " + msg + "</span>";

			ui_.msgLog->appendText(msg);

		}

		++at;
		i += (direction == EDB::Forward) ? -1 : +1;
	}

	ui_.msgLog->verticalScrollBar()->setValue(ui_.msgLog->verticalScrollBar()->maximum());
}

UserListItem* HistoryDlg::currentUserListItem()
{
	UserListItem* u = 0;
	QListWidgetItem *i = ui_.jidList->currentItem();
	if(!i)
		return u;

	u = d->pa->findFirstRelevant(i->toolTip());
	return u;
}

void HistoryDlg::startRequest()
{
	if(!ui_.busy->isActive()) {
		ui_.busy->start();
	}
	setEnabled(false);
}

void HistoryDlg::stopRequest()
{
	if(ui_.busy->isActive()) {
		ui_.busy->stop();		
	}
	setEnabled(true);
}
