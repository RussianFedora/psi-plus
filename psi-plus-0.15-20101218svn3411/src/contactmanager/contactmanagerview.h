#ifndef CONTACTMANAGERVIEW_H
#define CONTACTMANAGERVIEW_H

#include <QTableView>

class ContactManagerView : public QTableView
{
	Q_OBJECT
public:
	ContactManagerView( QWidget * parent = 0 );
	void init();

protected:
	void contextMenuEvent( QContextMenuEvent * e );
	void keyPressEvent( QKeyEvent * e );
};

#endif // CONTACTMANAGERVIEW_H
