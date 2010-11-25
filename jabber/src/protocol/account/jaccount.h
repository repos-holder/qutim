/****************************************************************************
 *  jaccount.h
 *
 *  Copyright (c) 2010 by Sidorov Aleksey <sauron@citadelspb.com>
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

#ifndef JACCOUNT_H
#define JACCOUNT_H

#include <qutim/account.h>
#include <gloox/presence.h>

#include "connection/jconnection.h"
#include "connection/jconnectionlistener.h"

namespace jreen
{
	class Client;
	class JID;
}

namespace Jabber {

using namespace qutim_sdk_0_3;
using namespace gloox;

class JAccountPrivate;
class JRoster;
class JConnection;
class JMessageHandler;
class JServiceDiscovery;
class JMUCManager;

class JAccount : public Account
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(JAccount)
public:
	JAccount(const QString &jid);
	virtual ~JAccount();
	ChatUnit *getUnitForSession(ChatUnit *unit);
	ChatUnit *getUnit(const QString &unitId, bool create = false);
	void beginChangeStatus(Presence::PresenceType presence);
	void endChangeStatus(Presence::PresenceType presence);
	QString name() const;
	void setNick(const QString &nick);
	const QString &password(bool *ok = 0);
	JConnection *connection();
	JMessageHandler *messageHandler();
	jreen::Client *client() const;
	JServiceDiscovery *discoManager();
	JMUCManager *conferenceManager();
	virtual void setStatus(Status status);
	void setAccountStatus(Status status);
	QString getAvatarPath();
	void setAvatar(const QString &hex);
	bool event(QEvent *);
	QSet<QString> features() const;
	bool checkFeature(const QString &feature) const;
	bool checkFeature(const std::string &feature) const;
	bool checkIdentity(const QString &category, const QString &type) const;
	bool checkIdentity(const std::string &category, const std::string &type) const;
	QString identity(const QString &category, const QString &type) const;
	std::string identity(const std::string &category, const std::string &type) const;
	void setPasswd(const QString &passwd);
protected:
	void loadSettings();
private:
	//jreen
	friend class JRoster;
	//old code
	friend class JServerDiscoInfo;
	//temporary hack for old code
	QScopedPointer<JAccountPrivate> d_ptr;
	JAccountPrivate *p;
};
} // Jabber namespace

#endif // JACCOUNT_H
