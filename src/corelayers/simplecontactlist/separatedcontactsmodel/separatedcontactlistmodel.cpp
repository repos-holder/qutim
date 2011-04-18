#include "separatedcontactlistmodel.h"
#include <qutim/notificationslayer.h>
#include <qutim/messagesession.h>
#include <qutim/status.h>
#include <qutim/icon.h>
#include <qutim/configbase.h>
#include <qutim/debug.h>
#include <qutim/metacontact.h>
#include <qutim/metacontactmanager.h>
#include <qutim/mimeobjectdata.h>
#include <qutim/event.h>
#include <qutim/systemintegration.h>
#include <qutim/protocol.h>
#include <QBasicTimer>
#include <QMimeData>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>

#define QUTIM_MIME_CONTACT_INTERNAL QLatin1String("application/qutim-contact-internal")
#define QUTIM_MIME_TAG_INTERNAL QLatin1String("application/qutim-tag-internal")

namespace Core
{

namespace SimpleContactList
{
struct ChangeEvent
{
	enum Type { ChangeTags, MergeContacts, MoveTag } type;
	void *child;
	ItemHelper *parent;

};

struct SeparatedModelPrivate
{
	QList<AccountItem*> accounts;
	QHash<Account*, AccountItem*> accountHash;
	QMap<Contact *, ContactData::Ptr> contacts;
	QList<ChangeEvent*> events;
	QBasicTimer timer;
	QString lastFilter;
	QStringList selectedTags;
	bool showOffline;
	QBasicTimer unreadTimer;
	QMap<ChatSession*, QSet<Contact*> > unreadBySession;
	QSet<Contact*> unreadContacts;
	bool showMessageIcon;
	QIcon unreadIcon;
	quint16 realUnitRequestEvent;

	struct InitData
	{
		QList<Contact*> contacts;
		quint16 qutimStartupEvent;
	};
	// Pointer to variables that are solely used at startup.
	// See Model::initialize()
	InitData *initData;
};

SeparatedModel::SeparatedModel(QObject *parent) : AbstractContactModel(parent), p(new SeparatedModelPrivate)
{
	p->showMessageIcon = false;
	Event::eventManager()->installEventFilter(this);
	p->initData = new SeparatedModelPrivate::InitData;
	p->realUnitRequestEvent = Event::registerType("real-chatunit-request");
	p->initData->qutimStartupEvent = Event::registerType("startup");
	p->unreadIcon = Icon(QLatin1String("mail-unread-new"));
	ConfigGroup group = Config().group("contactList");
	p->showOffline = group.value("showOffline", true);
}

SeparatedModel::~SeparatedModel()
{
}

QModelIndex SeparatedModel::index(int row, int, const QModelIndex &parent) const
{
	if(row < 0)
		return QModelIndex();
	switch(getItemType(parent))
	{
	case AccountType: {
		AccountItem *item = reinterpret_cast<AccountItem *>(parent.internalPointer());
		if(item->tags.size() <= row)
			return QModelIndex();
		return createIndex(row, 0, item->tags.at(row));
	}
	case TagType: {
		TagItem *item = reinterpret_cast<TagItem *>(parent.internalPointer());
		if(item->visible.size() <= row)
			return QModelIndex();
		return createIndex(row, 0, item->visible.at(row));
	}
	case ContactType:
		return QModelIndex();
	default:
		if(p->accounts.size() <= row)
			return QModelIndex();
		return createIndex(row, 0, p->accounts.at(row));
	}
}

QModelIndex SeparatedModel::parent(const QModelIndex &child) const
{
	switch(getItemType(child))
	{
	case ContactType: {
		ContactItem *item = reinterpret_cast<ContactItem *>(child.internalPointer());
		return createIndex(item->parent->parent->visibleTags.indexOf(item->parent),
						   0, item->parent);
	}
	case TagType: {
		TagItem *item = reinterpret_cast<TagItem *>(child.internalPointer());
		return createIndex(p->accounts.indexOf(item->parent), 0, item->parent);
	} default:
		return QModelIndex();
	}
}

int SeparatedModel::rowCount(const QModelIndex &parent) const
{
	switch(getItemType(parent))
	{
	case TagType:
		return reinterpret_cast<TagItem *>(parent.internalPointer())->visible.size();
	case ContactType:
		return 0;
	case AccountType:
		return reinterpret_cast<AccountItem *>(parent.internalPointer())->visibleTags.size();
	default:
		return p->accounts.size();
	}
}

int SeparatedModel::columnCount(const QModelIndex &) const
{
	return 1;
}

bool SeparatedModel::hasChildren(const QModelIndex &parent) const
{
	switch(getItemType(parent))
	{
	case TagType:
		return !reinterpret_cast<TagItem *>(parent.internalPointer())->visible.isEmpty();
	case ContactType:
		return false;
	case AccountType:
		return !reinterpret_cast<AccountItem *>(parent.internalPointer())->visibleTags.isEmpty();
	default:
		return !p->accounts.isEmpty();
	}
}

QVariant SeparatedModel::data(const QModelIndex &index, int role) const
{
	switch(getItemType(index))
	{
	case ContactType:
	{
		ContactItem *item = reinterpret_cast<ContactItem *>(index.internalPointer());
		switch(role)
		{
		case Qt::EditRole:
		case Qt::DisplayRole: {
			QString name = item->data->contact->name();
			return name.isEmpty() ? item->data->contact->id() : name;
		}
		case Qt::DecorationRole:
			if (p->showMessageIcon && p->unreadContacts.contains(item->data->contact))
				return p->unreadIcon;
			else
				return item->data->contact->status().icon();
		case ItemTypeRole:
			return ContactType;
		case StatusRole:
			return qVariantFromValue(item->data->contact->status());
		case AvatarRole:
			return item->data->contact->avatar();
		case BuddyRole: {
			ContactItem *item = reinterpret_cast<ContactItem*>(index.internalPointer());
			Buddy *buddy = item->data->contact;
			return qVariantFromValue(buddy);
		}
		default:
			return QVariant();
		}
	}
	case TagType:
	{
		TagItem *item = reinterpret_cast<TagItem *>(index.internalPointer());
		switch(role)
		{
		case Qt::DisplayRole:
			return item->name;
		case ItemTypeRole:
			return TagType;
		case Qt::DecorationRole:
			return Icon("feed-subscribe");
		case ContactsCountRole:
			return item->contacts.count();
		case OnlineContactsCountRole:
			return item->online;
		default:
			return QVariant();
		}
	}
	case AccountType:
	{
		AccountItem *item = reinterpret_cast<AccountItem *>(index.internalPointer());
		switch(role)
		{
		case Qt::DisplayRole:
			return item->account->name() +
					QLatin1String(" (") +
					item->account->id() +
					QLatin1String(")");
		case ItemTypeRole:
			return AccountType;
		case Qt::DecorationRole:
			return item->account->status().icon();
		default:
			return QVariant();
		}
	}
	default:
		return QVariant();
	}
}

QVariant SeparatedModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section==0) {
		if (p->selectedTags.isEmpty())
			return tr("All tags");
		else
			return tr("Custom tags");
	}

	return QVariant();
}

bool contactLessThan (ContactItem *a, ContactItem *b) {
	int result;

	//int unreadA = 0;
	//int unreadB = 0;
	//ChatSession *session = ChatLayer::get(a->data->contact,false);
	//if(session)
	//	unreadA = session->unread().count();
	//session = ChatLayer::get(b->data->contact,false);
	//if(session)
	//	unreadB = session->unread().count();
	//result = unreadA - unreadB;
	//if(result)
	//	return result < 0;

	result = a->data->status.type() - b->data->status.type();
	if (result)
		return result < 0;
	return a->data->contact->title().compare(b->data->contact->title(), Qt::CaseInsensitive) < 0;
};

void SeparatedModel::addContact(Contact *contact)
{
	//TODO implement more powerfull logic
	//			if (!contact->isInList())
	//				return;

	if (p->initData) {
		if (p->initData->contacts.contains(contact))
			return;
		p->initData->contacts << contact;
		return;
	}

	if (p->contacts.contains(contact))
		return;

	connect(contact, SIGNAL(destroyed(QObject*)),
			SLOT(contactDeleted(QObject*)));
	connect(contact, SIGNAL(statusChanged(qutim_sdk_0_3::Status,qutim_sdk_0_3::Status)),
			SLOT(contactStatusChanged(qutim_sdk_0_3::Status)));
	connect(contact, SIGNAL(nameChanged(QString,QString)),
			SLOT(contactNameChanged(QString)));
	connect(contact, SIGNAL(tagsChanged(QStringList,QStringList)),
			SLOT(contactTagsChanged(QStringList)));
	connect(contact, SIGNAL(inListChanged(bool)),
			SLOT(onContactInListChanged(bool)));

	QStringList tags = contact->tags();
	if(tags.isEmpty())
		tags << tr("Without tags");

	AccountItem *accountItem = p->accountHash.value(contact->account());
	if (!accountItem)
		accountItem = onAccountCreated(contact->account());

	ContactData::Ptr item_data(new ContactData);
	item_data->contact = contact;
	item_data->tags = QSet<QString>::fromList(tags);
	item_data->status = contact->status();
	int counter = item_data->status.type() == Status::Offline ? 0 : 1;
	p->contacts.insert(contact, item_data);
	for(QSet<QString>::const_iterator it = item_data->tags.constBegin(); it != item_data->tags.constEnd(); it++)
	{
		TagItem *tag = ensureTag<TagItem>(accountItem, *it);
		ContactItem *item = new ContactItem(item_data);
		item->parent = tag;
		bool show = isVisible(item);
		tag->online += counter;
		if (show) {
			hideContact(item, false, false);
		} else {
			tag->contacts.append(item);
			item_data->items.append(item);
		}
	}
}

bool SeparatedModel::containsContact(Contact *contact) const
{
	return p->contacts.contains(contact);
}

bool SeparatedModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::EditRole && getItemType(index) == ContactType) {
		ContactItem *item = reinterpret_cast<ContactItem *>(index.internalPointer());
		item->data->contact->setName(value.toString());
		return true;
	}
	return false;
}

Qt::ItemFlags SeparatedModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);

	ContactItemType type = getItemType(index);
	flags |= Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
	if (type == ContactType)
		flags |= Qt::ItemIsEditable;

	return flags;
}

Qt::DropActions SeparatedModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}

QStringList SeparatedModel::mimeTypes() const
{
	QStringList types;
	types << QUTIM_MIME_CONTACT_INTERNAL;
	types << QUTIM_MIME_TAG_INTERNAL;
	types << MimeObjectData::objectMimeType();
	return types;
}

QMimeData *SeparatedModel::mimeData(const QModelIndexList &indexes) const
{
	MimeObjectData *mimeData = new MimeObjectData();
	QModelIndex index = indexes.value(0);
	ContactItemType itemType = getItemType(index);
	QLatin1String type("");
	if (itemType == ContactType) {
		ContactItem *item = reinterpret_cast<ContactItem*>(index.internalPointer());
		mimeData->setText(item->data->contact->id());
		type = QUTIM_MIME_CONTACT_INTERNAL;
		mimeData->setObject(item->data->contact);
	} else if (itemType == TagType) {
		TagItem *item = reinterpret_cast<TagItem*>(index.internalPointer());
		mimeData->setText(item->name);
		type = QUTIM_MIME_TAG_INTERNAL;
	} else {
		return mimeData;
	}

	QByteArray encodedData;
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	stream << index.row() << index.column() << qptrdiff(index.internalPointer());
	mimeData->setData(type, encodedData);

	return mimeData;
}

bool SeparatedModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
						 int row, int column, const QModelIndex &parent)
{
	if (action == Qt::IgnoreAction)
		return true;

	ContactItemType parentType = getItemType(parent);
	if (parentType != ContactType && parentType != TagType)
		return false;

	qptrdiff internalId = 0;
	QByteArray encodedData;
	bool isContact = data->hasFormat(QUTIM_MIME_CONTACT_INTERNAL);
	if (isContact)
		encodedData = data->data(QUTIM_MIME_CONTACT_INTERNAL);
	else if (data->hasFormat(QUTIM_MIME_TAG_INTERNAL))
		encodedData = data->data(QUTIM_MIME_TAG_INTERNAL);
	else
		return false;

	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	stream >> row >> column >> internalId;
	QModelIndex index = createIndex(row, column, reinterpret_cast<void*>(internalId));
	if (isContact && getItemType(index) != ContactType)
		return false;

	ChangeEvent *ev = new ChangeEvent;
	ev->child = reinterpret_cast<ItemHelper*>(index.internalPointer());
	ev->parent = reinterpret_cast<ItemHelper*>(parent.internalPointer());
	if (getItemType(index) == TagType)
		ev->type = ChangeEvent::MoveTag;
	else if (getItemType(parent) == TagType)
		ev->type = ChangeEvent::ChangeTags;
	else if(getItemType(parent) == ContactType)
		ev->type = ChangeEvent::MergeContacts;
	p->events << ev;
	p->timer.start(1, this);

	return true;
	// We should return false
	//			return false;
}

void SeparatedModel::removeFromContactList(Contact *contact, bool deleted)
{
	Q_UNUSED(deleted);
	ContactData::Ptr item_data = p->contacts.value(contact);
	if(!item_data)
		return;
	int counter = item_data->status.type() == Status::Offline ? 0 : -1;
	for(int i = 0; i < item_data->items.size(); i++) {
		ContactItem *item = item_data->items.at(i);
		item->parent->online += counter;
		hideContact(item, true, false);
		delete item;
	}
	p->contacts.remove(contact);
	p->unreadContacts.remove(contact);
}

void SeparatedModel::contactDeleted(QObject *obj)
{
	Contact *contact = reinterpret_cast<Contact *>(obj);
	removeFromContactList(contact,true);
}

void SeparatedModel::removeContact(Contact *contact)
{
	Q_ASSERT(contact);
	if (MetaContact *meta = qobject_cast<MetaContact*>(contact)) {
		contact->removeEventFilter(this);
		foreach (ChatUnit *unit, meta->lowerUnits()) {
			Contact *subContact = qobject_cast<Contact*>(unit);
			if (subContact && !p->contacts.contains(subContact))
				addContact(subContact);
		}
	}
	contact->disconnect(this);
	removeFromContactList(contact,false);
}

void SeparatedModel::contactStatusChanged(Status status)
{
	Contact *contact = qobject_cast<Contact *>(sender());
	ContactData::Ptr item_data = p->contacts.value(contact);
	if(!item_data)
		return;
	bool statusTypeChanged = status.type() != item_data->status.type();
	int counter = 0;
	if (status.type() == Status::Offline)
		counter = statusTypeChanged ? -1 : 0;
	else if (item_data->status == Status::Offline)
		counter = +1;
	item_data->status = status;
	const QList<ContactItem *> &items = item_data->items;
	bool show = isVisible(item_data->items.value(0));
	for(int i = 0; i < items.size(); i++) {
		ContactItem *item = items.at(i);
		item->parent->online += counter;

		if (!hideContact(item, !show)) {
			if (!show)
				// The item is already hidden and it should stay that way.
				return;
		} else {
			// Depending on 'show' the item has been either added to the model
			// or removed from it in hideContact method.
			// Either way, we have nothing to do anymore.
			return;
		}

		// The item is already visible, so we need to move it in the right place
		// and update its content
		updateContact<AccountItem>(item, statusTypeChanged);
	}
}

void SeparatedModel::contactNameChanged(const QString &name)
{
	Q_UNUSED(name);
	Contact *contact = qobject_cast<Contact *>(sender());
	ContactData::Ptr item_data = p->contacts.value(contact);
	if(!item_data)
		return;
	const QList<ContactItem *> &items = item_data->items;
	if (items.isEmpty() || !isVisible(items.first()))
		return;
	for(int i = 0; i < items.size(); i++)
		updateContact<AccountItem>(items.at(i), true);
}

void SeparatedModel::onContactInListChanged(bool)
{
	//Contact *contact = qobject_cast<Contact*>(sender());
	//p->contacts.value(contact)->
	//TODO
}

void SeparatedModel::onSessionCreated(qutim_sdk_0_3::ChatSession *session)
{
	connect(session, SIGNAL(unreadChanged(qutim_sdk_0_3::MessageList)),
			this, SLOT(onUnreadChanged(qutim_sdk_0_3::MessageList)));
}

void SeparatedModel::onUnreadChanged(const qutim_sdk_0_3::MessageList &messages)
{
	ChatSession *session = qobject_cast<ChatSession*>(sender());
	QSet<Contact*> contacts;
	QSet<ChatUnit*> chatUnits;
	for (int i = 0; i < messages.size(); i++) {
		ChatUnit *unit = messages.at(i).chatUnit();
		if (chatUnits.contains(unit) || !unit)
			continue;
		chatUnits.insert(unit);
		Event event(p->realUnitRequestEvent);
		QCoreApplication::sendEvent(unit, &event);
		Contact *contact = event.at<Contact*>(0);
		while (unit && !contact) {
			if (!!(contact = qobject_cast<Contact*>(unit)))
				break;
			unit = unit->upperUnit();
		}
		if (Contact *meta = qobject_cast<MetaContact*>(contact->metaContact()))
			contact = meta;
		if (contact)
			contacts.insert(contact);
	}
	if (contacts.isEmpty())
		p->unreadBySession.remove(session);
	else
		p->unreadBySession.insert(session, contacts);
	foreach (const QSet<Contact*> &contactsSet, p->unreadBySession)
		contacts |= contactsSet;

	if (!contacts.isEmpty() && !p->unreadTimer.isActive())
		p->unreadTimer.start(500, this);
	else if (contacts.isEmpty())
		p->unreadTimer.stop();

	if (!p->showMessageIcon) {
		p->unreadContacts = contacts;
	} else {
		QSet<Contact*> needUpdate = p->unreadContacts;
		needUpdate.subtract(contacts);
		p->unreadContacts = contacts;
		foreach (Contact *contact, needUpdate) {
			ContactData::Ptr item_data = p->contacts.value(contact);
			for (int i = 0; i < item_data->items.size(); i++) {
				ContactItem *item = item_data->items.at(i);
				QModelIndex index = createIndex(item->index(), 0, item);
				emit dataChanged(index, index);
			}
		}
	}
}

void SeparatedModel::contactTagsChanged(const QStringList &tags_helper)
{
	Contact *contact = qobject_cast<Contact *>(sender());
	Q_ASSERT(contact);
	ContactData::Ptr item_data = p->contacts.value(contact);
	if(!item_data)
		return;
	AccountItem *accountItem = p->accountHash.value(contact->account());
	Q_ASSERT(accountItem);
	bool show = isVisible(item_data->items.value(0));
	QSet<QString> tags;
	tags = QSet<QString>::fromList(tags_helper);
	if(tags.isEmpty())
		tags << tr("Without tags");

	//It should be noted in contactlist those who are not in the roster
	//if(!contact->isInList())
	//	tags << tr("Not in list");

	int counter = item_data->status.type() == Status::Offline ? 0 : 1;
	QSet<QString> to_add = tags - item_data->tags;
	for (int i = 0, size = item_data->items.size(); i < size; i++) {
		ContactItem *item = item_data->items.at(i);
		if(tags.contains(item->parent->name))
			continue;
		item->parent->online -= counter;
		hideContact(item, true, false);
		delete item;
		i--;
		size--;
	}
	for (QSet<QString>::const_iterator it = to_add.constBegin(); it != to_add.constEnd(); it++) {
		TagItem *tag = ensureTag<TagItem>(accountItem, *it);
		tag->online += counter;
		ContactItem *item = new ContactItem(item_data);
		item->parent = tag;
		if (show) {
			hideContact(item, false, false);
		} else {
			tag->contacts.append(item);
			item_data->items.append(item);
		}
	}
	item_data->tags = tags;
}

void SeparatedModel::hideShowOffline()
{
	ConfigGroup group = Config().group("contactList");
	bool show = !group.value("showOffline", true);
	group.setValue("showOffline", show);
	group.sync();
	if (p->showOffline == show)
		return;
	p->showOffline = show;
	filterAllList();
}

void SeparatedModel::filterList(const QString &filter)
{
	if (filter == p->lastFilter)
		return;
	p->lastFilter = filter;
	filterAllList();
}

void SeparatedModel::filterList(const QStringList &tags)
{
	if (tags == p->selectedTags)
		return;
	p->selectedTags = tags;
	filterAllList();
}

QStringList SeparatedModel::tags() const
{
	QStringList all_tags;
	foreach (const AccountItem *acc, p->accounts)
		foreach (const TagItem *tag, acc->tags)
			all_tags.append(tag->name);
	return all_tags;
}

QStringList SeparatedModel::selectedTags() const
{
	return p->selectedTags;
}

void SeparatedModel::processEvent(ChangeEvent *ev)
{
	ContactItem *item = reinterpret_cast<ContactItem*>(ev->child);
	if (ev->type == ChangeEvent::ChangeTags) {
		TagItem *tag = reinterpret_cast<TagItem*>(ev->parent);
		if (tag->name == item->parent->name)
			return;

		QSet<QString> tags = item->data->tags;
		tags.remove(item->parent->name);
		tags.insert(tag->name);
		item->data->contact->setTags(tags.toList());
		debug() << "Moving contact from" << item->data->tags << "to" << tags;
	} else if (ev->type == ChangeEvent::MoveTag) {
		/*int to = -2, globalTo = -2;
		if (ev->parent->type == ContactType) {
			TagItem *tag = reinterpret_cast<ContactItem*>(ev->parent)->parent;
			to = p->visibleTags.indexOf(tag) + 1;
			globalTo = p->tags.indexOf(tag) + 1;
		} else if (ev->parent->type == TagType) {
			TagItem *tag = reinterpret_cast<TagItem*>(ev->parent);
			to = p->visibleTags.indexOf(tag);
			globalTo = p->tags.indexOf(tag);
		} else {
			Q_ASSERT(!"Not implemented");
		}
		TagItem *tag = reinterpret_cast<TagItem*>(ev->child);
		int from = p->visibleTags.indexOf(tag);
		int globalFrom = p->tags.indexOf(tag);
		Q_ASSERT(from >= 0 && to >= 0 && globalTo >= 0 && globalFrom >= 0);
		if (beginMoveRows(QModelIndex(), from, from, QModelIndex(), to)) {
			if (from < to) {
				Q_ASSERT(globalFrom < globalTo);
				--to;
				--globalTo;
			}
			p->visibleTags.move(from, to);
			p->tags.move(globalFrom, globalTo);
			endMoveRows();
		}
		saveTagOrder();*/
	}
}

void SeparatedModel::timerEvent(QTimerEvent *timerEvent)
{
	if (timerEvent->timerId() == p->timer.timerId()) {
		for (int i = 0; i < p->events.size(); i++) {
			processEvent(p->events.at(i));
			delete p->events.at(i);
		}
		p->events.clear();
		p->timer.stop();
		return;
	} else if (timerEvent->timerId() == p->unreadTimer.timerId()) {
		foreach (Contact *contact, p->unreadContacts) {
			ContactData::Ptr item_data = p->contacts.value(contact);
			//if (!item_data) {//FIXME why p->contacts doesn't contains contact
			//	qDebug() << "Unread" << contact <<  p->unreadContacts;
			//	continue;
			//}
			for (int i = 0; i < item_data->items.size(); i++) {
				ContactItem *item = item_data->items.at(i);
				QModelIndex index = createIndex(item->index(), 0, item);
				emit dataChanged(index, index);
			}
		}
		p->showMessageIcon = !p->showMessageIcon;
		return;
	}
	QAbstractItemModel::timerEvent(timerEvent);
}

bool SeparatedModel::eventFilter(QObject *obj, QEvent *ev)
{
	if (ev->type() == MetaContactChangeEvent::eventType()) {
		MetaContactChangeEvent *metaEvent = static_cast<MetaContactChangeEvent*>(ev);
		if (metaEvent->oldMetaContact() && !metaEvent->newMetaContact())
			addContact(metaEvent->contact());
		else if (!metaEvent->oldMetaContact() && metaEvent->newMetaContact())
			removeContact(metaEvent->contact());
		return false;
	} else if (ev->type() == Event::eventType()) {
		Event *event = static_cast<Event*>(ev);
		if (p->initData && event->id == p->initData->qutimStartupEvent)
			initialize();
		return false;
	}
	return QAbstractItemModel::eventFilter(obj, ev);
}

void SeparatedModel::filterAllList()
{
	for (int i = 0; i < p->accounts.size(); i++) {
		AccountItem *acc = p->accounts.at(i);
		for (int j = 0; j < acc->tags.size(); j++) {
			TagItem *tag = acc->tags.at(j);
			bool tagFiltered = !p->selectedTags.isEmpty() && !p->selectedTags.contains(tag->name);
			foreach (ContactItem *item, tag->contacts)
				hideContact(item, tagFiltered || !isVisible(item));
		}
	}
}

bool SeparatedModel::isVisible(ContactItem *item)
{
	if (!item) {
		qWarning() << Q_FUNC_INFO << "item is null";
		return true;
	}
	Contact *contact = item->data->contact;
	if (!p->lastFilter.isEmpty()) {
		return contact->id().contains(p->lastFilter,Qt::CaseInsensitive)
				|| contact->name().contains(p->lastFilter,Qt::CaseInsensitive);
	} else if (!p->selectedTags.isEmpty() && !p->selectedTags.contains(item->parent->name)) {
		return false;
	} else {
		return p->showOffline || contact->status().type() != Status::Offline;
	}
}

void SeparatedModel::initialize()
{
	connect(ChatLayer::instance(), SIGNAL(sessionCreated(qutim_sdk_0_3::ChatSession*)),
			this, SLOT(onSessionCreated(qutim_sdk_0_3::ChatSession*)));
	connect(MetaContactManager::instance(), SIGNAL(contactCreated(qutim_sdk_0_3::Contact*)),
			this, SLOT(addContact(qutim_sdk_0_3::Contact*)));

	foreach(Protocol *proto, Protocol::all()) {
		connect(proto, SIGNAL(accountCreated(qutim_sdk_0_3::Account*)), this, SLOT(onAccountCreated(qutim_sdk_0_3::Account*)));
		foreach(Account *account, proto->accounts())
			onAccountCreated(account);
	}

	SeparatedModelPrivate::InitData *initData = p->initData;
	p->initData = 0;
	// ensure correct order of tags
	/*QSet<QString> tags;
	foreach (Contact *contact, initData->contacts)
		foreach (const QString &tag, contact->tags())
			tags.insert(tag);
	foreach (const QString &tag, Config().value("contactList/tags", QStringList()))
		if (tags.contains(tag))
			ensureTag<TagItem>(tag);*/
	// add contacts to the contact list
	foreach (Contact *contact, initData->contacts)
		addContact(contact);
	delete initData;
}

void SeparatedModel::saveTagOrder()
{
	/*Config group = Config().group("contactList");
	QStringList tags;
	foreach (TagItem *tag, p->tags)
		tags << tag->name;
	group.setValue("tags", tags);*/
}

bool SeparatedModel::showOffline() const
{
	return p->showOffline;
}

AccountItem *SeparatedModel::onAccountCreated(qutim_sdk_0_3::Account *account)
{
	AccountItem *item = new AccountItem;
	item->account = account;
	int index = p->accounts.count();
	beginInsertRows(QModelIndex(), index, index);
	p->accounts.push_back(item);
	p->accountHash.insert(account, item);
	endInsertRows();

	foreach (Contact *contact, account->findChildren<Contact*>())
		addContact(contact);
	connect(account, SIGNAL(contactCreated(qutim_sdk_0_3::Contact*)),
			this, SLOT(addContact(qutim_sdk_0_3::Contact*)));
	connect(account, SIGNAL(destroyed(QObject*)),
			this, SLOT(onAccountDestroyed(QObject*)));
	return item;
}

void SeparatedModel::onAccountDestroyed(QObject *obj)
{
	AccountItem *item = p->accountHash.take(reinterpret_cast<Account*>(obj));
	int index = p->accounts.indexOf(item);
	beginRemoveRows(QModelIndex(), index, index);
	p->accounts.removeAt(index);
	foreach (TagItem *tag, item->tags) {
		foreach (ContactItem *contact, tag->contacts) {
			contact->data->items.removeOne(contact);
			delete contact;
		}
		delete tag;
	}
	endRemoveRows();
}

}
}


