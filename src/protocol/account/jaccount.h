#ifndef JACCOUNT_H
#define JACCOUNT_H

#include <qutim/account.h>
#include <gloox/presence.h>

#include "connection/jconnection.h"
#include "connection/jconnectionlistener.h"
#include "jinputpassword.h"

namespace Jabber {

	using namespace qutim_sdk_0_3;
	using namespace gloox;

	struct JAccountPrivate;
	class JRoster;
	class JConnection;

	class JAccount : public Account
	{
		Q_OBJECT
		public:
			JAccount(const QString &jid);
			virtual ~JAccount();
			//virtual void setStatus(Status status);
			//Roster *roster();
			ChatUnit *getUnitForSession(ChatUnit *unit);
			ChatUnit *getUnit(const QString &unitId, bool create = false);
			void beginChangeStatus(Presence::PresenceType presence);
			void endChangeStatus(Presence::PresenceType presence);
			const QString &jid();
			const QString &password(bool *ok = 0);
			void autoconnect();
			JConnection *connection();
			gloox::Client *client();
		protected:
			void loadSettings();
		private:
			QScopedPointer<JAccountPrivate> p;
	};
} // Jabber namespace

#endif // JACCOUNT_H
