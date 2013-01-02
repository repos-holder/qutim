/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2012 Ruslan Nigmatullin <euroelessar@yandex.ru>
**
*****************************************************************************
**
** $QUTIM_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $QUTIM_END_LICENSE$
**
****************************************************************************/

#ifndef CONTACTLISTMODELBASE_H
#define CONTACTLISTMODELBASE_H

#include <QAbstractItemModel>
#include <qutim/account.h>
#include <qutim/contact.h>
#include <qutim/servicemanager.h>

class ContactListFrontModel;

enum ContactListItemRole
{
	BuddyRole = Qt::UserRole,
	StatusRole,
	ContactsCountRole,
	OnlineContactsCountRole,
	AvatarRole,
	ItemTypeRole,
	AccountRole,
	ColorRole,
	TagNameRole,
	NotificationRole
};

enum ContactListItemType
{
	InvalidType = 0,
	TagType = 100,
	ContactType = 101,
	AccountType = 102
};

class ContactListBaseModel : public QAbstractItemModel
{
    Q_OBJECT
	Q_CLASSINFO("Service", "ContactBackendModel")
	Q_CLASSINFO("RuntimeSwitch", "yes")
	Q_CLASSINFO("Uses", "ContactComparator")
	Q_CLASSINFO("SettingsDescription", "Blank model")
	Q_PROPERTY(QStringList tags READ tags NOTIFY tagsChanged)
public:
    explicit ContactListBaseModel(QObject *parent = 0);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	QStringList tags() const;

	virtual void updateContactTags(qutim_sdk_0_3::Contact *contact,
								   const QStringList &current,
								   const QStringList &previous);
	virtual void addAccount(qutim_sdk_0_3::Account *account);
	virtual void removeAccount(qutim_sdk_0_3::Account *account);
	virtual void addContact(qutim_sdk_0_3::Contact *contact) = 0;
	virtual void removeContact(qutim_sdk_0_3::Contact *contact) = 0;

protected:
	class AccountNode;
	class ContactListNode;
	class TagListNode;
	class AccountListNode;

	enum NodeType
	{
		ContactListNodeType     = 0x01,
		TagListNodeType         = 0x02 | ContactListNodeType,
		AccountListNodeType     = 0x04 | TagListNodeType,
		ProtocolListNodeType    = 0x08 | AccountListNodeType,
		ContactNodeType         = 0x10,
		TagNodeType             = 0x20 | ContactListNodeType,
		AccountNodeType         = 0x40 | TagListNodeType,
		RootNodeType            = 0x80 | AccountListNodeType
	};

    class BaseNode
	{
    public:
		inline BaseNode(NodeType type, BaseNode *parent) : m_type(type), m_parent(parent) {}

		inline NodeType type() const { return m_type; }
		inline BaseNode *parent() const { return m_parent; }

	private:
		NodeType m_type;
		BaseNode *m_parent;
	};

	class ContactNode : public BaseNode
	{
	public:
		enum { Type = ContactNodeType };

		inline ContactNode(qutim_sdk_0_3::Contact *contact, BaseNode &parent)
			: BaseNode(ContactNodeType, &parent), contact(contact) {}

		inline ContactListNode *parent() const { return static_cast<ContactListNode*>(BaseNode::parent()); }
		inline bool operator  <(const ContactNode &other) const { return contact  < other.contact; }
		inline bool operator ==(const ContactNode &other) const { return contact == other.contact; }

		qutim_sdk_0_3::Contact *contact;
	};

	class ContactListNode : public BaseNode
	{
	public:
		enum { Type = ContactListNodeType };

		inline ContactListNode(NodeType type, BaseNode *parent) : BaseNode(type, parent) {}

		QList<ContactNode> contacts;
		QHash<qutim_sdk_0_3::Contact*, int> onlineContacts;
		QHash<qutim_sdk_0_3::Contact*, int> totalContacts;
	};

	class TagNode : public ContactListNode
	{
	public:
		enum { Type = TagNodeType };

		explicit inline TagNode(const QString &name, BaseNode &parent)
			: ContactListNode(TagNodeType, &parent), name(name) {}

		inline TagListNode *parent() const { return static_cast<TagListNode*>(BaseNode::parent()); }
		inline bool operator  <(const TagNode &other) const { return name  < other.name; }
		inline bool operator ==(const TagNode &other) const { return name == other.name; }

		QString name;
	};

	class TagListNode : public ContactListNode
	{
	public:
		enum { Type = TagListNodeType };

		inline TagListNode(NodeType type, BaseNode *parent) : ContactListNode(type, parent) {}

		QList<TagNode> tags;
	};

	class AccountNode : public TagListNode
    {
    public:
		enum { Type = AccountNodeType };

		explicit inline AccountNode(qutim_sdk_0_3::Account *account, BaseNode &parent)
			: TagListNode(AccountNodeType, &parent), account(account) {}

		inline AccountListNode *parent() const { return static_cast<AccountListNode*>(BaseNode::parent()); }

		inline bool operator  <(const AccountNode &other) const { return account  < other.account; }
		inline bool operator ==(const AccountNode &other) const { return account == other.account; }

		qutim_sdk_0_3::Account *account;
	};

	class AccountListNode : public TagListNode
	{
	public:
		enum { Type = AccountListNodeType };

		inline AccountListNode(NodeType type, BaseNode *parent) : TagListNode(type, parent) {}

		QList<AccountNode> accounts;
	};

	class RootNode : public AccountListNode
	{
	public:
		enum { Type = RootNodeType };

		inline RootNode() : AccountListNode(RootNodeType, NULL) {}
	};

	AccountNode *ensureAccount(qutim_sdk_0_3::Account *account, AccountListNode *parent);
	void eraseAccount(qutim_sdk_0_3::Account *account, AccountListNode *parent);
	TagNode *ensureTag(const QString &name, TagListNode *parent);
	ContactNode *ensureContact(qutim_sdk_0_3::Contact *contact, ContactListNode *parent);
	void eraseContact(qutim_sdk_0_3::Contact *contact, ContactListNode *parent);

	RootNode *rootNode() const;
	QStringList emptyTags() const;
	QStringList fixTags(const QStringList &tags) const;

signals:
	void tagsChanged(const QStringList &tags);

private slots:
	void onAccountDestroyed(QObject *obj);
	void onContactDestroyed(QObject *obj);
	void onContactAdded(qutim_sdk_0_3::Contact *contact);
	void onContactRemoved(qutim_sdk_0_3::Contact *contact);
	void onContactChanged(qutim_sdk_0_3::Contact *contact);
	void onContactChanged();
	void onContactTagsChanged(const QStringList &current, const QStringList &previous);
	void onStatusChanged(const qutim_sdk_0_3::Status &current, const qutim_sdk_0_3::Status &previous);
	void onAccountCreated(qutim_sdk_0_3::Account *account, bool addContacts = true);

	void connectContact(qutim_sdk_0_3::Contact *contact);
	void disconnectContact(qutim_sdk_0_3::Contact *contact);

private:
	class Comparator
	{
	public:
		inline bool operator() (const AccountNode &first, qutim_sdk_0_3::Account *second) const
		{ return first.account < second; }
		inline bool operator() (qutim_sdk_0_3::Account *first, const AccountNode &second) const
		{ return first < second.account; }
		inline bool operator() (const TagNode &first, const QString &second) const
		{ return first.name < second; }
		inline bool operator() (const QString &first, const TagNode &second) const
		{ return first < second.name; }
		inline bool operator() (const ContactNode &first, qutim_sdk_0_3::Contact *second) const
		{ return first.contact < second; }
		inline bool operator() (qutim_sdk_0_3::Contact *first, const ContactNode &second) const
		{ return first < second.contact; }
	};

	bool findNode(BaseNode *node) const;
	bool findNode(BaseNode *node, BaseNode *current) const;

	void findContacts(QSet<qutim_sdk_0_3::Contact*> &contacts, BaseNode *current);
	void addTags(const QStringList &tags);

	void updateItemCount(qutim_sdk_0_3::Contact *contact, ContactListNode *parent, int online, int total);
	void removeAccountNode(qutim_sdk_0_3::Account *account, BaseNode *parent);
	void clearContacts(BaseNode *parent);

	QModelIndex createIndex(BaseNode *node) const;
	inline QModelIndex createIndex(BaseNode &node, int row) const
	{ return QAbstractItemModel::createIndex(row, 0, &node); }
	BaseNode *extractNode(const QModelIndex &index) const;
	template <typename T> static inline T node_cast(BaseNode *node)
	{ return (node && (node->type() & reinterpret_cast<T>(NULL)->Type) == reinterpret_cast<T>(NULL)->Type) ? static_cast<T>(node) : NULL; }
	template <typename T> inline T *extractNode(const QModelIndex &index) const
	{ return node_cast<T*>(extractNode(index)); }

	typedef QHash<qutim_sdk_0_3::Contact*, QList<ContactNode *> > ContactHash;
	friend class ContactListFrontModel;

	RootNode m_root;
	ContactHash m_contactHash;
	mutable QStringList m_emptyTags;
	QStringList m_tags;
    qutim_sdk_0_3::ServicePointer<qutim_sdk_0_3::ContactComparator> m_comparator;
};

#endif // CONTACTLISTMODELBASE_H
