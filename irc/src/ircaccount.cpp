/****************************************************************************
 *  icqaccount.cpp
 *
 *  Copyright (c) 2010 by Prokhin Alexey <alexey.prokhin@yandex.ru>
 *
 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 *****************************************************************************/

#include "ircaccount_p.h"
#include "ircprotocol.h"
#include "ircconnection.h"
#include "ircchannel.h"
#include "ircchannelparticipant.h"
#include <qutim/status.h>

namespace qutim_sdk_0_3 {

namespace irc {

IrcContact *IrcAccountPrivate::newContact(const QString &nick)
{
	IrcContact *contact = new IrcContact(q, nick);
	q->connect(contact, SIGNAL(destroyed()), SLOT(onContactRemoved()));
	q->connect(contact, SIGNAL(nameChanged(QString)), SLOT(onContactNickChanged(QString)));
	contacts.insert(nick, contact);
	return contact;
}

IrcChannel *IrcAccountPrivate::newChannel(const QString &name)
{
	IrcChannel *channel = new IrcChannel(q, name);
	channels.insert(name, channel);
	return channel;
}

IrcAccount::IrcAccount(const QString &network) :
	Account(network, IrcProtocol::instance()), d(new IrcAccountPrivate)
{
	d->q = this;
	d->conn = new IrcConnection(this, this);
}

IrcAccount::~IrcAccount()
{
}

void IrcAccount::setStatus(Status status)
{
	Status current = this->status();
	if (current.type() == status.type())
		return;
	if (status == Status::Offline) {
		if (d->conn->isConnected()) {
			d->conn->disconnectFromHost(false);
		}
	} else {
		if (current == Status::Offline) {
			status = Status::Connecting;
			d->conn->connectToNetwork();
		} else {
			// Reseting away message.
		}
	}
	emit statusChanged(status);
	Account::setStatus(status);
}

QString IrcAccount::name() const
{
	return d->conn->nick();
}

ChatUnit *IrcAccount::getUnitForSession(ChatUnit *unit)
{
	if (IrcChannelParticipant *participant = qobject_cast<IrcChannelParticipant*>(unit)) {
		return participant->contact();
	}
	return unit;
}

ChatUnit *IrcAccount::getUnit(const QString &name, bool create)
{
	if (name.startsWith('#') || name.startsWith('&'))
		return 0;
	return getContact(name, true);
}

IrcChannel *IrcAccount::getChannel(const QString &name, bool create)
{
	IrcChannel *channel = d->channels.value(name);
	if (create && !channel)
		channel = d->newChannel(name);
	return channel;
}

IrcContact *IrcAccount::getContact(const QString &nick, bool create)
{
	IrcContact *contact = d->contacts.value(nick);
	if (create && !contact)
		contact = d->newContact(nick);
	return contact;
}

void IrcAccount::send(const QString &command) const
{
	d->conn->send(command);
}

void IrcAccount::setName(const QString &name) const
{
	send(QString("NICK %1").arg(name));
}

void IrcAccount::updateSettings()
{
	d->conn->loadSettings();
}

bool IrcAccount::event(QEvent *ev)
{
	return Account::event(ev);
}

void IrcAccountPrivate::removeContact(IrcContact *contact)
{
	QHash<QString, IrcContact *>::iterator itr = contacts.begin();
	QHash<QString, IrcContact *>::iterator endItr = contacts.end();
	while (itr != endItr) {
		if (*itr == contact) {
			contacts.erase(itr);
			break;
		}
		++itr;
	}
	Q_ASSERT(itr != endItr);
}

void IrcAccount::onContactRemoved()
{
	d->removeContact(reinterpret_cast<IrcContact*>(sender()));
}

void IrcAccount::onContactNickChanged(const QString &nick)
{
	Q_ASSERT(qobject_cast<IrcContact*>(sender()));
	IrcContact *contact = reinterpret_cast<IrcContact*>(sender());
	d->removeContact(contact);
	if (d->contacts.contains(nick)) {
		// It should never happen but anyway:
		d->contacts.take(nick)->deleteLater();
	}
	Q_ASSERT(contact->id() == nick);
	d->contacts.insert(nick, contact);
}

} } // namespace qutim_sdk_0_3::irc
