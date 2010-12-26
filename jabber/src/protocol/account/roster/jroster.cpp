#include "jroster.h"
#include "../jaccount.h"
#include "../../jprotocol.h"
#include "jcontact.h"
#include "jcontactresource.h"
#include "../vcard/jvcardmanager.h"
#include <QFile>
#include <qutim/metacontact.h>
#include <qutim/metacontactmanager.h>
#include <QFile>
#include <QStringBuilder>
#include <qutim/authorizationdialog.h>
#include <qutim/notificationslayer.h>
#include <qutim/messagesession.h>
#include "../jaccount_p.h"
#include <qutim/debug.h>
#include <qutim/notificationslayer.h>
//jreen
#include <jreen/chatstate.h>
#include <jreen/delayeddelivery.h>
#include <jreen/nickname.h>
#include <jreen/vcardupdate.h>

namespace Jabber
{

class JRosterPrivate
{
public:
	JRosterPrivate(JRoster *q) : q_ptr(q) {}
	JAccount *account;
	JRoster *q_ptr;
	QHash<QString, JContact*> contacts;
	bool isLoaded;
};

JRoster::JRoster(JAccount *account) :
	AbstractRoster(account->client()),
	d_ptr(new JRosterPrivate(this))
{
	Q_D(JRoster);
	d->account = account;
	d->isLoaded = false;
	connect(d->account->client(),SIGNAL(newPresence(jreen::Presence)),
			this,SLOT(handleNewPresence(jreen::Presence)));
	connect(d->account->client(),SIGNAL(disconnected()),
			this,SLOT(onDisconnected()));
	connect(d->account->client(),SIGNAL(newMessage(jreen::Message)),
			this,SLOT(onNewMessage(jreen::Message)));
}

JRoster::~JRoster()
{

}

void JRoster::onItemAdded(QSharedPointer<jreen::AbstractRosterItem> item)
{
	//Q_D(JRoster);
	JContact *c = static_cast<JContact*>(contact(item->jid(),true));
	Q_ASSERT(c);
	fillContact(c,item);
	if(d_func()->isLoaded)
		Notifications::send(Notifications::System,
							c,
							tr("Contact has been added to roster").arg(c->title()));
}
void JRoster::onItemUpdated(QSharedPointer<jreen::AbstractRosterItem> item)
{
	Q_D(JRoster);
	JContact *c = d->contacts.value(item->jid());
	if(c)
		fillContact(c,item);
}

void JRoster::onItemRemoved(const QString &jid)
{
	Q_D(JRoster);
	JContact *c = d->contacts.take(jid);
	if(c) {
		c->setContactInList(false);
		c->setContactSubscription(jreen::AbstractRosterItem::None);
		Notifications::send(Notifications::System,
							c,
							tr("Contact has been removed from roster").arg(c->title()));
	}
}

void JRoster::onLoaded(const QList<QSharedPointer<jreen::AbstractRosterItem> > &items)
{
	AbstractRoster::onLoaded(items);
	d_func()->isLoaded = true;
}

JContact *JRoster::createContact(const jreen::JID &id)
{
	Q_D(JRoster);
	JContact *contact = new JContact(id.bare(),d->account);
	d->contacts.insert(id.bare(),contact);
	contact->setContactInList(false);
	emit d->account->contactCreated(contact);
	connect(contact,SIGNAL(destroyed(QObject*)),SLOT(onContactDestroyed(QObject*)));
	return contact;
}

ChatUnit *JRoster::contact(const jreen::JID &jid, bool create)
{
	Q_D(JRoster);
	QString bare = jid.bare();
	QString resourceId = jid.resource();
	JContact *contact = d->contacts.value(bare);
	if (!resourceId.isEmpty()) {
		if (!contact) {
			if (create)
				return createContact(jid);
			else {
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
	} else if (create)
		return createContact(jid);
	return 0;
}

void JRoster::fillContact(JContact *contact, QSharedPointer<jreen::AbstractRosterItem> item)
{
	QString name = item->name();
	contact->setContactName(name);
	QStringList tags = item->groups();
	contact->setContactTags(tags);
	if (!contact->isInList())
		contact->setContactInList(true);
	contact->setContactSubscription(item->subscriptionType());
}

void JRoster::handleNewPresence(jreen::Presence presence)
{
	Q_D(JRoster);

	switch(presence.subtype())
	{
	case jreen::Presence::Subscribe:
	case jreen::Presence::Unsubscribe:
	case jreen::Presence::Unsubscribed:
	case jreen::Presence::Subscribed:
		handleSubscription(presence);
		break;
	case jreen::Presence::Error:
	case jreen::Presence::Probe:
		return;
	default:
		break;
	}

	jreen::JID from = presence.from();
	if(d->account->client()->jid() == from) {
		d->account->d_func()->setPresence(presence);
		return;
	}
	JContact *c = d->contacts.value(from.bare());
	if(c) {
		c->setStatus(presence);		
		const jreen::VCardUpdate *vcard = presence.findExtension<jreen::VCardUpdate>().data();
		if(vcard) {
			QString hash = vcard->photoHash();
			debug() << "vCard update" << (c->avatarHash() != hash);
			if (c->avatarHash() != hash) {
				if(hash.isEmpty() || QFile(d->account->getAvatarPath()%QLatin1Char('/')%hash).exists())
					c->setAvatar(hash);
				else
					d->account->d_ptr->vCardManager->fetchVCard(from);
			}
		}
	}
}

void JRoster::onDisconnected()
{
	Q_D(JRoster);
	foreach(JContact *c,d->contacts) {
		jreen::Presence unavailable(jreen::Presence::Unavailable,
									c->id());
		c->setStatus(unavailable);
	}
}

void JRoster::onNewMessage(jreen::Message message)
{
	Q_D(JRoster);
	//temporary
	JContact *c = d->contacts.value(message.from().bare());
	if(!c) {
		c = static_cast<JContact*>(contact(message.from(),true));
		c->setInList(false);
		if(jreen::Nickname *nick = message.findExtension<jreen::Nickname>().data())
			c->setName(nick->nick());
	}

	if(message.body().isEmpty())
		return;
	qutim_sdk_0_3::Message coreMessage;
	if(const jreen::DelayedDelivery *d = message.when())
		coreMessage.setTime(d->dateTime());
	else
		coreMessage.setTime(QDateTime::currentDateTime());
	coreMessage.setText(message.body());
	coreMessage.setProperty("subject",message.subject());
	coreMessage.setChatUnit(c);
	coreMessage.setIncoming(true);
	ChatLayer::get(c,true)->appendMessage(coreMessage);
}

void JRoster::handleSubscription(jreen::Presence subscription)
{
	Q_D(JRoster);
	QString bare = subscription.from().bare();
	QString name;
	JContact *contact = d->contacts.value(bare);
	if (contact) {
		name = contact->name();
	} else {
		jreen::Nickname *nickname = subscription.findExtension<jreen::Nickname>().data();
		if(nickname)
			name = nickname->nick();
	}

	switch(subscription.subtype())
	{
	case jreen::Presence::Subscribe: {
		contact = static_cast<JContact*>(this->contact(bare,true));
		contact->setContactName(name);
		contact->setContactInList(false);
		Authorization::Reply request(Authorization::Reply::New,
									 contact,
									 subscription.status());
		qApp->sendEvent(Authorization::service(),&request);
		break;
	}
	case jreen::Presence::Unsubscribe: { //how to handle it?
		QString text = tr("Received a request for removal from the subscribers");
		Authorization::Reply request(Authorization::Reply::Reject,
									 contact,
									 text);
		qApp->sendEvent(Authorization::service(),&request);
		break;
	}
	case jreen::Presence::Unsubscribed: {
		QString text = tr("You have been removed from the list of subscribers");
		Authorization::Reply request(Authorization::Reply::Rejected,
									 contact,
									 text);
		qApp->sendEvent(Authorization::service(),&request);
		break;
	}
	case jreen::Presence::Subscribed: {
		QString text = tr("You have been added to the list of subscribers");
		Authorization::Reply request(Authorization::Reply::Accepted,
									 contact,
									 text);
		qApp->sendEvent(Authorization::service(),&request);
		break;
	}
	default:
		break;
	}
}

void JRoster::addContact(const JContact *contact)
{
	Q_D(JRoster);
	add(contact->id(),contact->name(),contact->tags());
	jreen::Presence presence (jreen::Presence::Subscribed,
							  contact->id()
							  );
	d->account->client()->send(presence);
}

void JRoster::removeContact(const JContact *contact)
{
	Q_D(JRoster);
	remove(contact->id());
	removeSubscription(contact);
	jreen::Presence presence (jreen::Presence::Unsubscribed,
							  contact->id()
							  );
	d->account->client()->send(presence);
}

void JRoster::onContactDestroyed(QObject *obj)
{
	Q_D(JRoster);
	JContact *c = static_cast<JContact*>(obj);
	d->contacts.remove(d->contacts.key(c));
}

void JRoster::removeSubscription(const JContact *contact)
{
	Q_D(JRoster);
	jreen::Presence presence(jreen::Presence::Unsubscribe,
							 contact->id()
							 );
	d->account->client()->send(presence);
	synchronize();
}

void JRoster::requestSubscription(const jreen::JID &id,const QString &reason)
{
	Q_D(JRoster);
	jreen::Presence presence(jreen::Presence::Subscribe,
							 id,
							 reason
							 );
	d->account->client()->send(presence);
}


void JRoster::loadSettings()
{

}

void JRoster::saveSettings()
{

}

void JRoster::setGroups(const JContact *contact, const QStringList &groups)
{
	m_items.value(contact->id())->setGroups(groups);
	synchronize();
}

void JRoster::setName(const JContact *contact, const QString &name)
{
	m_items.value(contact->id())->setName(name);
	synchronize();
}

//dead code

//struct JRosterPrivate
//{
//	JAccount *account;
//	RosterManager *rosterManager;
//	MetaContactsStorage *metaStorage;
//	QMap<JContact*, MetaContact*> metaContacts;
//	QHash<QString, JContact*> contacts;
//	bool avatarsAutoLoad;
//	bool atMetaContactsLoad;
//};

//JRoster::JRoster(JAccount *account) : p(new JRosterPrivate)
//{
//	p->account = account;
//	p->atMetaContactsLoad = false;
//	p->metaStorage = new MetaContactsStorage(p->account->connection()->client());
//	p->metaStorage->registerMetaContactHandler(this);
//	p->rosterManager = p->account->connection()->client()->rosterManager();
//	p->rosterManager->registerRosterListener(this, false);
//	p->account->connection()->client()->registerPresenceHandler(this);
//	p->account->connection()->client()->registerSubscriptionHandler(this);
//	loadSettings();
//}

//JRoster::~JRoster()
//{
//}

//ChatUnit *JRoster::contact(const QString &id, bool create)
//{
//	JID jid(id.toStdString());
//	QString bare = id.contains('/') ? QString::fromStdString(jid.bare()) : id;
//	QString resourceId = id == bare ? QString() : QString::fromStdString(jid.resource());
//	JContact *contact = p->contacts.value(bare);
//	if (!resourceId.isEmpty()) {
//		if (!contact) {
//			if (create) {
//				contact = new JContact(id,p->account);
//				contact->installEventFilter(this);
//				p->contacts.insert(id,contact);
//				contact->setContactInList(false);
//				emit p->account->contactCreated(contact);
//				return contact;
//			} else {
//				return 0;
//			}
//		}
//		if (JContactResource *resource = contact->resource(resourceId))
//			return resource;
//		if (create) {
//			return contact;
//		}
//	} else if (contact) {
//		return contact;
//	} else if (create) {
//		contact = new JContact(id,p->account);
//		contact->installEventFilter(this);
//		p->contacts.insert(id,contact);
//		contact->setContactInList(false);
//		emit p->account->contactCreated(contact);
//		return contact;
//	}
//	return 0;
//}

//void JRoster::handleItemAdded(const JID &nativeJid)
//{
//	QString jid = QString::fromStdString(nativeJid.bare());
//	if (jid == p->account->id())
//		return;
//	JContact *contact;
//	RosterItem *item = p->rosterManager->getRosterItem(nativeJid);
//	if (!(contact = p->contacts.value(jid))) {
//		contact = new JContact(jid, p->account);
//		contact->installEventFilter(this);
//		p->contacts.insert(jid, contact);
//		fillContact(contact, *item);
//		emit p->account->contactCreated(contact);
//	} else {
//		fillContact(contact, *item);
//	}
//}

//void JRoster::handleItemSubscribed(const JID &jid)
//{
//}

//void JRoster::handleItemRemoved(const JID &nativeJid)
//{
//	QString jid = QString::fromStdString(nativeJid.bare());
//	if (jid == p->account->id())
//		return;
//	JContact *contact = p->contacts.value(jid);
//	contact->setContactInList(false);
//}

//void JRoster::handleItemUpdated(const JID &nativeJid)
//{
//	QString jid = QString::fromStdString(nativeJid.bare());
//	if (jid == p->account->id())
//		return;
//	RosterItem *item = p->rosterManager->getRosterItem(nativeJid);
//	JContact *contact = p->contacts.value(jid);
//	fillContact(contact, *item);
//}

//void JRoster::handleItemUnsubscribed(const JID &jid)
//{
//}

//void JRoster::handleRoster(const Roster &roster)
//{
//	Config config = p->account->config();
//	config.remove(QLatin1String("contactlist"));
//	config.beginGroup(QLatin1String("contactlist"));
//	QSet<QString> oldRoster = p->contacts.keys().toSet();
//	QSet<QString> newRoster;
//	std::map<const std::string, RosterItem *>::const_iterator items = roster.begin();
//	for(; items != roster.end(); ++items) {
//		RosterItem *item = items->second;
//		QString jid = QString::fromStdString(item->jidJID().bare());
//		if (jid == p->account->id())
//			continue;
//		newRoster << jid;
//		JContact *contact;
//		if (!(contact = p->contacts.value(jid))) {
//			contact = new JContact(jid, p->account);
//			contact->installEventFilter(this);
//			p->contacts.insert(jid, contact);
//			fillContact(contact, *item);
//			emit p->account->contactCreated(contact);
//		} else {
//			fillContact(contact, *item);
//		}
//		config.beginGroup(jid);
//		config.setValue("name", contact->name());
//		config.setValue("tags", contact->tags());
//		config.setValue("avatar", contact->avatarHash());
//		config.endGroup();
//	}
//	p->metaStorage->requestMetaContacts();
//}

//void JRoster::handleRosterPresence(const RosterItem &item, const std::string &resource,
//								   Presence::PresenceType presence, const std::string &msg)
//{
//	Q_UNUSED(item);
//	Q_UNUSED(resource);
//	Q_UNUSED(presence);
//	Q_UNUSED(msg);
//}

//void JRoster::handlePresence(const Presence &presence)
//{
////	QString jid(QString::fromStdString(presence.from().bare()));
////	QString resource(QString::fromStdString(presence.from().resource()));
////	if (presence.from().bare() == p->account->client()->jid().bare())
////		return;
////	if (!p->contacts.contains(jid)) {
////		JContact *contact = new JContact(jid, p->account);
////		contact->installEventFilter(this);
////		contact->setContactName(QString::fromStdString(presence.from().username()));
////		QStringList tags;
////		tags.append(tr("Not in list"));
////		contact->setContactTags(tags);
////		contact->setContactInList(false);
////		p->contacts.insert(jid, contact);
////	}
////	if (!resource.isEmpty())
////		p->contacts.value(jid)->setStatus(resource, presence.presence(), presence.priority(),
////										  QString::fromStdString(presence.status()));
////	if (presence.presence() != Presence::Unavailable && !presence.error()) {
////		const VCardUpdate *vcard = presence.findExtension<VCardUpdate>(ExtVCardUpdate);
////		if(vcard) {
////			QString hash = QString::fromStdString(vcard->hash());
////			JContact *contact = p->contacts.value(jid);
////			if (contact->avatarHash() != hash) {
////				if(hash.isEmpty() || QFile(p->account->getAvatarPath()%QLatin1Char('/')%hash).exists())
////					contact->setAvatar(hash);
////				else if (p->avatarsAutoLoad)
////					p->account->connection()->vCardManager()->fetchVCard(jid);
////			}
////		}
////	}
//}

//void JRoster::handleSelfPresence(const RosterItem &item, const std::string &resource,
//								 Presence::PresenceType presence, const std::string &msg)
//{
//}

//bool JRoster::handleSubscriptionRequest(const JID &jid, const std::string &msg)
//{
//	return false;
//}

//bool JRoster::handleUnsubscriptionRequest(const JID &jid, const std::string &msg)
//{
//	return true;
//}

//void JRoster::handleNonrosterPresence(const Presence &presence)
//{
//}

//void JRoster::handleRosterError(const IQ &iq)
//{
//}

//void JRoster::fillContact(JContact *contact, const RosterItem &item)
//{
//	QString name = QString::fromStdString(item.name());
//	if (contact->name() != name)
//		contact->setContactName(name);
//	QStringList tags;
//	StringList groups = item.groups();
//	StringList::const_iterator group = groups.begin();
//	for(; group != groups.end(); ++group)
//		tags.append(QString::fromStdString(*group));
//	if (contact->tags() != tags)
//		contact->setContactTags(tags);
//	if (!contact->isInList())
//		contact->setContactInList(true);
//}

//void JRoster::loadSettings()
//{
//	p->avatarsAutoLoad = JProtocol::instance()->config("general").value("getavatars", true);
//	Config config = p->account->config(QLatin1String("contactlist"));
//	foreach (const QString &jid, config.childGroups()) {
//		if (p->contacts.contains(jid))
//			continue;
//		JContact *contact = new JContact(jid, p->account);
//		contact->installEventFilter(this);
//		config.beginGroup(jid);
//		contact->setContactName(config.value("name", QString()));
//		contact->setContactTags(config.value("tags", QStringList()));
//		contact->setAvatar(config.value("avatar", QString()));
//		config.endGroup();
//		contact->setContactInList(true);
//		p->contacts.insert(jid, contact);
//		emit p->account->contactCreated(contact);
//	}
//}

//void JRoster::setOffline()
//{
//	foreach (JContact *contact, p->contacts)
//		contact->setStatus("", Presence::Unavailable, 0);
//}

//void JRoster::handleSubscription(const Subscription &subscription)
//{
//	QString jid = QString::fromStdString(subscription.from().bare());
//	QString name;
//	JContact *contact = p->contacts.value(jid);
//	if (contact) {
//		name = contact->name();
//	} else {
//		const Nickname *nickname = subscription.findExtension<Nickname>(ExtNickname);
//		name = nickname ? QString::fromStdString(nickname->nick()) : "";
//	}
//	QString text;
//	switch (subscription.subtype()) {
//	case Subscription::Subscribe:
//		if (!contact) {
//			contact = new JContact(jid, p->account);
//			contact->installEventFilter(this);
//			contact->setContactName(name);
//			contact->setContactInList(false);
//		}
//	{
//		AuthorizationDialog *dialog = AuthorizationDialog::request(contact,
//																   QString::fromStdString(subscription.status()));
//		connect(dialog, SIGNAL(finished(bool)), SLOT(sendAuthResponse(bool)));
//	}
//		break;
//	case Subscription::Subscribed:
//		text = tr("You were authorized");
//		break;
//	case Subscription::Unsubscribed:
//		text = tr("Your authorization was removed");
//		break;
//	case Subscription::Unsubscribe:
//		text = tr("Contacts's authorization was removed");
//		break;
//	case Subscription::Invalid:
//	default:
//		text = "";
//	}
//	if (!text.isEmpty()) {
//		QObject *sender = new QObject();
//		sender->setProperty("id", jid);
//		sender->setProperty("name", name);
//		Notifications::send(Notifications::System, sender, text);
//		sender->deleteLater();
//	}
//}

//void JRoster::sendAuthResponse(bool answer)
//{
//	AuthorizationDialog *dialog = qobject_cast<AuthorizationDialog *>(sender());
//	Q_ASSERT(dialog);
//	JContact *contact = qobject_cast<JContact*>(dialog->contact());
//	p->rosterManager->ackSubscriptionRequest(JID(contact->id().toStdString()), answer);
//	if (!contact->isInList()) {
//		if (answer) {
//			contact->setContactInList(true);
//			p->contacts.insert(contact->id(), contact);
//		} else {
//			contact->deleteLater();
//		}
//	}
//}

//void JRoster::handleMetaContact(const MetaContactList &mcList)
//{
//	p->atMetaContactsLoad = true;
//	p->metaContacts.clear();
//	foreach (const MetaContactListItem &item, mcList) {
//		QString jid = QString::fromStdString(JID(item.jid).bare());
//		JContact *contact = p->contacts.value(jid);
//		if (!contact)
//			continue;
//		QString tag = QString::fromStdString(item.tag);
//		MetaContact *metaContact = qobject_cast<MetaContact*>(contact->upperUnit());
//		if (metaContact && metaContact->id() == tag)
//			continue;
//		ChatUnit *unit = MetaContactManager::instance()->getUnit(tag, true);
//		metaContact = qobject_cast<MetaContact*>(unit);
//		Q_ASSERT(metaContact);
//		metaContact->addContact(contact);
//		p->metaContacts.insert(contact, metaContact);
//	}
//	p->atMetaContactsLoad = false;
//}

//bool JRoster::eventFilter(QObject *obj, QEvent *event)
//{
//	if (!p->atMetaContactsLoad && event->type() == MetaContactChangeEvent::eventType()) {
//		JContact *contact = static_cast<JContact*>(obj);
//		MetaContactChangeEvent *metaEvent = static_cast<MetaContactChangeEvent*>(event);
//		Q_ASSERT(metaEvent->contact() == contact);
//		if (metaEvent->newMetaContact())
//			p->metaContacts.insert(contact, metaEvent->newMetaContact());
//		else
//			p->metaContacts.remove(contact);
//		QMap<JContact*, MetaContact*>::iterator it = p->metaContacts.begin();
//		QMap<JContact*, MetaContact*>::iterator endit = p->metaContacts.end();
//		MetaContactList list;
//		for (; it != endit; it++) {
//			MetaContactListItem item;
//			item.jid = it.key()->id().toStdString();
//			item.tag = it.value()->id().toStdString();
//			item.order = it.value()->lowerUnits().indexOf(it.key());
//			list.push_back(item);
//		}
//		p->metaStorage->storeMetaContacts(list);
//		return false;
//	}
//	return QObject::eventFilter(obj, event);
//}

//void JRoster::onSessionCreated(qutim_sdk_0_3::ChatSession *session)
//{
//	Q_UNUSED(session);
//	//		ChatUnit *unit = session->getUnit();
//	//		if (MetaContact *meta = qobject_cast<MetaContact*>(unit)) {
//	//
//	//		}
//}


} //namespace jabber
