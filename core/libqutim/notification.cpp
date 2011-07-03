/****************************************************************************
 *  notification.cpp
 *
 *  Copyright (c) 2011 by Sidorov Aleksey <sauron@citadelspb.com>
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

#include "notification.h"
#include "sound_p.h"
#include "dynamicpropertydata_p.h"
#include "message.h"
#include "chatunit.h"
#include "chatsession.h"
#include "metaobjectbuilder.h"
#include "conference.h"
#include "event.h"
#include <QMetaEnum>
#include <QMetaMethod>
#include <QMultiMap>
#include <QApplication>

namespace qutim_sdk_0_3 {

static SoundHandler handler; //TODO move to other place

typedef QHash<QByteArray, NotificationBackend*> NotificationBackendHash;
Q_GLOBAL_STATIC(NotificationBackendHash, backendHash)

typedef QMultiHash<Notification::Type, QByteArray> DisabledBackends;
Q_GLOBAL_STATIC(DisabledBackends, disabledBackends)

typedef QHash<Notification::Type, NotificationAction> ActionHash;
Q_GLOBAL_STATIC(ActionHash, globalActions)

typedef QMultiMap<int, NotificationFilter*> HandlerMap;
Q_GLOBAL_STATIC(HandlerMap, handlers)

class NotificationPrivate
{
public:
	NotificationPrivate() :
		state(Notification::Active)
	{}
	NotificationRequest request;
	QAtomicInt ref;
	Notification::State state;
};

class NotificationRequestPrivate : public DynamicPropertyData
{
public:
	NotificationRequestPrivate() : DynamicPropertyData(), object(0)
	{
		enabledBackends = backendHash()->keys().toSet();
	}
	NotificationRequestPrivate(const NotificationRequestPrivate& o) :
		DynamicPropertyData(o), object(o.object), pixmap(o.pixmap), text(o.text),
		title(o.title), type(o.type), actions(o.actions) {}
	QObject *object;
	QPixmap pixmap;
	QString text;
	QString title;
	Notification::Type type;
	QList<NotificationAction> actions;
	QSet<QByteArray> enabledBackends;
	QSet<QByteArray> rejectionReasons;
};

class NotificationActionPrivate : public QSharedData
{
public:
	NotificationActionPrivate() :
		type(NotificationAction::AdditionalButton)
	{}
	QIcon icon;
	LocalizedString title;
	QPointer<QObject> receiver;
	QByteArray method;
	QPointer<Notification> notification;
	NotificationAction::Type type;
};

class NotificationBackendPrivate
{
public:
	QByteArray type;
	LocalizedString description;
	QSet<QByteArray> allowedRejectedNotifications;
};

Notification *Notification::send(const Message &msg)
{
	NotificationRequest request(msg);
	return request.send();
}

Notification *Notification::send(const QString &text)
{
	NotificationRequest request(System);
	request.setText(text);
	return request.send();
}

Notification::Notification(const NotificationRequest &request) :
	d_ptr(new NotificationPrivate)
{
	Q_D(Notification);
	d->request = request;

	QList<NotificationAction> &actions = d->request.d_ptr->actions;
	QList<NotificationAction>::iterator itr = actions.begin();
	while (itr != actions.end()) {
		itr->d->notification = this;
		++itr;
	}
}

Notification::~Notification()
{
}

NotificationRequest Notification::request() const
{
	return d_func()->request;
}

Notification::State Notification::state()
{
	return d_func()->state;
}

void Notification::accept()
{
	Q_D(Notification);
	if (d->state != Active)
		return;
	d->state = Accepted;
	foreach (const NotificationAction &action, d->request.actions()) {
		if (action.type() == NotificationAction::AcceptButton)
			action.trigger();
	}
	emit accepted();
	emit finished(Accepted);
}

void Notification::ignore()
{
	Q_D(Notification);
	if (d->state != Active)
		return;
	d->state = Ignored;
	foreach (const NotificationAction &action, d->request.actions()) {
		if (action.type() == NotificationAction::IgnoreButton)
			action.trigger();
	}
	emit ignored();
	emit finished(Ignored);
}

void Notification::reject()
{
	Q_D(Notification);
	if (d->state != Active)
		return;
	d->state = Rejected;
	emit rejected();
	emit finished(Rejected);
}

LocalizedStringList Notification::typeStrings()
{
	static LocalizedStringList list;
	if (list.isEmpty()) {
		list << QT_TRANSLATE_NOOP("Notification", "Incoming Message")
			 << QT_TRANSLATE_NOOP("Notification", "Outgoing Message")
			 << QT_TRANSLATE_NOOP("Notification", "qutIM Startup")
			 << QT_TRANSLATE_NOOP("Notification", "Blocked Message")
			 << QT_TRANSLATE_NOOP("Notification", "Contact joined conference")
			 << QT_TRANSLATE_NOOP("Notification", "Contact left conference")
			 << QT_TRANSLATE_NOOP("Notification", "Incoming conference message")
			 << QT_TRANSLATE_NOOP("Notification", "Outgoing conference message")
			 << QT_TRANSLATE_NOOP("Notification", "File transfer completed")
			 << QT_TRANSLATE_NOOP("Notification", "Contact online")
			 << QT_TRANSLATE_NOOP("Notification", "Contact offline")
			 << QT_TRANSLATE_NOOP("Notification", "Contact changed status")
			 << QT_TRANSLATE_NOOP("Notification", "Contact birthday")
			 << QT_TRANSLATE_NOOP("Notification", "Contact typing")
			 << QT_TRANSLATE_NOOP("Notification", "System");
	}
	return list;
}

LocalizedString Notification::typeString(Type type)
{
	return typeStrings().value(type);
}

LocalizedStringList Notification::descriptionStrings()
{
	static LocalizedStringList list;
	if (list.isEmpty()) {
		list << QT_TRANSLATE_NOOP("Notification", "A new message has been received")
			 << QT_TRANSLATE_NOOP("Notification", "A message has been sent")
			 << QT_TRANSLATE_NOOP("Notification", "qutIM has started")
			 << QT_TRANSLATE_NOOP("Notification", "A message has been blocked")
			 << QT_TRANSLATE_NOOP("Notification", "A user has joined a conference")
			 << QT_TRANSLATE_NOOP("Notification", "A user has left a conference")
			 << QT_TRANSLATE_NOOP("Notification", "A new conference message has been received")
			 << QT_TRANSLATE_NOOP("Notification", "A conference message has been sent")
			 << QT_TRANSLATE_NOOP("Notification", "A file transfer has been completed")
			 << QT_TRANSLATE_NOOP("Notification", "A contact has gone online")
			 << QT_TRANSLATE_NOOP("Notification", "A contact has gone offline")
			 << QT_TRANSLATE_NOOP("Notification", "A contact has changed status")
			 << QT_TRANSLATE_NOOP("Notification", "A contact has birthday!")
			 << QT_TRANSLATE_NOOP("Notification", "A contact is typing")
			 << QT_TRANSLATE_NOOP("Notification", "A system notification");
	}
	return list;
}

LocalizedString Notification::descriptionString(Type type)
{
	return descriptionStrings().value(type);
}

NotificationAction::NotificationAction(const QIcon &icon, const LocalizedString &title,
									   QObject *receiver, const char *method) :
	d(new NotificationActionPrivate)
{
	d->icon = icon;
	d->title = title;
	d->receiver = receiver;
	d->method = method;
}

NotificationAction::NotificationAction(const LocalizedString &title,
									   QObject *receiver, const char *method) :
	d(new NotificationActionPrivate)
{
	d->title = title;
	d->receiver = receiver;
	d->method = method;
}

NotificationAction::NotificationAction() :
	d(new NotificationActionPrivate)
{

}

NotificationAction::NotificationAction(const NotificationAction &action) :
	d(action.d)
{
}

NotificationAction::~NotificationAction()
{
}

const NotificationAction &NotificationAction::operator=(const NotificationAction &rhs)
{
	this->d = rhs.d;
	return *this;
}

QIcon NotificationAction::icon() const
{
	return d->icon;
}

LocalizedString NotificationAction::title() const
{
	return d->title;
}

NotificationAction::Type NotificationAction::type() const
{
	return d->type;
}

void NotificationAction::setType(NotificationAction::Type type)
{
	d->type = type;
}

QObject *NotificationAction::receiver() const
{
	return d->receiver;
}

const char *NotificationAction::method() const
{
	return d->method.constData();
}

void NotificationAction::trigger() const
{
	if (!d->receiver || !d->notification)
		return;

	const QMetaObject *meta = d->receiver->metaObject();
	const char *name = d->method.constData();
	const char type = name[0];
	QByteArray tmp = QMetaObject::normalizedSignature(name + 1);
	name = tmp.constData();

	int index = -1;
	switch (type) {
	case '0':
		index = meta->indexOfMethod(name);
		break;
	case '1':
		index = meta->indexOfSlot(name);
		break;
	case '2':
		index = meta->indexOfSignal(name);
		break;
	default:
		break;
	}

	if (index != -1) {
		meta->method(index).invoke(d->receiver,
								   Q_ARG(NotificationRequest, d->notification->request()));
	} else {
		warning() << "An invalid action has been triggered" << name;
	}

	if (d->type == AcceptButton)
		d->notification->accept();
	else if (d->type == IgnoreButton)
		d->notification->ignore();
}

namespace CompiledProperty
{
static QList<QByteArray> names = QList<QByteArray>() << "type"
													 << "text"
													 << "title"
													 << "object"
													 << "image"
													 << "actions";
static QList<Getter> getters   = QList<Getter>() //TODO
;
static QList<Setter> setters   = QList<Setter>() //TODO
;
}

NotificationRequest::NotificationRequest() :
	d_ptr(new NotificationRequestPrivate)
{
}

NotificationRequest::NotificationRequest(const Message &msg) :
	d_ptr(new NotificationRequestPrivate)
{
	d_ptr->text = msg.text();
	d_ptr->object = msg.chatUnit();

	if (qobject_cast<Conference*>(msg.chatUnit())) {
		d_ptr->type = msg.isIncoming() ? Notification::ChatIncomingMessage :
										 Notification::ChatOutgoingMessage;
	} else {
		d_ptr->type = msg.isIncoming() ? Notification::IncomingMessage :
										 Notification::OutgoingMessage;
	}
	setProperty("message", qVariantFromValue(msg));
}

NotificationRequest::NotificationRequest(Notification::Type type) :
	d_ptr(new NotificationRequestPrivate)
{
	d_ptr->type = type;
}

NotificationRequest::NotificationRequest(Buddy *buddy, const Status &status, const Status &previous) :
	d_ptr(new NotificationRequestPrivate)
{
	d_ptr->text = status.text();
	d_ptr->object = buddy;
	setProperty("status", qVariantFromValue(status));
	setProperty("previousStatus", qVariantFromValue(previous));

	Status::Type statusType = status.type();
	d_ptr->type = Notification::UserChangedStatus;
	if (statusType == Status::Offline)
		d_ptr->type = Notification::UserOffline;
	else if (previous.type() == Status::Offline)
		d_ptr->type = Notification::UserOnline;
}

NotificationRequest::NotificationRequest(const NotificationRequest &other)
{
	d_ptr = other.d_ptr;
}

NotificationRequest::~NotificationRequest()
{
}

NotificationRequest &NotificationRequest::operator =(const NotificationRequest &other)
{
	d_ptr = other.d_ptr;
	return *this;
}

void NotificationRequest::setObject(QObject *obj)
{
	d_ptr->object = obj;
}

QObject *NotificationRequest::object() const
{
	return d_ptr->object;
}

void NotificationRequest::setImage(const QPixmap &pixmap)
{
	d_ptr->pixmap = pixmap;
}

QPixmap NotificationRequest::image() const
{
	return d_ptr->pixmap;
}

void NotificationRequest::setTitle(const QString &title)
{
	d_ptr->title = title;
}

QString NotificationRequest::title() const
{
	return d_ptr->title;
}

void NotificationRequest::setText(const QString &text)
{
	d_ptr->text = text;
}

QString NotificationRequest::text() const
{
	return d_ptr->text;
}

void NotificationRequest::setType(Notification::Type type)
{
	d_ptr->type = type;
}

Notification::Type NotificationRequest::type() const
{
	return d_ptr->type;
}

void NotificationRequest::reject(const QByteArray &reason)
{
	d_ptr->rejectionReasons.insert(reason);
}

QSet<QByteArray> NotificationRequest::rejectionReasons() const
{
	return d_ptr->rejectionReasons;
}

void NotificationRequest::setBackends(const QSet<QByteArray> &backendTypes)
{
	d_ptr->enabledBackends = backendTypes;
}

void NotificationRequest::blockBackend(const QByteArray &backendType)
{
	d_ptr->enabledBackends.remove(backendType);
}

void NotificationRequest::unblockBackend(const QByteArray &backendType)
{
	d_ptr->enabledBackends.insert(backendType);
}

bool NotificationRequest::isBackendBlocked(const QByteArray &backendType)
{
	return !d_ptr->enabledBackends.contains(backendType);
}

void NotificationRequest::blockBackend(Notification::Type type, const QByteArray &backendType)
{
	disabledBackends()->insert(type, backendType);
}

void NotificationRequest::unblockBackend(Notification::Type type, const QByteArray &backendType)
{
	disabledBackends()->remove(type, backendType);
}

bool NotificationRequest::isBackendBlocked(Notification::Type type, const QByteArray &backendType)
{
	return disabledBackends()->contains(type, backendType);
}

QVariant NotificationRequest::property(const char *name, const QVariant &def) const
{
	return d_ptr->property(name, def, CompiledProperty::names, CompiledProperty::getters);
}

void NotificationRequest::setProperty(const char *name, const QVariant &value)
{
	d_ptr->setProperty(name, value, CompiledProperty::names, CompiledProperty::setters);
}

void NotificationRequest::addAction(const NotificationAction &action_helper)
{
	NotificationAction action = action_helper;
	d_ptr->actions.push_back(action);
}

void NotificationRequest::addAction(Notification::Type type, const NotificationAction &action)
{
	globalActions()->insert(type, action);
}

QList<NotificationAction> NotificationRequest::actions() const
{
	QList<NotificationAction> actions = d_ptr->actions;
	actions += globalActions()->values(d_ptr->type);
	return actions;
}

Notification *NotificationRequest::send()
{
	HandlerMap::iterator itr = handlers()->end();
	HandlerMap::iterator begin = handlers()->begin();
	while (itr != begin) {
		--itr;
		(*itr)->filter(*this);
	}

	Notification *notification = 0;
	foreach (NotificationBackend *backend, *backendHash()) {
		// Check that the notification has not been blocked for the backend
		QByteArray typeName = backend->backendType();
		if (isBackendBlocked(d_ptr->type, typeName) || isBackendBlocked(typeName))
			continue;

		// Check that the notifications has not been rejected
		QSet<QByteArray> allowed = backend->d_ptr->allowedRejectedNotifications;
		QSet<QByteArray> rejectionReasons = d_ptr->rejectionReasons - allowed;
		if (!rejectionReasons.isEmpty())
			continue;

		if (!notification) {
			notification = new Notification(*this);
			notification->d_func()->ref.ref();
			foreach (NotificationFilter *filter, *handlers())
				filter->notificationCreated(notification);
		}

		backend->handleNotification(notification);
	}
	if (notification)
		notification->d_func()->ref.deref();
	//TODO ref and deref impl
	return notification;
}

NotificationFilter::~NotificationFilter()
{

}

void NotificationFilter::registerFilter(NotificationFilter *handler, int priority)
{
	handlers()->insert(priority, handler);
}

void NotificationFilter::unregisterFilter(NotificationFilter *handler)
{
	HandlerMap::iterator itr = handlers()->begin();
	HandlerMap::iterator end = handlers()->end();
	while (itr != end) {
		if (*itr == handler)
			itr = handlers()->erase(itr);
		else
			++itr;
	}
}

void NotificationFilter::notificationCreated(Notification *notification)
{
	Q_UNUSED(notification);
}

void NotificationFilter::virtual_hook(int id, void *data)
{
	Q_UNUSED(id);
	Q_UNUSED(data);
}

NotificationBackend::NotificationBackend(const QByteArray &type) :
	d_ptr(new NotificationBackendPrivate)
{
	Q_ASSERT(!type.isEmpty());
	d_ptr->type = type;
	backendHash()->insert(d_ptr->type, this);

	if (qApp) {
		static quint16 eventType = Event::registerType("notification-backend-registered");
		Event event(eventType);
		event.args[0] = type;
		event.args[1] = qVariantFromValue(this);
		event.send();
	}
}

NotificationBackend::~NotificationBackend()
{
	NotificationBackendHash::iterator itr = backendHash()->find(d_ptr->type);
	Q_ASSERT(itr != backendHash()->end());
	if (*itr == this)
		backendHash()->erase(itr);

	if (qApp) {
		static quint16 eventType = Event::registerType("notification-backend-removed");
		Event event(eventType);
		event.args[0] = d_ptr->type;
		event.args[1] = qVariantFromValue(this);
		event.send();
	}
}

QByteArray NotificationBackend::backendType() const
{
	return d_ptr->type;
}

LocalizedString NotificationBackend::description() const
{
	return d_ptr->description;
}

QList<QByteArray> NotificationBackend::allTypes()
{
	return backendHash()->keys();
}

NotificationBackend* NotificationBackend::get(const QByteArray &type)
{
	return backendHash()->value(type);
}

QList<NotificationBackend*> NotificationBackend::all()
{
	return backendHash()->values();
}

void NotificationBackend::ref(Notification *notification)
{
	notification->d_func()->ref.ref();
}

void NotificationBackend::deref(Notification *notification)
{
	if (!notification->d_func()->ref.deref())
		notification->deleteLater();
}

void NotificationBackend::setDescription(const LocalizedString &description)
{
	d_ptr->description = description;
}

void NotificationBackend::allowRejectedNotifications(const QByteArray &reason)
{
	d_ptr->allowedRejectedNotifications.insert(reason);
}

void NotificationBackend::virtual_hook(int id, void *data)
{
	Q_UNUSED(id);
	Q_UNUSED(data);
}

} // namespace qutim_sdk_0_3
