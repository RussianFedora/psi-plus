/*
 * (c) 2006 Kevin Smith
 * (c) 2008 Maciej Niedzielski
 */

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore>
#include <QList>
#include <QMap>
#include <QHash>
#include <QDomElement>
#include <QMenu>

class QPluginLoader;

class PsiAccount;
class PsiPlugin;
class PluginHost;

namespace XMPP {
	class Client;
}

namespace QCA {
	class DirWatch;
}


class PluginManager : public QObject
{
	Q_OBJECT

public:
	static PluginManager* instance();
	void initNewSession();

	QStringList availablePlugins();

	void addAccount(PsiAccount* account, XMPP::Client* client);

	void loadEnabledPlugins();
	void loadUnloadPlugin(QString plugin, bool load);
	bool unloadAllPlugins();

	QString pathToPlugin(const QString& plugin);
	QString shortName(const QString& plugin);
	QString version(const QString& plugin);
	QWidget* optionsWidget(const QString& plugin);

	void setShortcuts();

	bool processEvent(PsiAccount* account, QDomElement& eventXml);
	bool processMessage(PsiAccount* account, const QString& jidFrom, const QString& body, const QString& subject);
	bool processOutgoingMessage(PsiAccount* account, const QString& jidTo, QString& body, const QString& type, QString& subject);
	void processOutgoingStanza(PsiAccount* account, QDomElement &stanza);
	void logout(PsiAccount* account);

	void applyOptions(const QString& plugin);
	void restoreOptions(const QString& plugin);
	void addToolBarButton(QObject* parent, QWidget* toolbar, PsiAccount* account, const QString& contact);
	void addGCToolBarButton(QObject* parent, QWidget* toolbar, PsiAccount* account, const QString& contact);
	void addAccountMenu(QMenu *menu, PsiAccount* account);
	void addContactMenu(QMenu *menu, PsiAccount* account, QString jid);

	QString pluginInfo(const QString& plugin);
	bool hasInfoProvider(const QString& plugin);

	static const QString loadOptionPrefix;
	static const QString pluginOptionPrefix;
	
private:
	PluginManager();
	void loadAllPlugins();
	bool verifyStanza(const QString& stanza);
	void updatePluginsList();

	static PluginManager* instance_;

	//account id, client
	QVector<XMPP::Client*> clients_;

	//account, account id
	QHash<PsiAccount*, int> accountIds_;

	//name, host
	QMap<QString, PluginHost*> hosts_;
	//file, host
	QMap<QString, PluginHost*> pluginByFile_;
	//sorted by priority
	QList<PluginHost*> pluginsByPriority_;

	
	QList<QCA::DirWatch*> dirWatchers_;

	// Options widget provides by plugin on opt_plugins
	QPointer<QWidget> optionsWidget_;

	class StreamWatcher;
	bool incomingXml(int account, const QDomElement &eventXml);
	void sendXml(int account, const QString& xml);
	QString uniqueId(int account);
	
	QString getStatus(int account);
	QString getStatusMessage(int account);
	QString proxyHost(int account);
	int proxyPort(int account);
	QString proxyUser(int account);
	QString proxyPassword(int account);
	QStringList getRoster(int account);
	QString getJid(int account);

	bool isSelf(int account, const QString& jid);
	bool isAgent(int account, const QString& jid);
	bool inList(int account, const QString& jid);
	bool isPrivate(int account, const QString& jid);
	bool isConference(int account, const QString& jid);
	QString name(int account, const QString& jid);
	QString status(int account, const QString& jid);

	bool setActivity(int account, QString Jid, QDomElement xml);
	bool setMood(int account, QString Jid, QDomElement xml);
	bool setTune(int account, QString Jid, QString tune);

	void initPopup(QString text, QString title, QString icon);

	void setStatus(int account, QString status, QString statusMessage);

	void createNewEvent(int account, const QString& jid, const QString& descr, QObject *receiver, const char* slot);

	friend class PluginHost;
	
private slots:
	void dirsChanged();
	void optionChanged(const QString& option);
};

#endif
