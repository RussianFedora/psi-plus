#ifndef CONTACTSTATEACCESSINGHOST_H
#define CONTACTSTATEACCESSINGHOST_H

class ContactStateAccessingHost
{
public:
	virtual ~ContactStateAccessingHost() {}

	virtual bool setActivity(int account, QString Jid, QDomElement xml) = 0;
	virtual bool setMood(int account, QString Jid, QDomElement xml) = 0;
	virtual bool setTune(int account, QString Jid, QString tune) = 0;
};

Q_DECLARE_INTERFACE(ContactStateAccessingHost, "org.psi-im.ContactStateAccessingHost/0.2");

#endif // CONTACTSTATEACCESSINGHOST_H
