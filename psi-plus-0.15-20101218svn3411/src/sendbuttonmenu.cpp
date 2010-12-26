#include "sendbuttonmenu.h"
#include "psioptions.h"
#include "psiiconset.h"

SendButtonTemplatesEditor::SendButtonTemplatesEditor(QWidget* parent)
	: QDialog(parent)
{
	ui_.setupUi(this);
	ui_.lstTemplates->addItems(PsiOptions::instance()->getOption("options.ui.chat.templates").toStringList());

}

void SendButtonTemplatesEditor::addTemplate()
{
	t = ui_.txtTemplate->text().trimmed();
	if (t.isEmpty() || ui_.lstTemplates->count() >= 50) // 50 - default limit templates
		return;
	ui_.lstTemplates->addItem(t);
}

void SendButtonTemplatesEditor::removeTemplate()
{
	int idx=ui_.lstTemplates->currentRow();
	if (idx>=0) {
		QListWidgetItem *item =ui_.lstTemplates->takeItem(idx);
		if (item) delete item;
	}
}

void SendButtonTemplatesEditor::accept()
{
	QStringList templates;
	int cnt=ui_.lstTemplates->count();
	for (int i=0; i<cnt; ++i)
		templates.append(ui_.lstTemplates->item(i)->text());
	PsiOptions::instance()->setOption("options.ui.chat.templates", templates);
	t=ui_.txtTemplate->text();
	QDialog::accept();
}

SendButtonTemplatesMenu::SendButtonTemplatesMenu(QWidget* parent, bool ps)
	: QMenu(parent)

{
	setAttribute(Qt::WA_DeleteOnClose);
	update();
	ps_ = ps;
}

void SendButtonTemplatesMenu::update()
{
	clear();
	QStringList temp = PsiOptions::instance()->getOption("options.ui.chat.templates").toStringList();
	QAction *pasteSendAct, *editTemplatesAct, *onlyPaste;

	QActionGroup *tempAct = new QActionGroup(this);

	pasteSendAct = new QAction(IconsetFactory::icon("psi/action_paste_and_send").icon(), tr("Paste and &Send"), this);
	connect(pasteSendAct, SIGNAL(triggered()), parentWidget(), SLOT(doPasteAndSend()));

	editTemplatesAct = new QAction(IconsetFactory::icon("psi/action_templates_edit").icon(), tr("&Edit Templates"), this);
	connect(editTemplatesAct, SIGNAL(triggered()), SLOT(editTemplates()));

	onlyPaste = new QAction(tr("Only &Paste"), this);
	onlyPaste->setCheckable(true);
	onlyPaste->setChecked(PsiOptions::instance()->getOption("options.ui.chat.only-paste-template").toBool());
	connect(onlyPaste, SIGNAL(triggered()), SLOT(clickOnlyPaste()));

	for (int i = 0; i < temp.size(); ++i)
		tempAct->addAction(temp[i].replace("&", "&&"));
	connect(tempAct, SIGNAL(triggered(QAction *)), parentWidget(), SLOT(sendTemp(QAction *)));

	if (ps_) {
		addAction(pasteSendAct);
		addSeparator();
	}
	addAction(editTemplatesAct);
	addAction(onlyPaste);
	addSeparator();

	QList<QAction*> acts = tempAct->actions();
	for (int i = 0; i < acts.size(); ++i)
		addAction(acts[i]);
}

void SendButtonTemplatesMenu::editTemplates()
{
	SendButtonTemplatesEditor* editor = new SendButtonTemplatesEditor(this);
	editor->show(); 
}

void SendButtonTemplatesMenu::clickOnlyPaste()
{
	if (PsiOptions::instance()->getOption("options.ui.chat.only-paste-template").toBool())
		PsiOptions::instance()->setOption("options.ui.chat.only-paste-template", false);
	else
		PsiOptions::instance()->setOption("options.ui.chat.only-paste-template", true);
}
