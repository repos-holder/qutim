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
#include <QHostAddress>

namespace qutim_sdk_0_3 {

namespace oscar {

struct IcqAccountPrivate;
class RosterPlugin;
class Feedbag;
class AbstractConnection;

class LIBOSCAR_EXPORT IcqAccount: public Account
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(IcqAccount)
public:
	IcqAccount(const QString &uin);
	virtual ~IcqAccount();
	virtual void setStatus(Status status);
	void setStatus(OscarStatusEnum status);
	virtual QString name() const;
	Feedbag *feedbag();
	AbstractConnection *connection();
	const AbstractConnection *connection() const;
	ChatUnit *getUnit(const QString &unitId, bool create = false);
	IcqContact *getContact(const QString &id, bool create = false);
	const QHash<QString, IcqContact*> &contacts() const;
	void setCapability(const Capability &capability, const QString &type = QString());
	bool removeCapability(const Capability &capability);
	bool removeCapability(const QString &type);
	bool containsCapability(const Capability &capability) const;
	bool containsCapability(const QString &type) const;
	QList<Capability> capabilities() const;
	void registerRosterPlugin(RosterPlugin *plugin);
signals:
	void loginFinished();
	void settingsUpdated();
public slots:
	void updateSettings();
private slots:
	void onReconnectTimeout();
protected:
	virtual bool event(QEvent *ev);
private:
	friend class Roster;
	friend class Cookie;
	friend class OscarConnection;
	QScopedPointer<IcqAccountPrivate> d_ptr;
};

} } // namespace qutim_sdk_0_3::oscar

#endif // ICQACCOUNT_H
