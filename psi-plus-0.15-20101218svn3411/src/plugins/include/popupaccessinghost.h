#ifndef POPUPACCESSINGHOST_H
#define POPUPACCESSINGHOST_H

class PopupAccessingHost
{
public:
	virtual ~PopupAccessingHost() {}

	virtual void initPopup(QString text, QString title, QString icon = "psi/headline") = 0;
};

Q_DECLARE_INTERFACE(PopupAccessingHost, "org.psi-im.PopupAccessingHost/0.1");

#endif
