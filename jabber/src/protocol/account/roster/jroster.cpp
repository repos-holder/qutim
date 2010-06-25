#include "jroster.h"
#include "../jaccount.h"
#include "../../jprotocol.h"
#include "jcontact.h"
#include "jcontactresource.h"
#include "../vcard/jvcardmanager.h"
#include <QFile>
#include <gloox/vcardupdate.h>
#include <gloox/subscription.h>
#include <gloox/nickname.h>
#include <qutim/metacontact.h>
#include <qutim/metacontactmanager.h>
#include <QFile>
#include <QStringBuilder>
#include <qutim/authorizationdialog.h>
#include <qutim/notificationslayer.h>

namespace Jabber
{
	struct JRosterPrivate
	{
		JAccount *account;
		RosterManager *rosterManager;
		MetaContactsStorage *metaStorage;
		QMap<JContact*, MetaContact*> metaContacts;
		QHash<QString, JContact*> contacts;
		bool avatarsAutoLoad;
		bool atMetaContactsLoad;
	};

	JRoster::JRoster(JAccount *account) : p(new JRosterPrivate)
	{
		loadSettings();
		p->atMetaContactsLoad = false;
		p->account = account;
		p->metaStorage = new MetaContactsStorage(p->account->connection()->client());
		p->metaStorage->registerMetaContactHandler(this);
		p->rosterManager = p->account->connection()->client()->rosterManager();
		p->rosterManager->registerRosterListener(this, false);
		p->account->connection()->client()->registerPresenceHandler(this);
		p->account->connection()->client()->registerSubscriptionHandler(this);
	}

	JRoster::~JRoster()
	{
	}

	ChatUnit *JRoster::contact(const QString &id, bool create)
	{
		JID jid(id.toStdString());
		QString bare = id.contains('/') ? QString::fromStdString(jid.bare()) : id;
		QString resourceId = id == bare ? QString() : QString::fromStdString(jid.resource());
		JContact *contact = p->contacts.value(bare);
		if (!resourceId.isEmpty()) {
			if (!contact) {
				if (create) {
					contact = new JContact(id,p->account);
					contact->installEventFilter(this);
					p->contacts.insert(id,contact);
					contact->setContactInList(false);
					emit p->account->contactCreated(contact);
					return contact;
				} else {
					return 0;
				}
			}
			if (JContactResource *resource = contact->resource(resourceId))
				return resource;
			if (create) {
				return contact;
			}
		} else if (contact) {
			return contact;
		} else if (create) {
			contact = new JContact(id,p->account);
			contact->installEventFilter(this);
			p->contacts.insert(id,contact);
			contact->setContactInList(false);
			emit p->account->contactCreated(contact);
			return contact;
		}
		return 0;
	}

	void JRoster::handleItemAdded(const JID &jid)
	{
	}

	void JRoster::handleItemSubscribed(const JID &jid)
	{
	}

	void JRoster::handleItemRemoved(const JID &jid)
	{
	}

	void JRoster::handleItemUpdated(const JID &jid)
	{
	}

	void JRoster::handleItemUnsubscribed(const JID &jid)
	{
	}

	void JRoster::handleRoster(const Roster &roster)
	{
		std::map<const std::string, RosterItem *>::const_iterator items = roster.begin();
		for(; items != roster.end(); ++items) {
			RosterItem *item = items->second;
			QString jid(QString::fromStdString(item->jidJID().bare()));
			if (jid == p->account->id()) {
				continue;
			} else if (!p->contacts.contains(jid)) {
				JContact *contact = new JContact(jid, p->account);
				contact->installEventFilter(this);
				contact->setContactName(QString::fromStdString(item->name()));
				QStringList tags;
				StringList groups = item->groups();
				StringList::const_iterator group = groups.begin();
				for(; group != groups.end(); ++group)
					tags.append(QString::fromStdString(*group));
				contact->setContactTags(tags);
				contact->setContactInList(true);
				p->contacts.insert(jid, contact);
				emit p->account->contactCreated(contact);
			}
		}
		p->metaStorage->requestMetaContacts();
	}

	void JRoster::handleRosterPresence(const RosterItem &item, const std::string &resource,
									   Presence::PresenceType presence, const std::string &msg)
	{
		Q_UNUSED(item);
		Q_UNUSED(resource);
		Q_UNUSED(presence);
		Q_UNUSED(msg);
	}

	void JRoster::handlePresence(const Presence &presence)
	{
		QString jid(QString::fromStdString(presence.from().bare()));
		QString resource(QString::fromStdString(presence.from().resource()));
		if (presence.from().bare() == p->account->client()->jid().bare())
			 return;
		if (!p->contacts.contains(jid)) {
			JContact *contact = new JContact(jid, p->account);
			contact->installEventFilter(this);
			contact->setContactName(QString::fromStdString(presence.from().username()));
			QStringList tags;
			tags.append(tr("Not in list"));
			contact->setContactTags(tags);
			contact->setContactInList(false);
			p->contacts.insert(jid, contact);
		}
		if (!resource.isEmpty())
			p->contacts.value(jid)->setStatus(resource, presence.presence(), presence.priority(),
					QString::fromStdString(presence.status()));
		if (presence.presence() != Presence::Unavailable && !presence.error()) {
			const VCardUpdate *vcard = presence.findExtension<VCardUpdate>(ExtVCardUpdate);
			if(vcard) {
				QString hash = QString::fromStdString(vcard->hash());
				JContact *contact = p->contacts.value(jid);
				if (contact->avatarHash() != hash) {
					if(hash.isEmpty() || QFile(p->account->getAvatarPath()%QLatin1Char('/')%hash).exists())
						contact->setAvatar(hash);
					else if (p->avatarsAutoLoad)
						p->account->connection()->vCardManager()->fetchVCard(jid);
				}
			}
		}
	}

	void JRoster::handleSelfPresence(const RosterItem &item, const std::string &resource,
			Presence::PresenceType presence, const std::string &msg)
	{
	}

	bool JRoster::handleSubscriptionRequest(const JID &jid, const std::string &msg)
	{
		return false;
	}

	bool JRoster::handleUnsubscriptionRequest(const JID &jid, const std::string &msg)
	{
		return true;
	}

	void JRoster::handleNonrosterPresence(const Presence &presence)
	{
	}

	void JRoster::handleRosterError(const IQ &iq)
	{
	}

	void JRoster::loadSettings()
	{
		p->avatarsAutoLoad = JProtocol::instance()->config("general").value("getavatars", true);
	}

	void JRoster::setOffline()
	{
		foreach (JContact *contact, p->contacts)
			contact->setStatus("", Presence::Unavailable, 0);
	}

	void JRoster::handleSubscription(const Subscription &subscription)
	{
		QString jid = QString::fromStdString(subscription.from().bare());
		QString name;
		JContact *contact = p->contacts.value(jid);
		if (contact) {
			name = contact->name();
		} else {
			const Nickname *nickname = subscription.findExtension<Nickname>(ExtNickname);
			name = nickname ? QString::fromStdString(nickname->nick()) : "";
		}
		QString text;
		switch (subscription.subtype()) {
		case Subscription::Subscribe:
			if (!contact) {
				contact = new JContact(jid, p->account);
				contact->installEventFilter(this);
				contact->setContactName(name);
				contact->setContactInList(false);
			}
			{
				AuthorizationDialog *dialog = AuthorizationDialog::request(contact,
						QString::fromStdString(subscription.status()));
				connect(dialog, SIGNAL(finished(bool)), SLOT(sendAuthResponse(bool)));
			}
			break;
		case Subscription::Subscribed:
			text = tr("You were authorized");
			break;
		case Subscription::Unsubscribed:
			text = tr("Your authorization was removed");
			break;
		case Subscription::Unsubscribe:
			text = tr("Contacts's authorization was removed");
			break;
		case Subscription::Invalid:
		default:
			text = "";
		}
		if (!text.isEmpty()) {
			QObject *sender = new QObject();
			sender->setProperty("id", jid);
			sender->setProperty("name", name);
			Notifications::sendNotification(Notifications::System, sender, text);
			sender->deleteLater();
		}
	}

	void JRoster::sendAuthResponse(bool answer)
	{
		AuthorizationDialog *dialog = qobject_cast<AuthorizationDialog *>(sender());
		Q_ASSERT(dialog);
		JContact *contact = qobject_cast<JContact*>(dialog->contact());
		p->rosterManager->ackSubscriptionRequest(JID(contact->id().toStdString()), answer);
		if (!contact->isInList()) {
			if (answer) {
				contact->setContactInList(true);
				p->contacts.insert(contact->id(), contact);
			} else {
				contact->deleteLater();
			}
		}
	}
	
	void JRoster::handleMetaContact(const MetaContactList &mcList)
	{
		p->atMetaContactsLoad = true;
		p->metaContacts.clear();
		foreach (const MetaContactListItem &item, mcList) {
			QString jid = QString::fromStdString(JID(item.jid).bare());
			JContact *contact = p->contacts.value(jid);
			if (!contact)
				continue;
			QString tag = QString::fromStdString(item.tag);
			MetaContact *metaContact = qobject_cast<MetaContact*>(contact->upperUnit());
			if (metaContact && metaContact->id() == tag)
				continue;
			ChatUnit *unit = MetaContactManager::instance()->getUnit(tag, true);
			metaContact = qobject_cast<MetaContact*>(unit);
			Q_ASSERT(metaContact);
			metaContact->addContact(contact);
			p->metaContacts.insert(contact, metaContact);
		}
		p->atMetaContactsLoad = false;
	}
	
	bool JRoster::eventFilter(QObject *obj, QEvent *event)
	{
		if (!p->atMetaContactsLoad && event->type() == MetaContactChangeEvent::eventType()) {
			JContact *contact = static_cast<JContact*>(obj);
			MetaContactChangeEvent *metaEvent = static_cast<MetaContactChangeEvent*>(event);
			Q_ASSERT(metaEvent->contact() == contact);
			if (metaEvent->newMetaContact())
				p->metaContacts.insert(contact, metaEvent->newMetaContact());
			else
				p->metaContacts.remove(contact);
			QMap<JContact*, MetaContact*>::iterator it = p->metaContacts.begin();
			QMap<JContact*, MetaContact*>::iterator endit = p->metaContacts.end();
			MetaContactList list;
			for (; it != endit; it++) {
				MetaContactListItem item;
				item.jid = it.key()->id().toStdString();
				item.tag = it.value()->id().toStdString();
				item.order = it.value()->lowerUnits().indexOf(it.key());
				list.push_back(item);
			}
			p->metaStorage->storeMetaContacts(list);
			return false;
		}
		return QObject::eventFilter(obj, event);
	}
	
	void JRoster::onSessionCreated(qutim_sdk_0_3::ChatSession *session)
	{
		Q_UNUSED(session);
//		ChatUnit *unit = session->getUnit();
//		if (MetaContact *meta = qobject_cast<MetaContact*>(unit)) {
//			
//		}
	}
}
