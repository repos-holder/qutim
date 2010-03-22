#include "jvcardmanager.h"
#include "../roster/jcontact.h"
#include "../roster/jroster.h"
#include "../jaccount.h"
#include <QDir>
#include <gloox/client.h>
#include <gloox/vcardmanager.h>
#include <gloox/sha.h>

namespace Jabber
{
	struct JVCardManagerPrivate
	{
		JAccount *account;
		VCardManager *manager;
		QStringList contacts;
	};

	JVCardManager(JAccount *account, QObject *parent) : QObject(parent)
	{
		Q_D(JVCardManager);
		d->account = account;
		d->manager = new VCardManager(d->account->client());
	}

	~JVCardManager()
	{

	}

	VCardManager *JVCardManager::manager()
	{
		return d_ptr->manager;
	}

	void JVCardManager::fetchVCard(const QString &contact)
	{
		Q_D(JVCardManager);
		if (!d->contacts.contains(contact)) {
			d->contacts << contact;
			d->manager->fetchVCard(contact.toStdString());
		}
	}

	void JVCardManager::storeVCard(VCard *vcard)
	{
		Q_D(JVCardManager);
		d->manager->storeVCard(vcard, this);
	}

	void JVCardManager::handleVCard(const JID &jid, const VCard *fetchedVCard)
	{
		Q_D(JVCardManager);
		QString id = QString::fromStdString(jid.full());
		QString avatar;
		const VCard *vcard = (!fetchedVCard) ? new VCard() : fetchedVCard;
		VCard::Photo &photo = vcard->photo();
		if(!photo.binval.empty()) {
			QByteArray data(photo.binval.c_str(), photo.binval.length());
			SHA sha;
			sha.feed(photo.binval);
			sha.finalize();
			avatar = QString::fromStdString(sha.hex());
			QDir dir(d->account->getAvatarPath());
			if(!dir.exists())
				dir.mkpath(dir.absolutePath());
			QFile file(dir.absoluteFilePath(avatar));
			if(file.open(QIODevice::WriteOnly)) {
				file.write(data);
				file.close();
			}
		}
		if (d->account->id() == id) {
			QString nick = QString::fromStdString(vcard->nickname());
			if(nick.isEmpty())
				nick = QString::fromStdString(vcard->formattedname());
			if(nick.isEmpty())
				nick = d->account->id();
			if (d->account->nick() != nick)
				d->account->setNick(nick);
			d->account->connection()->setAvatar(avatar);
		} else {
			if (JContact *contact = qobject_cast<JContact *>(d->account->getUnit(id)))
				contact->setAvatar(avatar);
		}
		d->contacts.removeOne(id);
	}

	void JVCardManager::handleVCardResult(VCardContext context, const JID &jid, StanzaError se);
	{
		if (context == StoreVCard && se == StanzaErrorUndefined);
	}
}
