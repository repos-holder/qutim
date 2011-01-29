#ifndef JACCOUNT_P_H
#define JACCOUNT_P_H

#include "jaccount.h"
#include "muc/jbookmarkmanager.h"
//jreen
#include <jreen/client.h>
#include <jreen/privacymanager.h>
//#include <jreen/pubsubmanager.h>
#include "vcard/jvcardmanager.h"
#include <sdk/jabber.h>
#include <QSignalMapper>
namespace jreen
{
class Client;
}

namespace Jabber {

typedef QHash<QString, QHash<QString, QString> > Identities;

class JAccountPrivate
{
	Q_DECLARE_PUBLIC(JAccount)
public:
	inline JAccountPrivate(JAccount *q) : q_ptr(q),keepStatus(false) {}
	inline ~JAccountPrivate() {}
	//jreen
	jreen::Client client;
	JSoftwareDetection *softwareDetection;
	JRoster *roster;
	jreen::PrivacyManager *privacyManager;
	JMessageSessionManager *messageSessionManager;
	JVCardManager *vCardManager;
	jreen::PubSub::Manager *pubSubManager;
	jreen::PrivateXml *privateXml;
	JMessageHandler *messageHandler;
	QPointer<JServiceDiscovery> discoManager;
	JAccount *q_ptr;
	QString nick;
	bool keepStatus;
	Status status;
	bool isOnline;
	QList<JabberExtension *> extensions;
	JabberParams params;
	QPointer<JMUCManager> conferenceManager;
	QSignalMapper signalMapper;
	int loadedModules;
	int priority;
	
	void applyStatus(const Status &status);
	void setPresence(jreen::Presence);
	void _q_disconnected(jreen::Client::DisconnectReason);
	void _q_init_extensions(const QSet<QString> &features);
	void _q_on_module_loaded(int i);
	void _q_connected();

	//old code
	Identities identities;
};

}


#endif // JACCOUNT_P_H
