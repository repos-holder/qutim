#include "jaccount.h"
#include "roster/jroster.h"
#include "roster/jmessagehandler.h"
#include "servicediscovery/jservicebrowser.h"
#include "servicediscovery/jservicediscovery.h"
#include "../jprotocol.h"
#include "muc/jmucmanager.h"

namespace Jabber {

	struct JAccountPrivate
	{
		inline JAccountPrivate() : keepStatus(false), autoConnect(false) {}
		inline ~JAccountPrivate() {}
		JConnection *connection;
		JRoster *roster;
		JConnectionListener *connectionListener;
		JMessageHandler *messageHandler;
		QString passwd;
		QString nick;
		bool keepStatus;
		bool autoConnect;
		Presence::PresenceType status;
		JMUCManager *conferenceManager;
		QPointer<JServiceDiscovery> discoManager;
	};

	JAccount::JAccount(const QString &jid) : Account(jid, JProtocol::instance()), p(new JAccountPrivate)
	{
		p->discoManager = 0;
		p->connection = new JConnection(this);
		p->connectionListener = new JConnectionListener(this);
		p->roster = new JRoster(this);
		p->messageHandler = new JMessageHandler(this);
		p->conferenceManager = new JMUCManager(this);
		p->connection->initExtensions();
		loadSettings();
		autoconnect();
	}

	JAccount::~JAccount()
	{
	}

	ChatUnit *JAccount::getUnitForSession(ChatUnit *unit)
	{
		return p->messageHandler->getSession(unit);
	}

	ChatUnit *JAccount::getUnit(const QString &unitId, bool create)
	{
		ChatUnit *unit = 0;
		if (!!(unit = p->conferenceManager->muc(unitId)))
			return unit;
		if (!!(unit = p->messageHandler->getSession(unitId)))
			return unit;
		return p->roster->contact(unitId, create);
	}

	void JAccount::beginChangeStatus(Presence::PresenceType presence)
	{
		p->connection->setConnectionPresence(presence);
	}

	void JAccount::endChangeStatus(Presence::PresenceType presence)
	{
		Status newStatus = JProtocol::presenceToStatus(presence);
		if (status() == Status::Offline && newStatus != Status::Offline)
			emit stateConnected();
		if (status() != Status::Offline && newStatus == Status::Offline)
			emit stateDisconnected();
		Account::setStatus(newStatus);
		p->conferenceManager->syncBookmarks();
	}

	void JAccount::autoconnect()
	{
		if (p->autoConnect) {
			if (p->keepStatus)
				beginChangeStatus(p->status);
			else
				beginChangeStatus(Presence::Available);
		}
	}

	void JAccount::loadSettings()
	{
		p->passwd = config().group("general").value("passwd", QString(), Config::Crypted);
		p->autoConnect = config().group("general").value("autoconnect", false);
		p->keepStatus = config().group("general").value("keepstatus", true);
		p->nick = config().group("general").value("nick", id());
		p->status = static_cast<Presence::PresenceType>(
		config().group("general").value("prevstatus", 8));
	}

	JServiceDiscovery *JAccount::discoManager()
	{
		if (!p->discoManager)
			p->discoManager = new JServiceDiscovery(this);
		return p->discoManager;
	}

	const QString &JAccount::nick()
	{
		return p->nick;
	}

	const QString &JAccount::password(bool *ok)
	{
		if (ok)
			*ok = true;
		if (p->passwd.isEmpty()) {
			JInputPassword *inputPasswd = new JInputPassword(id());
			if (inputPasswd->exec()) {
				p->passwd = inputPasswd->passwd();
				if (inputPasswd->isSave()) {
					config().group("general").setValue("passwd",
							p->passwd,
							Config::Crypted);
					config().sync();
				}
			} else {
				if (ok)
					*ok = false;
			}
			inputPasswd->close();
			delete inputPasswd;
		}
		return p->passwd;
	}

	JConnection *JAccount::connection()
	{
		return p->connection;
	}

	JMessageHandler *JAccount::messageHandler()
	{
		return p->messageHandler;
	}

	gloox::Client *JAccount::client()
	{
		return p->connection->client();
	}

	JMUCManager *JAccount::conferenceManager()
	{
		return p->conferenceManager;
	}
} // Jabber namespace

