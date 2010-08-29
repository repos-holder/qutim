/****************************************************************************
 *  icqprotocol.cpp
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

#include "icqprotocol_p.h"
#include "icqaccount_p.h"
#include "icq_global.h"
#include "util.h"
#include "icqaccount.h"
#include <qutim/icon.h>
#include "ui/icqmainsettings.h"
#include "ui/icqaccountmainsettings.h"
#include <qutim/settingslayer.h>
#include <qutim/icon.h>
#include <QStringList>
#include <QPointer>

namespace qutim_sdk_0_3 {

namespace oscar {

IcqProtocol *IcqProtocol::self = 0;

IcqProtocol::IcqProtocol() :
	d_ptr(new IcqProtocolPrivate)
{
	Q_ASSERT(!self);
	self = this;
	Settings::registerItem(new GeneralSettingsItem<IcqMainSettings>(
			Settings::Protocol,
			Icon("im-icq"),
			QT_TRANSLATE_NOOP_UTF8("Settings", "Icq")));
	MenuController::addSettingsItem<IcqAccount>(
			new GeneralSettingsItem<IcqAccountMainSettingsWindget>(
			Settings::Protocol,
			Icon("im-icq"),
			QT_TRANSLATE_NOOP_UTF8("Settings", "Icq account settings")));

	updateSettings();
}

IcqProtocol::~IcqProtocol()
{
	self = 0;
}

void IcqProtocol::loadAccounts()
{
	Q_D(IcqProtocol);
	Q_UNUSED(OscarStatus());
	QStringList accounts = config("general").value("accounts", QStringList());
	foreach(const QString &uin, accounts) {
		IcqAccount *acc = new IcqAccount(uin);
		d->accounts_hash->insert(uin, acc);
		acc->updateSettings();
		emit accountCreated(acc);
		acc->d_func()->loadRoster();
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

QHash<QString, IcqAccount *> IcqProtocol::accountsHash() const
{
	return *d_func()->accounts;
}

void IcqProtocol::addAccount(IcqAccount *account)
{
	Q_D(IcqProtocol);
	Config cfg = config("general");
	QStringList accounts = cfg.value("accounts", QStringList());
	accounts << account->id();
	cfg.setValue("accounts", accounts);
	account->updateSettings();
	d->accounts_hash->insert(account->id(), account);
	emit accountCreated(account);
}

void IcqProtocol::updateSettings()
{
	Q_D(IcqProtocol);
	QString localeCodecName = QLatin1String(QTextCodec::codecForLocale()->name());
	QString codecName = config("general").value("codec", localeCodecName);
	Util::setAsciiCodec(QTextCodec::codecForName(codecName.toLatin1()));
	foreach (QPointer<IcqAccount> acc, *d->accounts_hash)
		acc->updateSettings();
}

QVariant IcqProtocol::data(DataType type)
{
	switch (type) {
		case ProtocolIdName:
			return "UIN";
		case ProtocolContainsContacts:
			return true;
		default:
			return QVariant();
	}
}
} } // namespace qutim_sdk_0_3::oscar
