#include "vcontact.h"
#include "vconnection.h"
#include "vaccount.h"
#include "vmessages.h"
#include "vroster.h"
#include <qutim/tooltip.h>

struct VContactPrivate
{
	bool online;
	QString id;
	bool inList;
	QStringList tags;
	QString name;
	QString avatar;
	QString activity;
	VAccount *account;
};


VContact::VContact(const QString& id, VAccount* account): Contact(account), d_ptr(new VContactPrivate)
{
	Q_D(VContact);
	d->id = id;
	d->account = account;
	d->online = false;
	d->inList = false;
}


QString VContact::id() const
{
	return d_func()->id;
}

bool VContact::isInList() const
{
	return d_func()->inList;
}

void VContact::sendMessage(const Message& message)
{
	d_func()->account->connection()->messages()->sendMessage(message);
}

void VContact::setTags(const QStringList& tags)
{
	d_func()->tags = tags;
}

void VContact::setInList(bool inList)
{
	d_func()->inList = inList;
}

Status VContact::status() const
{
	Status status (d_func()->online ? Status::Online : Status::Offline);
	status.initIcon("vkontakte");
	status.setText(d_func()->activity);
	return status;
}

void VContact::setStatus(bool online)
{
	Q_D(VContact);
	if (d->online != online) {
		d->online = online;
		emit statusChanged(status());
	}
}

void VContact::setActivity(const QString &activity)
{
	Q_D(VContact);
	if (d->activity != activity) {
		d->activity = activity;
		emit statusChanged(status());
	}
}

VContact::~VContact()
{

}

QStringList VContact::tags() const
{
	return d_func()->tags;
}

QString VContact::name() const
{
	return d_func()->name;
}
void VContact::setName(const QString& name)
{
	d_func()->name = name;
}

void VContact::setAvatar(const QString &avatar)
{
	Q_D(VContact);
	if (d->avatar != avatar) {
		d->avatar = avatar;
		emit avatarChanged(avatar);
	}
}

QString VContact::avatar() const
{
	return d_func()->avatar;
}

bool VContact::event(QEvent *ev)
{
	if (ev->type() == ToolTipEvent::eventType()) {
		ToolTipEvent *event = static_cast<ToolTipEvent*>(ev);
		if (!d_func()->activity.isEmpty())
			event->appendField(QT_TRANSLATE_NOOP("ContactList","Activity"),d_func()->activity);
	}
	return Contact::event(ev);
}
