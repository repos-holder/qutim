#include "jcontactresource_p.h"
#include "jcontact.h"
#include "../vcard/jinforequest.h"
#include "../jaccount.h"
#include "../../jprotocol.h"
#include "jmessagesession.h"
#include "jmessagehandler.h"
#include <gloox/message.h>
#include <qutim/status.h>
#include <qutim/inforequest.h>
#include <qutim/tooltip.h>
#include <QStringBuilder>

using namespace gloox;
using namespace qutim_sdk_0_3;

namespace Jabber
{

JContactResource::JContactResource(ChatUnit *parent, const QString &name) :
	Buddy(parent->account()), d_ptr(new JContactResourcePrivate(parent))
{
	Q_D(JContactResource);
	d->name = name;
	d->id = parent->id() % QLatin1Char('/') % name;
	connect(parent, SIGNAL(avatarChanged(QString)), this, SIGNAL(avatarChanged(QString)));
}

JContactResource::JContactResource(ChatUnit *parent, JContactResourcePrivate &ptr) :
	Buddy(parent->account()), d_ptr(&ptr)
{
}

JContactResource::~JContactResource()
{
}

QString JContactResource::name() const
{
	return d_func()->name;
}

QString JContactResource::id() const
{
	return d_func()->id;
}

QString JContactResource::title() const
{
	Q_D(const JContactResource);
	if (Contact *contact = qobject_cast<Contact *>(d->contact)) {
		return contact->title() % "/" % d->name;
	} else {
		return Buddy::title();
	}
}

bool JContactResource::sendMessage(const qutim_sdk_0_3::Message &message)
{
	//		JAccount *acc = static_cast<JAccount*>(account());

	//		if (acc->status() == Status::Offline)
	//			return false;

	//		if (session()) {
	//			session()->sendMessage(message);
	//		} else {
	//			gloox::Message msg(gloox::Message::Chat, id().toStdString(), message.text().toStdString(),
	//							   message.property("subject", QString()).toStdString());
	//			acc->client()->send(msg);
	//		}
	return true;
}

void JContactResource::setPriority(int priority)
{
	d_func()->presence.setPriority(priority);
}

int JContactResource::priority()
{
	return d_func()->presence.priority();
}

void JContactResource::setStatus(const jreen::Presence presence)
{
	Q_D(JContactResource);
	Status current = status();
	d->presence = presence;
	emit statusChanged(status(), current);
}

void JContactResource::setStatus(Presence::PresenceType presence, int priority, const QString &text)
{
//	Q_D(JContactResource);
//	Status current = status();
//	d->presence = presence;
//	d->priority = priority;
//	d->text = text;
//	emit statusChanged(status(), current);
}

Status JContactResource::status() const
{
	Q_D(const JContactResource);
	Status status = JStatus::presenceToStatus(d->presence.presence());
	status.setProperty("text", d->presence.status());
	status.setExtendedInfos(d->extInfo);
	return status;
}

bool JContactResource::event(QEvent *ev)
{
	if (ev->type() == ChatStateEvent::eventType()) {
		//			Q_D(JContactResource);
		//			ChatStateEvent *chatEvent = static_cast<ChatStateEvent *>(ev);
		//TODO
		//Client *client = d->contact->account->connection()->client();
		//gloox::Message gmes(gloox::Message::Chat, d->jid.toStdString());
		//gmes.addExtension(new gloox::ChatState(qutIM2gloox(chatEvent->chatState())));
		//client->send(gmes);
	} else if (ev->type() == ToolTipEvent::eventType()) {
		ToolTipEvent *event = static_cast<ToolTipEvent*>(ev);
		event->addField(QT_TRANSLATE_NOOP("ContactResource", "Resource"),
						QString("%1 (%2)").arg(id()).arg(priority()), 75);
		if (!text().isEmpty())
			event->addField(text(), QString());
		event->addHtml("<font size=-1>", 50);
		QString client = property("client").toString();
		if (!client.isEmpty()) {
			event->addField(QT_TRANSLATE_NOOP("ContactResource", "Possible client"),
							client,
							property("clientIcon").toString(),
							ToolTipEvent::IconBeforeDescription,
							25);
			QString os = property("os").toString();
			if (!os.isEmpty())
				event->addField(QT_TRANSLATE_NOOP("ContactResource", "OS"), os, 25);
		}
		event->addHtml("</font>", 10);
		return true;
	}
	return Buddy::event(ev);
}

QSet<QString> JContactResource::features() const
{
	return d_ptr->features;
}

void JContactResource::setFeatures(const QSet<QString> &features)
{
	d_ptr->features = features;
}

bool JContactResource::checkFeature(const QLatin1String &feature) const
{
	return d_ptr->features.contains(feature);
}

bool JContactResource::checkFeature(const QString &feature) const
{
	return d_ptr->features.contains(feature);
}

bool JContactResource::checkFeature(const std::string &feature) const
{
	return d_ptr->features.contains(QString::fromStdString(feature));
}

ChatUnit *JContactResource::upperUnit()
{
	return qobject_cast<Contact *>(d_ptr->contact);
}

QString JContactResource::avatar() const
{
	if (Buddy *buddy = qobject_cast<Buddy*>(d_func()->contact))
		return buddy->avatar();
	return QString();
}

QString JContactResource::text() const
{
	return d_func()->presence.status();
}

void JContactResource::setExtendedInfo(const QString &name, const QVariantHash &info)
{
	Status current = status();
	d_func()->extInfo.insert(name, info);
	emit statusChanged(status(), current);
}

void JContactResource::removeExtendedInfo(const QString &name)
{
	Status current = status();
	d_func()->extInfo.remove(name);
	emit statusChanged(status(), current);
}

}
