#ifndef JACCOUNT_P_H
#define JACCOUNT_P_H

#include "jaccount.h"
#include "muc/jbookmarkmanager.h"
//jreen
#include <jreen/client.h>
#include <jreen/message.h>

namespace jreen
{
class Client;
}

namespace Jabber {

typedef QHash<QString, QHash<QString, QString> > Identities;

class JAccountPrivate : public QObject
{
	Q_OBJECT
	Q_DECLARE_PUBLIC(JAccount)
public:
	inline JAccountPrivate(JAccount *q) : q_ptr(q),keepStatus(false) {}
	inline ~JAccountPrivate() {}
	//jreen
	jreen::Client client;
	JRoster *roster;
	JAccount *q_ptr;
	QString passwd;
	QString nick;
	bool keepStatus;
public slots:
	void onNewPresence(jreen::Presence);
	void onConnected();
	void onDisconnected();
public:	//old code
	JConnection *connection;
//	JRoster *roster;
	JConnectionListener *connectionListener;
	JMessageHandler *messageHandler;
	QVariantList toVariant(const QList<JBookmark> &list);
	Presence::PresenceType status;
	JMUCManager *conferenceManager;
	QPointer<JServiceDiscovery> discoManager;
//	QSet<QString> features;
	Identities identities;
};

}


#endif // JACCOUNT_P_H
