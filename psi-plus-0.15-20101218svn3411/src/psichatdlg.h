#ifndef PSICHATDLG_H
#define PSICHATDLG_H

#include "minicmd.h"
#include "mcmdsimplesite.h"
#include "mcmdmanager.h"
#include "chatdlg.h"
#include "mcmdcompletion.h"
#include "sendbuttonmenu.h"

#include "ui_chatdlg.h"
#include "typeaheadfind.h"
#include "psiwindowheader.h"

class IconAction;
class PsiContact;

class PsiChatDlg : public ChatDlg
{
	Q_OBJECT
public:
	PsiChatDlg(const Jid& jid, PsiAccount* account, TabManager* tabManager);

	virtual void setVSplitterPosition(int log,int chat);

protected:
	// reimplemented
	void contextMenuEvent(QContextMenuEvent *);
	void doSend();
	bool eventFilter(QObject *obj, QEvent *event);


private:
	void setContactToolTip(QString text);

private slots:
	void toggleSmallChat();
	void doClearButton();
	void doMiniCmd();
	void addContact();
	void doMinimize();
	void buildMenu();
	void updateCounter();
	void updateIdentityVisibility();
	void updateCountVisibility();
	void updateContactAdding(PsiContact* c = 0);
	void updateContactAdding(const Jid &j);

	// reimplemented
	void chatEditCreated();
	void sendButtonMenu();
	void doPasteAndSend();
	void sendTemp(QAction * action);
	void psButtonEnabled();
	void verticalSplitterMoved(int, int);

private:
	void initToolBar();
	void initToolButtons();

	// reimplemented
	void initUi();
	void capsChanged();
	bool isEncryptionEnabled() const;
	void contactUpdated(UserListItem* u, int status, const QString& statusString);
	void updateAvatar();
	void optionsUpdate();
	void updatePGP();
	void setPGPEnabled(bool enabled);
	void activated();
	void setLooks();
	void setShortcuts();
	void appendSysMsg(const QString &);
	ChatView* chatView() const;
	ChatEdit* chatEdit() const;
	void setMargins();

private:
	Ui::ChatDlg ui_;

	QMenu* pm_settings_;

	IconAction* act_clear_;
	IconAction* act_history_;
	IconAction* act_info_;
	IconAction* act_pgp_;
	IconAction* act_icon_;
	IconAction* act_file_;
	IconAction* act_compact_;
	IconAction* act_voice_;
	TypeAheadFindBar *typeahead;
	IconAction* act_find;
	IconAction* act_ps_;
	IconAction* act_templates_;
	IconAction* act_html_text;
	IconAction* act_add_contact;
	QAction *act_mini_cmd_, *act_minimize_;
	SendButtonTemplatesMenu *template_menu;

	MCmdManager mCmdManager_;
	MCmdSimpleSite mCmdSite_;

	MCmdTabCompletion tabCompletion;

	bool smallChat_;
	class ChatDlgMCmdProvider;

	int logHeight;
	int chateditHeight;

private:
	bool tabmode;
	QPointer <PsiWindowHeader> winHeader_;

};

#endif
