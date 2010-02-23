/****************************************************************************
 *  icqaccount.h
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

#ifndef ICQACCOUNT_H
#define ICQACCOUNT_H

#include <qutim/account.h>
#include "cookie.h"
#include "capability.h"
#include "oscarstatus.h"

namespace qutim_sdk_0_3 {

namespace oscar {

struct IcqAccountPrivate;
class RosterPlugin;
class Roster;
class Feedbag;
class OscarConnection;

enum Visibility
{
	AllowAllUsers    = 1,
	BlockAllUsers    = 2,
	AllowPermitList  = 3,
	BlockDenyList    = 4,
	AllowContactList = 5
};

class LIBOSCAR_EXPORT IcqAccount: public Account
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(IcqAccount)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(bool avatarsSupport READ avatarsSupport)
public:
	IcqAccount(const QString &uin);
	virtual ~IcqAccount();
	virtual void setStatus(Status status);
	void setStatus(OscarStatusEnum status);
	virtual QString name() const;
	void setName(const QString &name);
	Feedbag *feedbag();
	OscarConnection *connection();
	ChatUnit *getUnit(const QString &unitId, bool create = false);
	IcqContact *getContact(const QString &id, bool create = false);
	const QHash<QString, IcqContact*> &contacts() const;
	bool avatarsSupport();
	void setCapability(const Capability &capability, const QString &type = QString());
	bool removeCapability(const Capability &capability);
	bool removeCapability(const QString &type);
	bool containsCapability(const Capability &capability);
	bool containsCapability(const QString &type);
	QList<Capability> capabilities();
	void setVisibility(Visibility visibility);
	void registerRosterPlugin(RosterPlugin *plugin);
signals:
	void loginFinished();
public slots:
	void updateSettings();
	void onReconnectTimeout();
private:
	QHash<quint64, Cookie*> &cookies();
	QString password();
	friend class Roster;
	friend class SsiHandler;
	friend class Cookie;
	friend class OscarConnection;
	QScopedPointer<IcqAccountPrivate> d_ptr;
};

} } // namespace qutim_sdk_0_3::oscar

#endif // ICQACCOUNT_H
