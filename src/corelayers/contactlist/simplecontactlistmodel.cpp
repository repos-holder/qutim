#include "simplecontactlistmodel.h"
#include "libqutim/notificationslayer.h"
#include <QDebug>
#include "libqutim/messagesession.h"
#include "libqutim/status.h"
#include <QTreeView>
#include <libqutim/icon.h>
#include "libqutim/configbase.h"


namespace Core
{
	namespace SimpleContactList
	{
		struct ModelPrivate
		{
			QTreeView *view;
			QList<TagItem *> tags;
			QHash<QString, TagItem *> tagsHash;
			QMap<Contact *, ContactData::Ptr> contacts;
			QString lastFilter;
			bool showOffline;
		};

		Model::Model(QObject *parent) : QAbstractItemModel(parent), p(new ModelPrivate)
		{
			p->view = static_cast<QTreeView*>(parent);
			ConfigGroup group = Config().group("contactList");
			p->showOffline = group.value("showOffline", true);
		}

		Model::~Model()
		{
		}

		QModelIndex Model::index(int row, int, const QModelIndex &parent) const
		{
			if(row < 0)
				return QModelIndex();
			switch(getItemType(parent))
			{
			case TagType: {
				TagItem *item = reinterpret_cast<TagItem *>(parent.internalPointer());
				if(item->contacts.size() <= row)
					return QModelIndex();
				return createIndex(row, 0, item->contacts.at(row));
				}
			case ContactType:
				return QModelIndex();
			default:
				if(p->tags.size() <= row)
					return QModelIndex();
				return createIndex(row, 0, p->tags.at(row));
			}
		}

		QModelIndex Model::parent(const QModelIndex &child) const
		{
			switch(getItemType(child))
			{
			case ContactType: {
				ContactItem *item = reinterpret_cast<ContactItem *>(child.internalPointer());
				return createIndex(p->tags.indexOf(item->parent), 0, item->parent);
				}
			case TagType:
			default:
				return QModelIndex();
			}
		}

		int Model::rowCount(const QModelIndex &parent) const
		{
			switch(getItemType(parent))
			{
			case TagType:
				return reinterpret_cast<TagItem *>(parent.internalPointer())->contacts.size();
			case ContactType:
				return 0;
			default:
				return p->tags.size();
			}
		}

		int Model::columnCount(const QModelIndex &) const
		{
			return 1;
		}

		bool Model::hasChildren(const QModelIndex &parent) const
		{
			switch(getItemType(parent))
			{
			case TagType:
				return !reinterpret_cast<TagItem *>(parent.internalPointer())->contacts.isEmpty();
			case ContactType:
				return false;
			default:
				return !p->tags.isEmpty();
			}
		}

		QVariant Model::data(const QModelIndex &index, int role) const
		{
			switch(getItemType(index))
			{
			case ContactType:
				{
					ContactItem *item = reinterpret_cast<ContactItem *>(index.internalPointer());
					switch(role)
					{
					case Qt::DisplayRole: {
						QString name = item->data->contact->name();
						return name.isEmpty() ? item->data->contact->id() : name;
					}
					case Qt::DecorationRole:
						return item->data->contact->status().icon();
					case ItemDataType:
						return ContactType;
					case ItemStatusText:
						return item->data->contact->status().text();
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
					case ItemDataType:
						return TagType;
					case Qt::DecorationRole:
						return Icon("feed-subscribe");
					default:
						return QVariant();
					}
				}
			default:
				return QVariant();
			}
		}
		
		bool contactLessThan (ContactItem *a, ContactItem *b) {
			int result = a->data->status.type() - b->data->status.type();
			if (result != 0)
				return result < 0;
			return a->data->contact->title().compare(b->data->contact->title(), Qt::CaseInsensitive) < 0;
		};		

		void Model::addContact(Contact *contact)
		{
			if(p->contacts.contains(contact))
				return;
			connect(contact, SIGNAL(destroyed(QObject*)), this, SLOT(contactDeleted(QObject*)));
            connect(contact, SIGNAL(statusChanged(qutim_sdk_0_3::Status)), SLOT(contactStatusChanged(qutim_sdk_0_3::Status)));
            connect(contact, SIGNAL(nameChanged(QString)), SLOT(contactNameChanged(QString)));
			connect(contact, SIGNAL(tagsChanged(QSet<QString>)), SLOT(contactTagsChanged(QSet<QString>)));
			QSet<QString> tags = contact->tags();
			if(tags.isEmpty())
				tags << QLatin1String("Default");
			ContactData::Ptr item_data(new ContactData);
			item_data->contact = contact;
			item_data->tags = tags;
			item_data->status = contact->status();
			p->contacts.insert(contact, item_data);
			for(QSet<QString>::const_iterator it = tags.constBegin(); it != tags.constEnd(); it++)
			{
				TagItem *tag = ensureTag(*it);
				int tagIndex = p->tags.indexOf(tag);
				QModelIndex tagModelIndex = createIndex(tagIndex, 0, tag);
				ContactItem *item = new ContactItem(item_data);
				
				QList<ContactItem *> contacts = tag->contacts;
				QList<ContactItem *>::const_iterator contacts_it =
						qLowerBound(contacts.constBegin(), contacts.constEnd(), item, contactLessThan);
				int index = contacts_it - contacts.constBegin();

				beginInsertRows(tagModelIndex, index, index);
				item->parent = tag;
				item_data->items.insert(index,item);
				tag->contacts.insert(index,item);
				endInsertRows();
				bool show = isVisible(item);
				// hideContact will decrease it by one
				tag->visible++;
				if (show)
					recheckTag(tag, tagIndex);
				else
					hideContact(index, tagModelIndex, true);
			}
		}

		void Model::removeContact(Contact *contact)
		{
			contact->disconnect(this);
			ContactData::Ptr item_data = p->contacts.value(contact);
			if(!item_data)
				return;
			int counter = item_data->status.type() == Status::Offline ? -1 : 0;
			for(int i = 0; i < item_data->items.size(); i++)
			{
				ContactItem *item = item_data->items.at(i);
				item->parent->online += counter;
				int index = item->index();
				beginRemoveRows(createIndex(p->tags.indexOf(item->parent), 0, item->parent), index, index);
				item->parent->contacts.removeAt(index);
				endRemoveRows();

				int tagIndex = p->tags.indexOf(item->parent);

				if (item->parent->contacts.empty()) {
					beginRemoveRows(QModelIndex(),tagIndex,tagIndex);
					p->tagsHash.remove(p->tags.at(tagIndex)->name);
					p->tags.removeAt(tagIndex);
					endRemoveRows();
				} else if (isVisible(item)) {
					item->parent->visible--;
					recheckTag(item->parent, tagIndex);
				}
			}
			p->contacts.remove(contact);
		}

		bool Model::containsContact(Contact *contact) const
		{
			return p->contacts.contains(contact);
		}

		void Model::contactDeleted(QObject *obj)
		{
			if (Contact *contact = qobject_cast<Contact *>(obj))
				removeContact(contact);
		}

		void Model::contactStatusChanged(Status status)
		{
			Contact *contact = qobject_cast<Contact *>(sender());
			ContactData::Ptr item_data = p->contacts.value(contact);
			if(!item_data)
				return;
			if(status.type() == item_data->status.type())
				return;
			int counter = 0;
			if (status.type() == Status::Offline)
				counter = -1;
			else if (item_data->status == Status::Offline)
				counter = +1;
			item_data->status = status;
			const QList<ContactItem *> &items = item_data->items;
			bool show = isVisible(item_data->items.value(0));
			for(int i = 0; i < items.size(); i++)
			{
				ContactItem *item = items.at(i);
				item->parent->online += counter;
				QList<ContactItem *> contacts = item->parent->contacts;
				QList<ContactItem *>::const_iterator it =
						qLowerBound(contacts.constBegin(), contacts.constEnd(), item, contactLessThan);
				
				int to = it - contacts.constBegin();
				int from = contacts.indexOf(item);
				
				QModelIndex parentIndex = createIndex(p->tags.indexOf(item->parent), 0, item->parent);
	
				if (from == to) {
					QModelIndex index = createIndex(item->index(), 0, item);
					dataChanged(index, index);					
				}
				else {
					if (to == -1 || to >= contacts.count())
						continue;					

					beginMoveRows(parentIndex, from, from, parentIndex, to);
					item->parent->contacts.move(from,to);
					//item_data->items.move(from,to); //FIXME
					endMoveRows();	
				}
				hideContact(to, parentIndex, !show);
			}
			//if (ChatLayer::get(contact,false))
			//	return; //TODO FIXME
// 			Notifications::Type notify;
// 			if(status.type() == Status::Offline)
// 				notify = Notifications::Offline;
// 			else if(item_data->status.type() == Status::Offline)
// 				notify = Notifications::Online;
// 			else
// 				notify = Notifications::StatusChange;
// 			Notifications::sendNotification(notify, contact);
		}

		void Model::contactNameChanged(const QString &name)
		{
			// TODO: Add sort by name
			Q_UNUSED(name);
			Contact *contact = qobject_cast<Contact *>(sender());
			ContactData::Ptr item_data = p->contacts.value(contact);
			if(!item_data)
				return;
			const QList<ContactItem *> &items = item_data->items;
			for(int i = 0; i < items.size(); i++)
			{
				ContactItem *item = items.at(i);
				QModelIndex index = createIndex(item->index(), 0, item);
				dataChanged(index, index);
			}
		}

		void Model::contactTagsChanged(const QSet<QString> &tags_helper)
		{
			Contact *contact = qobject_cast<Contact *>(sender());
			ContactData::Ptr item_data = p->contacts.value(contact);
			if(!item_data)
				return;
			bool show = isVisible(item_data->items.value(0));
			QSet<QString> tags = tags_helper;
			if(tags.isEmpty())
				tags << QLatin1String("Default");
			QSet<QString> to_add = tags - item_data->tags;
			for (int i = 0, size = item_data->items.size(); i < size; i++) {
				ContactItem *item = item_data->items.at(i);
				if(tags.contains(item->parent->name))
					continue;
				int index = item->index();
				int tagIndex = p->tags.indexOf(item->parent);
				beginRemoveRows(createIndex(tagIndex, 0, item->parent), index, index);
				item->parent->contacts.removeAt(index);
				item_data->items.removeAt(i);
				i--;
				size--;
				endRemoveRows();
				if (show) {
					item->parent->visible--;
					recheckTag(item->parent, tagIndex);
				}
			}
			for (QSet<QString>::const_iterator it = to_add.constBegin(); it != to_add.constEnd(); it++) {
				TagItem *tag = ensureTag(*it);
				tag->visible++;
				ContactItem *item = new ContactItem(item_data);
				int tagIndex = p->tags.indexOf(tag);
				QModelIndex tagModelIndex = createIndex(tagIndex, 0, tag);
				QList<ContactItem *> contacts = tag->contacts;
				QList<ContactItem *>::const_iterator it =
						qLowerBound(contacts.constBegin(), contacts.constEnd(), item, contactLessThan);

				int index = it - contacts.constBegin();
				beginInsertRows(tagModelIndex, index, index);
				item->parent = tag;
				item_data->items << item;
				tag->contacts << item;
				endInsertRows();
				if (!show)
					hideContact(index, tagModelIndex, true);
			}
			item_data->tags = tags;
		}

		void Model::onHideShowOffline()
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

		void Model::onFilterList(const QString &filter)
		{
			if (filter == p->lastFilter)
				return;
			p->lastFilter = filter;
			filterAllList();
		}

		void Model::filterAllList()
		{
			for (int i = 0; i < p->tags.size(); i++) {
				TagItem *tag = p->tags.at(i);
				QModelIndex index = createIndex(i, 0, tag);
				tag->visible = 0;
				for (int j = 0; j < tag->contacts.size(); j++) {
					bool show = isVisible(tag->contacts.at(j));
					if (show)
						tag->visible++;
					if (p->view->isRowHidden(j, index) == show)
						p->view->setRowHidden(j, index, !show);
				}
				index = QModelIndex();
				if (p->view->isRowHidden(i, index) == !!tag->visible)
					p->view->setRowHidden(i, index, !tag->visible);
			}
		}

		bool Model::isVisible(ContactItem *item)
		{
			if (!item) {
				qWarning() << Q_FUNC_INFO << "item is null";
				return true;
			}
			Contact *contact = item->data->contact;
			if (!p->lastFilter.isEmpty()) {
				return contact->id().contains(p->lastFilter)
				|| contact->name().contains(p->lastFilter);
			} else {
				return p->showOffline || contact->status().type() != Status::Offline;
			}
		}

		void Model::hideContact(int index, const QModelIndex &tagIndex, bool hide)
		{
			TagItem *item = reinterpret_cast<TagItem*>(tagIndex.internalPointer());
			if (p->view->isRowHidden(index, tagIndex) != hide) {
				item->visible += hide ? -1 : 1;
				p->view->setRowHidden(index, tagIndex, hide);
				recheckTag(item, tagIndex.row());
			}
		}

		void Model::recheckTag(TagItem *item, int index)
		{
			if (index < 0)
				index = p->tags.indexOf(item);
			bool hidden = !item->visible;
			if (p->view->isRowHidden(index, QModelIndex()) == hidden)
				return;
			p->view->setRowHidden(index, QModelIndex(), hidden);
		}

		TagItem *Model::ensureTag(const QString &name)
		{
			TagItem *tag = 0;
			if(!(tag = p->tagsHash.value(name, 0)))
			{
				int index = p->tags.size();
				beginInsertRows(QModelIndex(), index, index);
				tag = new TagItem;
				tag->name = name;
				p->tagsHash.insert(tag->name, tag);
				p->tags << tag;
				endInsertRows();
				p->view->setRowHidden(index, QModelIndex(), true);
			}
			return tag;
		}
		
		QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
		{
			if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section==0)
				return tr("All tags");

			return QVariant();
		}

	}
}
