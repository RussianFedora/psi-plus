/*
 * msgmle.cpp - subclass of PsiTextView to handle various hotkeys
 * Copyright (C) 2001-2003  Justin Karneges, Michail Pishchagin
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

#include "msgmle.h"

#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QLayout>
#include <QMenu>
#include <QResizeEvent>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTimer>

#include "shortcutmanager.h"
#include "spellchecker/spellhighlighter.h"
#include "spellchecker/spellchecker.h"
#include "psioptions.h"
#include "htmltextcontroller.h"


//----------------------------------------------------------------------------
// CapitalLettersController
//----------------------------------------------------------------------------

class CapitalLettersController : public QObject
{
	Q_OBJECT
public:
	CapitalLettersController(QTextEdit* parent)
		: QObject()
		, te_(parent)
		, enabled_(true)
		, capitalizeNext_(true)
	{
		connect(te_->document(), SIGNAL(contentsChange(int,int,int)), SLOT(textChanged(int,int,int)));
	}

	virtual ~CapitalLettersController() {};

	void setAutoCapitalizeEnabled(bool enabled)
	{
		if(enabled_ != enabled) {
			enabled_ = enabled;
			capitalizeNext_ = false;
		}
	}

private:
	void capitalizeChar(int pos, QChar c)
	{
		changeChar(pos, c.toUpper());
	}

	void decapitalizeChar(int pos, QChar c)
	{
		changeChar(pos, c.toLower());
	}

	void changeChar(int pos, QChar c)
	{
		QTextCursor cur = te_->textCursor();
		cur.setPosition(pos+1);
		const QTextCharFormat cf = cur.charFormat();
		cur.deletePreviousChar();
		cur.setCharFormat(cf);
		cur.insertText(c);
	}

public slots:
	void textChanged(int pos, int charsRemoved, int charsAdded)
	{
		if(enabled_) {
			if(charsRemoved != 0) {
				if(te_->toPlainText().isEmpty()) {
					capitalizeNext_ = true;
				}
				else {
					for(int i = te_->document()->characterCount(); i >= 0; i--) {
						QChar ch = te_->document()->characterAt(i);
						if(QString(ch) == "." || QString(ch) == "!" || QString(ch) == "?") {
							capitalizeNext_ = true;
							break;
						}
						else if(!ch.isLetter() || QString(ch).contains(QRegExp("\\s+"))) {
							continue;
						}
						else {
							capitalizeNext_ = false;
							break;
						}
					}
				}
			}

			if(charsAdded == 0) {
				return;
			}
			if(charsAdded == 2 && charsRemoved == 1 && pos == 0) { //the first letter after the previous message was sent
				capitalizeNext_ = true;
			}

			if(!te_->textCursor().atEnd()) {
				capitalizeNext_ = false;
				return;
			}

			QChar ch = te_->document()->characterAt(pos);
			QString str(ch);

			if(str == "." || str == "!" || str == "?") {
				capitalizeNext_ = true;
				return;
			}
			if(!ch.isLetter() || str.contains(QRegExp("\\s+"))) {
				return;
			}
			if(capitalizeNext_) {
				if(ch.isLower()) {
					capitalizeChar(pos, ch);
				}
				capitalizeNext_ = false;
			}
		}
	}

	void changeCase()
	{
		bool tmpEnabled = enabled_;
		enabled_ = false;
		for(int i = 0; i < te_->document()->characterCount(); i++) {
			QChar ch = te_->document()->characterAt(i);
			if(!ch.isLetter()) {
				continue;
			}

			if(ch.isLower()) {
				capitalizeChar(i, ch);
			}
			else {
				decapitalizeChar(i, ch);
			}
		}
		enabled_ = tmpEnabled;
	}

private:
	QTextEdit* te_;
	bool enabled_;
	bool capitalizeNext_;
};


//----------------------------------------------------------------------------
// ChatEdit
//----------------------------------------------------------------------------
ChatEdit::ChatEdit(QWidget *parent)
	: QTextEdit(parent)
	, dialog_(0)
	, check_spelling_(false)
	, spellhighlighter_(0)
{
	controller_ = new HTMLTextController(this);
	capitalizer_ = new CapitalLettersController(this);

	setWordWrapMode(QTextOption::WordWrap);
	setAcceptRichText(false);

	setReadOnly(false);
	setUndoRedoEnabled(true);

	setMinimumHeight(48);

	previous_position_ = 0;
	setCheckSpelling(checkSpellingGloballyEnabled());
	connect(PsiOptions::instance(),SIGNAL(optionChanged(const QString&)),SLOT(optionsChanged()));
	typedMsgsIndex = 0;

	initActions();
	setShortcuts();
	optionsChanged();
}

ChatEdit::~ChatEdit()
{
	clearMessageHistory();
	delete spellhighlighter_;
	delete controller_;
	delete capitalizer_;
}

CapitalLettersController * ChatEdit::capitalizer()
{
	return capitalizer_;
}

void ChatEdit::initActions() {
	act_showMessagePrev = new QAction(this);
	addAction(act_showMessagePrev);
	connect(act_showMessagePrev, SIGNAL(activated()), SLOT(showHistoryMessagePrev()));

	act_showMessageNext= new QAction(this);
	addAction(act_showMessageNext);
	connect(act_showMessageNext, SIGNAL(activated()), SLOT(showHistoryMessageNext()));

	act_showMessageFirst = new QAction(this);
	addAction(act_showMessageFirst);
	connect(act_showMessageFirst, SIGNAL(activated()), SLOT(showHistoryMessageFirst()));

	act_showMessageLast= new QAction(this);
	addAction(act_showMessageLast);
	connect(act_showMessageLast, SIGNAL(activated()), SLOT(showHistoryMessageLast()));

	act_changeCase = new QAction(this);
	addAction(act_changeCase);
	connect(act_changeCase, SIGNAL(triggered()), capitalizer_, SLOT(changeCase()));
}

void ChatEdit::setShortcuts() {
	act_showMessagePrev->setShortcuts(ShortcutManager::instance()->shortcuts("chat.show-messagePrev"));
	act_showMessageNext->setShortcuts(ShortcutManager::instance()->shortcuts("chat.show-messageNext"));
	act_showMessageFirst->setShortcuts(ShortcutManager::instance()->shortcuts("chat.show-messageFirst"));
	act_showMessageLast->setShortcuts(ShortcutManager::instance()->shortcuts("chat.show-messageLast"));
	act_changeCase->setShortcuts(ShortcutManager::instance()->shortcuts("chat.change-case"));
}

void ChatEdit::setDialog(QWidget* dialog)
{
	dialog_ = dialog;
}

QSize ChatEdit::sizeHint() const
{
	return minimumSizeHint();
}

void ChatEdit::setFont(const QFont &f)
{
	QTextEdit::setFont(f);
	controller_->setFont(f);
}

bool ChatEdit::checkSpellingGloballyEnabled()
{
	return (SpellChecker::instance()->available() && PsiOptions::instance()->getOption("options.ui.spell-check.enabled").toBool());
}

void ChatEdit::setCheckSpelling(bool b)
{
	check_spelling_ = b;
	if (check_spelling_) {
		if (!spellhighlighter_)
			spellhighlighter_ = new SpellHighlighter(document());
	}
	else {
		delete spellhighlighter_;
		spellhighlighter_ = 0;
	}
}

bool ChatEdit::focusNextPrevChild(bool next)
{
	return QWidget::focusNextPrevChild(next);
}

// Qt text controls are quite greedy to grab key events.
// disable that.
bool ChatEdit::event(QEvent * event) {
	if (event->type() == QEvent::ShortcutOverride) {
		return false;
	}
	return QTextEdit::event(event);
}

void ChatEdit::keyPressEvent(QKeyEvent *e)
{
/*	if(e->key() == Qt::Key_Escape || (e->key() == Qt::Key_W && e->modifiers() & Qt::ControlModifier))
		e->ignore();
	else if(e->key() == Qt::Key_Return && 
	       ((e->modifiers() & Qt::ControlModifier) 
#ifndef Q_WS_MAC
	       || (e->modifiers() & Qt::AltModifier) 
#endif
	       ))
		e->ignore();
	else if(e->key() == Qt::Key_M && (e->modifiers() & Qt::ControlModifier)) // newline
		insert("\n");
	else if(e->key() == Qt::Key_H && (e->modifiers() & Qt::ControlModifier)) // history
		e->ignore();
	else  if(e->key() == Qt::Key_S && (e->modifiers() & Qt::AltModifier))
		e->ignore();
	else*/ if(e->key() == Qt::Key_U && (e->modifiers() & Qt::ControlModifier))
		setText("");
/*	else if((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && !((e->modifiers() & Qt::ShiftModifier) || (e->modifiers() & Qt::AltModifier)) && LEGOPTS.chatSoftReturn)
		e->ignore();
	else if((e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown) && (e->modifiers() & Qt::ShiftModifier))
		e->ignore();
	else if((e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown) && (e->modifiers() & Qt::ControlModifier))
		e->ignore(); */
#ifdef Q_WS_MAC
	else if (e->key() == Qt::Key_QuoteLeft && e->modifiers() == Qt::ControlModifier) {
		e->ignore();
	}
#endif
	else
	{
		QTextEdit::keyPressEvent(e);
	}
}

/**
 * Work around Qt bug, that QTextEdit doesn't accept() the 
 * event, so it could result in another context menu popping
 * out after the first one.
 */
void ChatEdit::contextMenuEvent(QContextMenuEvent *e) 
{
	last_click_ = e->pos();
	if (check_spelling_ && textCursor().selectedText().isEmpty() && SpellChecker::instance()->available()) {
		// Check if the word under the cursor is misspelled
		QTextCursor tc = cursorForPosition(last_click_);
		tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
		tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
		QString selected_word = tc.selectedText();
		if (!selected_word.isEmpty() && !SpellChecker::instance()->isCorrect(selected_word)) {
			QList<QString> suggestions = SpellChecker::instance()->suggestions(selected_word);
			if (!suggestions.isEmpty() || SpellChecker::instance()->writable()) {
				QMenu spell_menu;
				if (!suggestions.isEmpty()) {
					foreach(QString suggestion, suggestions) {
						QAction* act_suggestion = spell_menu.addAction(suggestion);
						connect(act_suggestion,SIGNAL(triggered()),SLOT(applySuggestion()));
					}
					spell_menu.addSeparator();
				}
				if (SpellChecker::instance()->writable()) {
					QAction* act_add = spell_menu.addAction(tr("Add to dictionary"));
					connect(act_add,SIGNAL(triggered()),SLOT(addToDictionary()));
				}
				spell_menu.exec(QCursor::pos());
				e->accept();
				return;
			}
		}
	}

	// Do normal menu
	QTextEdit::contextMenuEvent(e);
	e->accept();
}

/*!
 * \brief handles a click on a suggestion
 * \param the action is just the container which holds the suggestion.
 * 
 * This method is called by the framework whenever a user clicked on the child popupmenu
 * to select a suggestion for a missspelled word. It exchanges the missspelled word with the
 * suggestion which is the text of the QAction parameter.
 */
void ChatEdit::applySuggestion()
{
	QAction* act_suggestion = (QAction*) sender();
	int current_position = textCursor().position();
	
	// Replace the word
	QTextCursor	tc = cursorForPosition(last_click_);
	tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
	tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
	int old_length = tc.position() - tc.anchor();
	tc.insertText(act_suggestion->text());
	tc.clearSelection();

	// Put the cursor where it belongs
	int new_length = act_suggestion->text().length();
	tc.setPosition(current_position - old_length + new_length);
	setTextCursor(tc);
}

/*!
 * \brief handles a click on the add2dict action of the parent popupmenu
 * \param Never used bool parameter
 * 
 * The method sets the cursor to the last mouseclick position and looks for the word which is placed there.
 * This word is than added to the dictionary of aspell.
 */
void ChatEdit::addToDictionary()
{
	QTextCursor	tc = cursorForPosition(last_click_);
	int current_position = textCursor().position();

	// Get the selected word
	tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
	tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
	SpellChecker::instance()->add(tc.selectedText());
	
	// Put the cursor where it belongs
	tc.clearSelection();
	tc.setPosition(current_position);
	setTextCursor(tc);
}

void ChatEdit::optionsChanged()
{
	setCheckSpelling(checkSpellingGloballyEnabled());
	capitalizer_->setAutoCapitalizeEnabled(PsiOptions::instance()->getOption("options.ui.chat.auto-capitalize").toBool());
}

void ChatEdit::showHistoryMessageNext() {
	if (!typedMsgsHistory.isEmpty()) {
		if (typedMsgsIndex + 1 < typedMsgsHistory.size()) {
			++typedMsgsIndex;
			showMessageHistory();
		} else {
			if(typedMsgsIndex != typedMsgsHistory.size()) {
				typedMsgsIndex = typedMsgsHistory.size();
				// Restore last typed text
				setEditText(currentText);
			}
		}
	}
}

void ChatEdit::showHistoryMessagePrev() {
	if (!typedMsgsHistory.isEmpty() && typedMsgsIndex > 0) {
		// Save current typed text
		if (typedMsgsIndex == typedMsgsHistory.size())
			currentText = text();
		--typedMsgsIndex;
		showMessageHistory();
	}
}

void ChatEdit::showHistoryMessageFirst() {
	if (!typedMsgsHistory.isEmpty()) {
		if (currentText.isEmpty()) {
			typedMsgsIndex = typedMsgsHistory.size() - 1;
			showMessageHistory();
		} else {
			typedMsgsIndex = typedMsgsHistory.size();
			// Restore last typed text
			setEditText(currentText);
		}
	}
}

void ChatEdit::showHistoryMessageLast() {
	if (!typedMsgsHistory.isEmpty()) {
		typedMsgsIndex = 0;
		showMessageHistory();
	}
}

void ChatEdit::setEditText(QString text)
{
	setPlainText(text);
	moveCursor(QTextEdit::MoveEnd, false);
}

void ChatEdit::showMessageHistory() {
	setEditText(typedMsgsHistory.at(typedMsgsIndex));
}

void ChatEdit::appendMessageHistory(QString text) {
	if (!text.simplified().isEmpty()) {
		if (currentText == text)
			// Remove current typed text only if we want to add it to history
			currentText.clear();
		long index = typedMsgsHistory.indexOf(text);
		if (index >=0) {
			typedMsgsHistory.removeAt(index);
		}
		if (typedMsgsHistory.size() >= MAX_MESSAGE_HISTORY) {
			typedMsgsHistory.removeAt(0);
		}
		typedMsgsHistory += text;
		typedMsgsIndex = typedMsgsHistory.size();
	}
}

void ChatEdit::clearMessageHistory() {
	typedMsgsHistory.clear();
	typedMsgsIndex = 0;
}

XMPP::HTMLElement ChatEdit::toHTMLElement() {
	XMPP::HTMLElement elem;
	QString html = toHtml();
	int index = html.indexOf("<body");
	int lastIndex = html.lastIndexOf("</body>");
	if(index == -1 || lastIndex == -1)
		return elem;
	lastIndex += 7;
	html = html.mid(index, lastIndex-index);
	QDomDocument doc;
	if(!doc.setContent(html))
		return elem;	
	QDomElement htmlElem = doc.firstChildElement("body");
	QDomElement p = htmlElem.firstChildElement("p");
	QDomElement body = doc.createElementNS("http://www.w3.org/1999/xhtml", "body");
	bool foundSpan = false;
	while(!p.isNull()) {
		p.removeAttribute("style");
		body.appendChild(p.cloneNode(true).toElement());
		if(!p.firstChildElement("span").isNull())
			foundSpan = true;
		p = p.nextSiblingElement("p");
	}
	if(foundSpan)
		elem.setBody(body);
	return elem;
}

void ChatEdit::doHTMLTextMenu() {
	controller_->doMenu();
}

void ChatEdit::setCssString(const QString &css) {
	controller_->setCssString(css);
}

//----------------------------------------------------------------------------
// LineEdit
//----------------------------------------------------------------------------
LineEdit::LineEdit( QWidget *parent)
	: ChatEdit(parent)
{
	setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // no need for horizontal scrollbar with this
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setMinimumHeight(0);

	connect(this, SIGNAL(textChanged()), SLOT(recalculateSize()));
}

LineEdit::~LineEdit()
{
}

QSize LineEdit::minimumSizeHint() const
{
	QSize sh = QTextEdit::minimumSizeHint();
	sh.setHeight(fontMetrics().height() + 1);
	sh += QSize(0, QFrame::lineWidth() * 2);
	return sh;
}

QSize LineEdit::sizeHint() const
{
	QSize sh = QTextEdit::sizeHint();
	sh.setHeight(int(document()->documentLayout()->documentSize().height()));
	sh += QSize(0, QFrame::lineWidth() * 2);
	((QTextEdit*)this)->setMaximumHeight(sh.height());
	return sh;
}

void LineEdit::resizeEvent(QResizeEvent* e)
{
	ChatEdit::resizeEvent(e);
	QTimer::singleShot(0, this, SLOT(updateScrollBar()));
}

void LineEdit::recalculateSize()
{
	updateGeometry();
	QTimer::singleShot(0, this, SLOT(updateScrollBar()));
}

void LineEdit::updateScrollBar()
{
	setVerticalScrollBarPolicy(sizeHint().height() > height() ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
	ensureCursorVisible();
}

#include "msgmle.moc"
