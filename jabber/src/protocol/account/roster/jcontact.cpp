#include "jcontact_p.h"
#include "../jaccount.h"
#include "../../jprotocol.h"
#include <gloox/message.h>
#include <gloox/chatstate.h>
#include <gloox/rostermanager.h>
#include <gloox/rosteritem.h>
#include <QStringBuilder>
#include <qutim/debug.h>

using namespace gloox;

namespace Jabber
{
	JContact::JContact(const QString &jid, JAccount *account) : Contact(account), d_ptr(new JContactPrivate)
	{
		Q_D(JContact);
		d->account = account;
		d->jid = jid;
	}

	JContact::~JContact()
	{
	}

	QString JContact::id() const
	{
		return d_func()->jid;
	}

	void JContact::sendMessage(const qutim_sdk_0_3::Message &message)
	{
	}

	void JContact::setName(const QString &name)
	{
		Q_D(JContact);
		RosterManager *rosterManager = d->account->connection()->client()->rosterManager();
		RosterItem *item = rosterManager->getRosterItem(d->jid.toStdString());
		if (!item)
			return;
		item->setName(name.toStdString());
		rosterManager->synchronize();
		d->name = name;
		emit nameChanged(name);
	}

	QString JContact::name() const
	{
		return d_func()->name;
	}

	void JContact::setTags(const QSet<QString> &tags)
	{
		Q_D(JContact);
		if (d->tags == tags)
			return;
		d->tags = tags;
		RosterManager *rosterManager = d->account->connection()->client()->rosterManager();
		RosterItem *item = rosterManager->getRosterItem(JID(d->jid.toStdString()));
		if(!item)
			return;
		StringList stdGroups;
		foreach (QString group, d->tags) {
			stdGroups.push_back(group.toStdString());
		}
		item->setGroups(stdGroups);
		rosterManager->synchronize();
		emit tagsChanged(tags);
	}

	QSet<QString> JContact::tags() const
	{
		return d_func()->tags;
	}

	bool JContact::isInList() const
	{
		return d_func()->inList;
	}

	void JContact::setInList(bool inList)
	{
		Q_D(JContact);
		if (d->inList == inList)
			return;
		RosterManager *rosterManager = d->account->connection()->client()->rosterManager();
		if (inList)
			rosterManager->add(d->jid.toStdString(), d->name.toStdString(), StringList());
		else
			rosterManager->remove(d->jid.toStdString());
		emit inListChanged(inList);
	}

	inline gloox::ChatStateType qutIM2gloox(qutim_sdk_0_3::ChatState state)
	{
		switch (state) {
		case qutim_sdk_0_3::ChatStateActive:
			return gloox::ChatStateActive;
		case qutim_sdk_0_3::ChatStateInActive:
			return gloox::ChatStateInactive;
		case qutim_sdk_0_3::ChatStateGone:
			return gloox::ChatStateGone;
		case qutim_sdk_0_3::ChatStateComposing:
			return gloox::ChatStateComposing;
		case qutim_sdk_0_3::ChatStatePaused:
			return gloox::ChatStatePaused;
		default:
			return gloox::ChatStateInvalid;
		}
	}

	bool JContact::event(QEvent *ev)
	{
		if (ev->type() == ChatStateEvent::eventType()) {
			Q_D(JContact);
			ChatStateEvent *chatEvent = static_cast<ChatStateEvent *>(ev);
			Client *client = d->account->connection()->client();
			gloox::Message gmes(gloox::Message::Chat, d->jid.toStdString());
			gmes.addExtension(new gloox::ChatState(qutIM2gloox(chatEvent->chatState())));
			client->send(gmes);
			return true;
		}
		return Contact::event(ev);
	}

	bool JContact::hasResource(const QString &resource)
	{
		return d_func()->resources.contains(resource);
	}

	void JContact::addResource(const QString &resource)
	{
		JContactResource *res = new JContactResource(this, resource);
		d_func()->resources.insert(resource, res);
	}

	void JContact::setStatus(const QString &resource, Presence::PresenceType presence, int priority)
	{
		Q_D(JContact);
		Status oldStatus = status();
		if (presence == Presence::Unavailable && resource.isEmpty()) {
			qDeleteAll(d->resources);
			d->resources.clear();
			d->currentResources.clear();
		} else if (resource.isEmpty()) {
			return;
		} else if (presence == Presence::Unavailable) {
			if (d->resources.contains(resource))
				removeResource(resource);
		} else {
			if (!d->resources.contains(resource))
				addResource(resource);
			d->resources.value(resource)->setStatus(presence, priority);
			fillMaxResource();
		}
		Status newStatus = status();
		debug() << oldStatus.type() << newStatus.type();
		if(oldStatus.type() != newStatus.type())
			emit statusChanged(newStatus);
	}

	void JContact::removeResource(const QString &resource)
	{
		delete d_func()->resources.take(resource);
		fillMaxResource();
	}

	Status JContact::status() const
	{
		Q_D(const JContact);
		return d->currentResources.isEmpty()
				? Status::Offline : d->resources.value(d->currentResources.first())->status();
	}

	void JContact::fillMaxResource()
	{
		Q_D(JContact);
		d->currentResources.clear();
		foreach (QString resource, d->resources.keys()) {
			if (d->currentResources.isEmpty()) {
				d->currentResources << resource;
			} else {
				int prevPriority = d->resources.value(d->currentResources.first())->priority();
				if (d->resources.value(resource)->priority() > prevPriority) {
					d->currentResources.clear();
					d->currentResources << resource;
				} else if (d->resources.value(resource)->priority() == prevPriority) {
					d->currentResources << resource;
				}
			}
		}
	}

	QStringList JContact::resources()
	{
		return d_func()->resources.keys();
	}

	JContactResource *JContact::resource(const QString &key)
	{
		return d_func()->resources.value(key);
	}

	ChatUnitList JContact::lowerUnits()
	{
		ChatUnitList list;
		foreach(ChatUnit *unit, d_func()->resources)
			list << unit;
		return list;
	}

	QString JContact::avatar() const
	{
		return d_func()->avatar;
	}

	QString JContact::avatarHash() const
	{
		return d_func()->hash.toString();
	}

	void JContact::setAvatar(const QString &hex)
	{
		Q_D(JContact);
		if (d->avatar == hex)
			return;
		d->avatar = d->account->getAvatarPath() % QLatin1Char('/') % hex;
		int pos = d->avatar.lastIndexOf('/') + 1;
		int length = d->avatar.length() - pos;
		d->hash = QStringRef(&d->avatar, pos, length);
		emit avatarChanged(d->avatar);
	}
}
