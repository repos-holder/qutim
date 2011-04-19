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
class TreeModelPrivate;
class TreeModel;

struct ChangeEvent;
class ContactItem;

class TagItem : protected ItemHelper
{
public:
	inline TagItem() : ItemHelper(TagType), online(0) {}
	inline TreeModelPrivate *getTagContainer(AbstractContactModel *m);
	inline void setTagContainer(void *) { }
	QList<ContactItem *> visible;
	int online;
	QString name;
	QList<ContactItem *> contacts;
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
	inline int index() const { return parent->contacts.indexOf(const_cast<ContactItem *>(this)); }
	TagItem *parent;
	ContactData::Ptr data;
};

class TreeModel : public AbstractContactModel
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(TreeModel)
public:
	TreeModel(QObject *parent = 0);
	virtual ~TreeModel();
	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool containsContact(Contact *contact) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	Qt::DropActions supportedDropActions() const;
	QStringList mimeTypes() const;
	QMimeData *mimeData(const QModelIndexList &indexes) const;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action,
					  int row, int column, const QModelIndex &parent);
	bool showOffline() const;
	QStringList tags() const;
	QStringList selectedTags() const;

	void processEvent(ChangeEvent *ev);
	bool eventFilter(QObject *obj, QEvent *ev);
public slots:
	void addContact(qutim_sdk_0_3::Contact *contact);
	void removeContact(qutim_sdk_0_3::Contact *contact);
	void filterList(const QStringList &tags);
	void filterList(const QString &filter);
protected slots:
	void contactDeleted(QObject *obj);
	void contactStatusChanged(qutim_sdk_0_3::Status status);
	void contactNameChanged(const QString &name);
	void contactTagsChanged(const QStringList &tags);
	void hideShowOffline();
	void onContactInListChanged(bool isInList);
	void onSessionCreated(qutim_sdk_0_3::ChatSession *session);
	void onUnreadChanged(const qutim_sdk_0_3::MessageList &messages);
	void onAccountCreated(qutim_sdk_0_3::Account *);
protected:
	void timerEvent(QTimerEvent *ev);
private:
	friend class TagItem;
	void filterAllList();
	bool isVisible(ContactItem *item);
	void removeFromContactList(Contact *contact, bool deleted);
	void initialize();
	void saveTagOrder();
};

TreeModelPrivate *TagItem::getTagContainer(AbstractContactModel *m)
{
	return reinterpret_cast<TreeModel*>(m)->d_func();
}

}
}

#endif // SIMPLECONTACTLISTMODEL_H
