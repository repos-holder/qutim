/****************************************************************************
 *  mrimcontact.cpp
 *
 *  Copyright (c) 2009 by Rusanov Peter <peter.rusanov@gmail.com>
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

#include <QSet>

#include "roster.h"
#include "mrimcontact.h"

struct MrimContactPrivate
{
    quint32 id;
    QString name;
    QString email;
    quint32 groupId;
    quint32 serverFlags;
    MrimContact::ContactFlags contactflags;
    MrimConnection::FeatureFlags featureFlags;
    UserAgent userAgent;
};

MrimContact::MrimContact(MrimAccount *account)
    : Contact(account), p(new MrimContactPrivate)
{
}

MrimContact::~MrimContact()
{ }

//from Contact
QString MrimContact::id() const
{ return email(); }

void MrimContact::setName(const QString &name)
{ p->name = name; }

QSet<QString> MrimContact::tags() const
{ return QSet<QString>() << account()->roster()->groupName(p->groupId); }

void MrimContact::setTags(const QSet<QString> &tags)
{
    Q_UNUSED(tags);
    //do nothing
}

void MrimContact::sendMessage(const Message &message)
{
    //TODO:
}

bool MrimContact::isInList() const
{ return true; } //not supported yet

void MrimContact::setInList(bool inList)
{ Q_UNUSED(inList); } //not supported yet

void MrimContact::setChatState(ChatState state)
{ Q_UNUSED(state); } //not supported yet

//new
const MrimAccount *MrimContact::account() const
{ return static_cast<const MrimAccount*>(Contact::account());}

MrimAccount *MrimContact::account()
{ return static_cast<MrimAccount*>(Contact::account());}

quint32 MrimContact::contactId() const
{ return p->id; }

void MrimContact::setContactId(quint32 id)
{ p->id = id; }

quint32 MrimContact::groupId() const
{ return p->groupId; }

void MrimContact::setGroupId(quint32 id)
{ p->groupId = id; }

MrimContact::ContactFlags MrimContact::flags() const
{ return p->contactflags; }

void MrimContact::setFlags(ContactFlags flags)
{ p->contactflags = flags; }

quint32 MrimContact::serverFlags() const
{ return p->serverFlags; }

void MrimContact::setServerFlags(quint32 flags)
{ p->serverFlags = flags; }

QString MrimContact::email() const
{ return p->email; }

void MrimContact::setEmail(const QString& email)
{ p->email = email; }

MrimConnection::FeatureFlags MrimContact::featureFlags() const
{ return p->featureFlags; }

void MrimContact::setFeatureFlags(MrimConnection::FeatureFlags flags)
{ p->featureFlags = flags; }

const UserAgent& MrimContact::userAgent() const
{ return p->userAgent; }

void MrimContact::setUserAgent(const UserAgent& agent)
{ p->userAgent.set(agent); }

QDebug operator<<(QDebug dbg, const MrimContact &c)
{
	dbg.nospace() << "MrimContact (email=" << c.email() << ", name=" << c.name()
				  << ", groupId=" << c.groupId()
				  << ", contactId=" << c.contactId()
				  << ", flags=0x" << hex << c.flags()
				  << ", serverFlags=0x" << hex << c.serverFlags()
				  << ", featureFlags=0x" << hex << c.featureFlags()
				  << ", userAgent=" << c.userAgent().toReadable()
				  << ", account=" << c.account()->id()
				  << ")";

	return dbg.space();
}
