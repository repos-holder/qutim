/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Sidorov Aleksey <sauron@citadelspb.com>
**
*****************************************************************************
**
** $QUTIM_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $QUTIM_END_LICENSE$
**
****************************************************************************/

#include "vmessages.h"
#include "vmessages_p.h"
#include "vrequest.h"
#include "vconnection.h"
#include "vaccount.h"
#include <QNetworkReply>
#include <qutim/debug.h>
#include <QtScript/QScriptEngine>
#include <qutim/message.h>
#include <QDateTime>
#include "vcontact.h"
#include <qutim/messagesession.h>
#include <qutim/json.h>
#include <QTextDocument>
#include <QApplication>

#define HAVE_ADVANCED_API 1

void VMessagesPrivate::onHistoryRecieved()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	Q_ASSERT(reply);
	QByteArray rawData = reply->readAll();
	qDebug() << rawData;
	QVariantList data = Json::parse(rawData).toMap().value("response").toList();
	for (int i = 1; i < data.size(); i++) {
		QVariantMap map = data.at(i).toMap();
		QString html = map.value("body").toString();
		Message mess;
		mess.setText(unescape(html));
		mess.setProperty("html", html);
		mess.setProperty("subject", map.value("topic"));
		mess.setIncoming(true);
		QString id = map.value("uid").toString();
		mess.setProperty("mid",map.value("mid"));
		QDateTime time = QDateTime::fromTime_t(map.value("date").toInt());
		mess.setTime(time);
		VContact *contact = connection->account()->getContact(id, false);
		if (!contact) {
			QString name = map.value("user_name").toString();
			contact = connection->account()->getContact(id, true);
			contact->setContactName(name);
		}
		mess.setChatUnit(contact);
		ChatSession *s = ChatLayer::get(contact, true);
		s->appendMessage(mess);
		unreadMess[s].append(mess);
		connect(s,SIGNAL(unreadChanged(qutim_sdk_0_3::MessageList)),SLOT(onUnreadChanged(qutim_sdk_0_3::MessageList)));
	}
}

void VMessagesPrivate::onMessageSended()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	Q_ASSERT(reply);
	QVariantMap data = Json::parse(reply->readAll()).toMap();
	
	Message message = reply->property("message").value<Message>();
	if (message.chatUnit()) {
		ChatSession *s = ChatLayer::get(const_cast<ChatUnit *>(message.chatUnit()), false);
		if (s) {
			bool success = (reply->error() == QNetworkReply::NoError)
						   && data.contains("response");
			QApplication::instance()->postEvent(s,new MessageReceiptEvent(message.id(), success));
		}
	}	
}

void VMessagesPrivate::onSmsSended()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	Q_ASSERT(reply);
	QByteArray rawData =  reply->readAll();
	QVariantMap data = Json::parse(rawData).toMap();

	debug() << rawData;
	debug() << reply->url();

	Message message = reply->property("message").value<Message>();
	if (message.chatUnit()) {
		ChatSession *s = ChatLayer::get(const_cast<ChatUnit *>(message.chatUnit()), false);
		if (s) {
			bool success = (reply->error() == QNetworkReply::NoError)
						   && data.contains("response");
			QApplication::instance()->postEvent(s,new MessageReceiptEvent(message.id(), success));
		}
	}
}

void VMessagesPrivate::onMessagesRecieved()
{
}

void VMessages::markAsRead(const QStringList &messages)
{
#ifdef HAVE_ADVANCED_API
	QVariantMap data;
	data.insert("mids", messages);
	d_func()->connection->get("messages.markAsRead", data);
#else
	Q_UNUSED(messages);
#endif
}

void VMessagesPrivate::onConnectStateChanged(VConnectionState state)
{
	Q_Q(VMessages);
	switch (state) {
		case Connected: {
			q->getHistory();
//			historyTimer.start();
			break;
		}
		case Disconnected: {
//			historyTimer.stop();
			}
			break;
		default:
			break;
	}
}

VMessages::VMessages(VConnection* connection, QObject* parent): QObject(parent), d_ptr(new VMessagesPrivate)
{
	Q_D(VMessages);
	d->q_ptr = this;
	d->connection = connection;
	loadSettings();
	connect(connection,SIGNAL(connectionStateChanged(VConnectionState)),d,SLOT(onConnectStateChanged(VConnectionState)));
//	connect(&d->historyTimer,SIGNAL(timeout()),SLOT(getHistory()));
}

VMessages::~VMessages()
{
	//saveSettings();
}

void VMessages::getLastMessages(int count)
{
	if (count < 1)
		return;
//	Q_D(VMessages);
//	QByteArray time_stamp = d->historyTimer.property("timeStamp").toByteArray();
//	QUrl url("http://userapi.com/data");
//	url.addEncodedQueryItem("act","history");
//	url.addEncodedQueryItem("message",time_stamp); //FIXME WTF ? o.O
//	url.addEncodedQueryItem("inbox",QByteArray::number(count));
//	VRequest request(url);
//	QNetworkReply *reply = d->connection->get(request);
//	connect(reply,SIGNAL(finished()),d,SLOT(onMessagesRecieved()));
}

void VMessages::getHistory()
{
	Q_D(VMessages);
	QVariantMap data;
#ifdef HAVE_ADVANCED_API
	data.insert("filters", "1");
	data.insert("preview_length", "0");
	QNetworkReply *reply = d->connection->get("messages.get", data);
#else
	//	data.insert(
	QNetworkReply *reply = d->connection->get("getMessages", data);
#endif
	connect(reply,SIGNAL(finished()),d,SLOT(onHistoryRecieved()));
}

void VMessages::sendMessage(const Message& message)
{
	Q_D(VMessages);
#ifdef HAVE_ADVANCED_API
	QVariantMap data;
	data.insert("uid", message.chatUnit()->id());
	QString html = message.property("html", Qt::escape(message.text()));
	data.insert("message", html);
	QNetworkReply *reply = d->connection->get("messages.send", data);
#endif
	reply->setProperty("message",qVariantFromValue<Message>(message));
	connect(reply,SIGNAL(finished()),d,SLOT(onMessageSended()));
}

void VMessages::sendSms(const Message &message)
{
	Q_D(VMessages);
	QVariantMap data;
	data.insert("uid", message.chatUnit()->id());
	data.insert("message",message.text());
	QNetworkReply *reply = d->connection->get("secure.sendSMSNotification", data);
	reply->setProperty("message",qVariantFromValue<Message>(message));
	connect(reply,SIGNAL(finished()),d,SLOT(onSmsSended()));
}

ConfigGroup VMessages::config()
{
	return d_func()->connection->config("messages");
}

void VMessages::loadSettings()
{
//	Q_D(VMessages);
//	ConfigGroup history = config().group("history");
//	d->historyTimer.setInterval(history.value<int>("updateInterval",15000));
//	d->historyTimer.setProperty("timeStamp",history.value("timeStamp",0));
}

void VMessages::saveSettings()
{
//	Q_D(VMessages);
//	ConfigGroup history = config().group("history");
//	history.setValue("timeStamp",d->historyTimer.property("timeStamp"));
//	history.sync();
}

void VMessagesPrivate::onUnreadChanged(const qutim_sdk_0_3::MessageList &list)
{
	ChatSession *s = qobject_cast<ChatSession*>(sender());
	Q_ASSERT(s);
	Q_UNUSED(list);
	QStringList messageIds;
	//TODO resolve problem with containers
	MessageList unread = unreadMess.value(s);
	foreach (const Message &m, unread) {
		messageIds << m.property("mid").toString();
	}
	unreadMess[s].clear();
	connection->messages()->markAsRead(messageIds);
}

