/****************************************************************************
 *  icqprotocol.cpp
 *
 *  Copyright (c) 2009 by Nigmatullin Ruslan <euroelessar@gmail.com>
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

#include "icqprotocol_p.h"
#include "icq_global.h"
#include "util.h"
#include <qutim/icon.h>
#include "icqaccount.h"
#include <QStringList>
#include <QPointer>

namespace Icq
{

IcqProtocol *IcqProtocol::self = 0;

qutim_sdk_0_3::Status icqStatusToQutim(quint16 status)
{
	if (status & IcqFlagInvisible)
		return Invisible;
	else /*if(status & IcqFlagEvil)
		return Evil;
	else if(status & IcqFlagDepress)
		return Depression;
	else if(status & IcqFlagHome)
		return AtHome;
	else if(status & IcqFlagWork)
		return AtWork;
	else if(status & IcqFlagLunch)
		return OutToLunch;
	else */if (status & IcqFlagDND)
		return DND;
	else if (status & IcqFlagOccupied)
		return Occupied;
	else if (status & IcqFlagNA)
		return NA;
	else if (status & IcqFlagAway)
		return Away;
	else if (status & IcqFlagFFC)
		return FreeChat;
	return Online;
}

IcqProtocol::IcqProtocol() :
	d_ptr(new IcqProtocolPrivate)
{
	Q_ASSERT(!self);
	self = this;
	updateSettings();
}

IcqProtocol::~IcqProtocol()
{
	self = 0;
}

void initActions(IcqProtocol *proto)
{
	static bool inited = false;
	if (inited)
		return;
	QList<ActionGenerator *> actions;
	actions << (new ActionGenerator(Icon("user-online-icq"),
				LocalizedString("Status", "Online"), proto,
				SLOT(onStatusActionPressed())))->addProperty("status", Online)->setPriority(Online);
	actions << (new ActionGenerator(Icon("user-online-chat-icq"),
				LocalizedString("Status", "Free for chat"), proto,
				SLOT(onStatusActionPressed())))->addProperty("status", FreeChat)->setPriority(FreeChat);
	actions << (new ActionGenerator(Icon("user-away-icq"),
				LocalizedString("Status", "Away"), proto,
				SLOT(onStatusActionPressed())))->addProperty("status", Away)->setPriority(Away);
	actions << (new ActionGenerator(Icon("user-away-extended-icq"),
				LocalizedString("Status", "NA"), proto,
				SLOT(onStatusActionPressed())))->addProperty("status", NA)->setPriority(NA);
	actions << (new ActionGenerator(Icon("user-busy-icq"),
				LocalizedString("Status", "DND"), proto,
				SLOT(onStatusActionPressed())))->addProperty("status", DND)->setPriority(DND);
	actions << (new ActionGenerator(Icon("user-offline-icq"),
				LocalizedString("Status", "Offline"), proto,
				SLOT(onStatusActionPressed())))->addProperty("status", Offline)->setPriority(Offline);
	foreach (ActionGenerator *action, actions)
		MenuController::addAction(action, &IcqAccount::staticMetaObject);
	inited = true;
}

void IcqProtocol::loadAccounts()
{
	Q_D(IcqProtocol);
	initActions(this);
	QStringList accounts = config("general").value("accounts", QStringList());
	foreach(const QString &uin, accounts) {
		IcqAccount *acc = new IcqAccount(uin);
		d->accounts_hash->insert(uin, acc);
		emit accountCreated(acc);
	}

}

QList<Account *> IcqProtocol::accounts() const
{
	Q_D(const IcqProtocol);
	QList<Account *> accounts;
	QHash<QString, QPointer<IcqAccount> >::const_iterator it;
	for (it = d->accounts_hash->begin(); it != d->accounts_hash->end(); it++)
		accounts.append(it.value());
	return accounts;
}

Account *IcqProtocol::account(const QString &id) const
{
	Q_D(const IcqProtocol);
	return d->accounts_hash->value(id);
}

void IcqProtocol::updateSettings()
{
	Q_D(IcqProtocol);
	QString codecName = config("general").value("codec", "System").toString();
	Util::setAsciiCodec(QTextCodec::codecForName(codecName.toLatin1()));
	foreach (QPointer<IcqAccount> acc, *d->accounts_hash)
		acc->updateSettings();
}

void IcqProtocol::onStatusActionPressed()
{
	Q_D(IcqProtocol);
	QAction *action = qobject_cast<QAction *>(sender());
	Q_ASSERT(action);
	MenuController *item = action->data().value<MenuController *>();
	if (IcqAccount *account = qobject_cast<IcqAccount *>(item))
		account->setStatus(static_cast<Status>(action->property("status").toInt()));
}

} // namespace Icq
