#ifndef CONTACTMANAGERMODEL_H
#define CONTACTMANAGERMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QSet>

class UserListItem;
class PsiAccount;
namespace XMPP
{
	class RosterItem;
	class Jid;
}
using namespace XMPP;

class ContactManagerModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	enum Role {
		// DisplayRole / EditRole
		CheckRole,
		NickRole,
		GroupRole,
		NodeRole,
		DomainRole,
		SubscriptionRole
	};
	const static int SimpleMatch = 1;
	const static int RegexpMatch = 2;

	ContactManagerModel(QObject * parent, PsiAccount *pa);
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						 int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags ( const QModelIndex & index ) const;
	bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	void sort ( int column, Qt::SortOrder order );
	static bool sortLessThan(UserListItem *u1, UserListItem *u2);
	static Role sortRole;
	static Qt::SortOrder sortOrder;


	QStringList manageableFields();
	void reloadUsers();
	void clear();
	void addContact(UserListItem *u);
	QList<UserListItem *> checkedUsers();
	void invertByMatch(int columnIndex, int matchType, const QString &str);

	void startBatch() { layoutAboutToBeChanged(); }
	void stopBatch() { layoutChanged(); }


private:
	PsiAccount *pa_;
	QList<UserListItem *> _userList;
	QStringList columnNames;
	QList<Role> roles;
	QSet<QString> checks;

	QString userFieldString(UserListItem *u, ContactManagerModel::Role columnRole) const;
	void contactUpdated(const Jid &);

private slots:
	void view_contactUpdated(const UserListItem &);
	void client_rosterItemUpdated(const RosterItem &);
};

#endif // CONTACTMANAGERMODEL_H
