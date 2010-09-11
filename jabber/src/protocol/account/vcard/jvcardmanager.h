#ifndef JVCARDMANAGER_H
#define JVCARDMANAGER_H

#include <QObject>
#include <gloox/vcardhandler.h>

namespace gloox
{
	class VCardManager;
	class Client;
}

namespace Jabber
{
	using namespace gloox;

	class JAccount;
	class JInfoRequest;
	class JVCardManagerPrivate;

	class JVCardManager : public QObject, public VCardHandler
	{
		Q_OBJECT
		Q_DECLARE_PRIVATE(JVCardManager)
		public:
			JVCardManager(JAccount *account, Client *client, QObject *parent = 0);
			~JVCardManager();
			void fetchVCard(const QString &contact, JInfoRequest *request = 0);
			void storeVCard(VCard *vcard);
			void handleVCard(const JID &jid, const VCard *fetchedVCard);
			void handleVCardResult(VCardContext context, const JID &jid, StanzaError se);
			VCardManager *manager();
		private:
			QScopedPointer<JVCardManagerPrivate> d_ptr;
	};
}

#endif // JVCARDMANAGER_H
