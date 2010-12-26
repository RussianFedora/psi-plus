#ifndef SENDBUTTONMENU_H
#define SENDBUTTONMENU_H

#include <QDialog>
#include <QMenu>
#include "ui_sendbuttontemplateseditor.h"

class SendButtonTemplatesEditor : public QDialog
{
	Q_OBJECT
public:
	SendButtonTemplatesEditor(QWidget* parent = 0);
private:
		Ui::SendButtonTemplatesEditor ui_;
		QString t;

private slots:
	void addTemplate();
	void removeTemplate();

protected slots:
	void accept();

};

class SendButtonTemplatesMenu : public QMenu
{
	Q_OBJECT 
public:
	SendButtonTemplatesMenu(QWidget* parent = 0, bool ps = false);
private:
	bool ps_;
private slots:
	void update();
	void editTemplates();
	void clickOnlyPaste();

};

#endif
