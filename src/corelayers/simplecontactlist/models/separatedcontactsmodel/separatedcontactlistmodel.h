#ifndef SIMPLECONTACTLISTMODEL_H
#define SIMPLECONTACTLISTMODEL_H

#include <QAbstractItemModel>
#include <simplecontactlistitem.h>
#include <qutim/messagesession.h>
#include <abstractcontactmodel.h>

namespace Core
{
namespace SimpleContactList
{
class SeparatedModelPrivate;
class SeparatedModel;

struct ChangeEvent;
class ContactItem;
class TagItem;

class AccountItem : public ItemHelper
{
public:
	inline AccountItem() : ItemHelper(AccountType) {}
	Account *account;
	QString id;
	QList<TagItem*> tags;
	QList<TagItem *> visibleTags;
	QHash<QString, TagItem *> tagsHash;
};

class TagItem : public ItemHelper
{
public:
	inline TagItem() : ItemHelper(TagType), online(0) {}
	inline AccountItem *getTagContainer(void*) { return parent; }
	inline void setTagContainer(AccountItem *p) { parent = p; }
	inline QModelIndex parentIndex(AbstractContactModel *m);
	QString getName();
	QList<ContactItem *> visible;
	int online;
	QString name;
	QList<ContactItem *> contacts;
	AccountItem *parent;
};

class ContactData : public QSharedData
{
public:
	typedef QExplicitlySharedDataPointer<ContactData> Ptr;
	inline ContactData() {}
	inline ContactData(const ContactData &other)
		: QSharedData(other), contact(other.contact), tags(other.tags), items(other.items) {}
	inline ~ContactData() {}
	Contact *contact;
	QSet<QString> tags;
	QList<ContactItem *> items;
	Status status;
};

class ContactItem  : public ItemHelper
{
public:
	inline ContactItem() : ItemHelper(ContactType) {}
	inline ContactItem(const ContactData::Ptr &other_data) : ItemHelper(ContactType), data(other_data) {}
	inline ContactItem(const ContactItem &other) : ItemHelper(ContactType), parent(other.parent), data(other.data) {}
	inline int index() { return parent->visible.indexOf(this); }
	inline QModelIndex parentIndex(AbstractContactModel *m);
	inline Contact *getContact() { return data->contact; }
	inline Status getStatus() { return data->status; }
	inline QList<ContactItem*> &siblings(void*) { return parent->visible; }
	inline bool isInSelectedTag(const QSet<QString> &selectedTags) { return selectedTags.contains(parent->name); }
	TagItem *parent;
	ContactData::Ptr data;
};

class SeparatedModel : public AbstractContactModel
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(SeparatedModel)
	Q_CLASSINFO("SettingsDescription", "Show accounts, tags and contacts")
public:
	SeparatedModel(QObject *parent = 0);
	virtual ~SeparatedModel();
	virtual QList<Contact*> contacts() const;
	virtual void setContacts(const QList<qutim_sdk_0_3::Contact*> &contacts);
	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	bool containsContact(Contact *contact) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	QStringList mimeTypes() const;
	QMimeData *mimeData(const QModelIndexList &indexes) const;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action,
					  int row, int column, const QModelIndex &parent);
	QStringList tags() const;
public slots:
	void addContact(qutim_sdk_0_3::Contact *contact);
	void removeContact(qutim_sdk_0_3::Contact *contact);
protected slots:
	void contactDeleted(QObject *obj);
	void contactStatusChanged(qutim_sdk_0_3::Status status);
	void contactNameChanged(const QString &name);
	void contactTagsChanged(const QStringList &tags);
	void onContactInListChanged(bool isInList);
	AccountItem *onAccountCreated(qutim_sdk_0_3::Account *);
	void onAccountDestroyed(QObject *);
	void init();
protected:
	AccountItem *addAccount(qutim_sdk_0_3::Account *account, bool addContacts);
	void filterAllList();
	void updateContactData(Contact *contact);
	void processEvent(ChangeEvent *ev);
	bool eventFilter(QObject *obj, QEvent *ev);
private:
	friend class TagItem;
	friend class ContactItem;
	void removeFromContactList(Contact *contact, bool deleted);
	void saveTagOrder(AccountItem *accountItem);
};

}
}

#endif // SIMPLECONTACTLISTMODEL_H
