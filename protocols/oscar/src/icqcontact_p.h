/****************************************************************************
 *  icqcontact_p.h
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *                        Prokhin Alexey <alexey.prokhin@yandex.ru>
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

#include "icqcontact.h"

#ifndef ICQCONTACT_PH_H
#define ICQCONTACT_PH_H

#include "capability.h"
#include "oscarconnection.h"
#include <QDateTime>
#include <QTimer>

namespace qutim_sdk_0_3 {

namespace oscar {

inline uint qHash(const QWeakPointer<IcqContact> &ptr)
{ return qHash(ptr.data()); }

enum SsiBuddyTlvs
{
	SsiBuddyProto = 0x0084,
	SsiBuddyNick = 0x0131,
	SsiBuddyComment = 0x013c,
	SsiBuddyTags = 0x023c
};

enum ContactCapabilityFlags
{
	html_support      = 0x0001,
	utf8_support      = 0x0002,
	srvrelay_support  = 0x0004
};

class ChatStateUpdater : public QObject
{
	Q_OBJECT
public:
	ChatStateUpdater();
	void updateState(IcqContact *contact, ChatState state);
private slots:
	void sendState();
private:
	void sendState(IcqContact *contact, ChatState state);
	QHash<QWeakPointer<IcqContact>, ChatState> m_states;
	QTimer m_timer;
};

class IcqContactPrivate
{
public:
	Q_DECLARE_PUBLIC(IcqContact);
	void clearCapabilities();
	void requestNick();
	void setCapabilities(const Capabilities &caps);
	FeedbagItem getNotInListGroup();
	Q_DECLARE_FLAGS(CapabilityFlags, ContactCapabilityFlags)
	IcqAccount *account;
	QString uin;
	QString name;
	QString proto;
	Status status;
	QString avatar;
	quint16 version;
	CapabilityFlags flags;
	Capabilities capabilities;
	DirectConnectionInfo dc_info;
	QList<FeedbagItem> items;
	QStringList tags;
	ChatState state;
	QDateTime onlineSince;
	QDateTime awaySince;
	QDateTime regTime;
	IcqContact *q_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IcqContactPrivate::CapabilityFlags)

} } // namespace qutim_sdk_0_3::oscar

#endif // ICQCONTACT_PH_H
