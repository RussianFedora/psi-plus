#ifndef MENUACCESSOR_H
#define MENUACCESSOR_H

#include <QList>
#include <QVariantHash>

class MenuAccessor
{
public:
	virtual ~MenuAccessor() {}

	virtual QList < QVariantHash >* getAccountMenuParam() = 0; //should return 0 if you don't need
	virtual QList < QVariantHash >* getContactMenuParam() = 0; //should return 0 if you don't need

};

Q_DECLARE_INTERFACE(MenuAccessor, "org.psi-im.MenuAccessor/0.1");

#endif
