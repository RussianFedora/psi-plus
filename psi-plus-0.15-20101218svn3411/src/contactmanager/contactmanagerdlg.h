#ifndef CONTACTMANAGERDLG_H
#define CONTACTMANAGERDLG_H
#include "ui_contactmanagerdlg.h"

#include <QtGui/QDialog>
#include "contactmanagermodel.h"
class PsiAccount;

namespace Ui {
    class ContactManagerDlg;
}

class ContactManagerDlg : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ContactManagerDlg)
public:
	explicit ContactManagerDlg(PsiAccount *pa);
    virtual ~ContactManagerDlg();

protected:
    virtual void changeEvent(QEvent *e);

private:
	void changeDomain(QList<UserListItem *>& users);
	void changeGroup(QList<UserListItem *>& users);
	void exportRoster(QList<UserListItem *>& users);
	void importRoster();

	Ui::ContactManagerDlg ui_;
	PsiAccount *pa_;
	ContactManagerModel *um;

private slots:
	void doSelect();
	void executeCurrent();
	void showParamField(int index);
	void client_rosterUpdated(bool,int,QString);
};

#endif // CONTACTMANAGERDLG_H
