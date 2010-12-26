#include <QMenu>
#include <QContextMenuEvent>
#include <QtGui/QHeaderView>

#include "contactmanagerview.h"
#include "psiiconset.h"

ContactManagerView::ContactManagerView( QWidget * parent )
		: QTableView(parent)
{

}

void ContactManagerView::init()
{
	resizeColumnsToContents();
	horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
	horizontalHeader()->setStretchLastSection(true);
	horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
	verticalHeader()->setDefaultAlignment( Qt::AlignHCenter );

	connect(horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortByColumn(int)));
}

void ContactManagerView::contextMenuEvent( QContextMenuEvent * e )
{
	QMenu *popup = new QMenu(this);
	QList<QAction *> actions;
	actions <<new QAction(IconsetFactory::icon("psi/cm_check").icon(), tr("Check"), popup)
			<<new QAction(IconsetFactory::icon("psi/cm_uncheck").icon(), tr("Uncheck"), popup)
			<<new QAction(IconsetFactory::icon("psi/cm_invertcheck").icon(), tr("Invert"), popup);
	popup->addActions(actions);
	QAction *result = popup->exec(e->globalPos());
	int iresult;
	if (result) {
		iresult = actions.indexOf(result);
		const QVariant value(2);
		foreach(const QModelIndex &check, selectionModel()->selectedRows(0)) {
			switch (iresult) {
				case 0: //check
					model()->setData(check, 2);
					break;
				case 1: //uncheck
					model()->setData(check, 0);
					break;
				case 2: //invert
					model()->setData(check, 3);
					break;
			}
		}
	}
	delete popup;
}

void ContactManagerView::keyPressEvent( QKeyEvent * e )
{
	if (e->key() == Qt::Key_Space) {
		int data = 2; //check
		if (e->modifiers() & Qt::ControlModifier) {
			data = 3; //invert
		} else if (e->modifiers() & Qt::ShiftModifier) {
			data = 0; //uncheck
		}
		foreach(const QModelIndex &check, selectionModel()->selectedRows(0)) {
			model()->setData(check, data);
		}
		e->accept();
	} else {
		QTableView::keyPressEvent(e);
		e->ignore();
	}
}
