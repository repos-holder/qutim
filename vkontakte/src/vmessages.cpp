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
	Q_Q(VMessages);
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	Q_ASSERT(reply);
	QByteArray rawData = reply->readAll();
	qDebug() << rawData;
	QVariantList data = Json::parse(rawData).toMap().value("response").toList();
	QStringList messIds;
	for (int i = 1; i < data.size(); i++) {
		QVariantMap map = data.at(i).toMap();
		QString html = map.value("body").toString();
		Message mess;
		mess.setText(unescape(html));
		mess.setProperty("html", html);
		mess.setProperty("topic", map.value("topic"));
		mess.setIncoming(true);
		QString id = map.value("uid").toString();
		messIds << map.value("mid").toString();
		QDateTime time = QDateTime::fromTime_t(map.value("date").toInt());
		mess.setTime(time);
		VContact *contact = connection->account()->getContact(id, false);
		if (!contact) {
			QString name = map.value("user_name").toString();
			contact = connection->account()->getContact(id, true);
			contact->setContactName(name);
		}
		mess.setChatUnit(contact);
		ChatLayer::get(contact, true)->appendMessage(mess);
	}
	if (!messIds.isEmpty())
		markAsRead(messIds);
	
//    QScriptEngine engine;
//    QScriptValue sc_data = engine.evaluate('(' + data + ')');
//	int count = sc_data.property("nm").toInteger();
//	q->getLastMessages(count);
//	historyTimer.setProperty("timeStamp",QVariant(QDateTime::currentDateTime().toTime_t()));
}

void VMessagesPrivate::onMessageSended()
{
	Q_Q(VMessages);
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

void VMessagesPrivate::onMessagesRecieved()
{
	Q_Q(VMessages);
//	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
//	Q_ASSERT(reply);
//	QByteArray data = reply->readAll();
//	
//    QScriptEngine engine;
//    QScriptValue sc_data = engine.evaluate('(' + data + ')');
//	quint32 count = sc_data.property("nm").toUInt32();
//	QString readed_messages;
//	
//    for (quint32 i=0; i<count; i++) {
//		QScriptValue sc_item = sc_data.property("inbox").property("d").property(i);
//		
//		Message mess;
//		QString id = sc_item.property("3").property(0).toString();
//		mess.setTime(QDateTime::fromTime_t(sc_item.property("1").toInteger()));
//		VContact *c = connection->account()->getContact(id,true);
//		if (!c->isInList()) {
//			c->setName(sc_item.property("3").property(1).toString());
//			c->setProperty("avatarUrl",sc_item.property("3").property(2).toVariant());
//		}
//		mess.setText(sc_item.property("2").property(0).toString());
//		mess.setProperty("html",mess.text());
//		mess.setChatUnit(c);
//		mess.setIncoming(true);
//		ChatLayer::get(c,true)->appendMessage(mess);
//		
//		readed_messages.append(sc_item.property("0").toString() + "_");
//    }
//    if (!readed_messages.isEmpty()) {
//		readed_messages.chop(1);
//		markAsRead(readed_messages);
//	}
}

void VMessagesPrivate::markAsRead(const QStringList &messages)
{
#ifdef HAVE_ADVANCED_API
	QVariantMap data;
	data.insert("mids", messages);
	connection->get("messages.markAsRead", data);
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
			historyTimer.start();
			break;
		}
		case Disconnected: {
			historyTimer.stop();
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
	d->unreadMessageCount = 0;
	loadSettings();
	connect(connection,SIGNAL(connectionStateChanged(VConnectionState)),d,SLOT(onConnectStateChanged(VConnectionState)));
	connect(&d->historyTimer,SIGNAL(timeout()),SLOT(getHistory()));
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

ConfigGroup VMessages::config()
{
	return d_func()->connection->config("messages");
}

void VMessages::loadSettings()
{
	Q_D(VMessages);
	ConfigGroup history = config().group("history");
	d->historyTimer.setInterval(history.value<int>("updateInterval",15000));
	d->historyTimer.setProperty("timeStamp",history.value("timeStamp",0));
}

void VMessages::saveSettings()
{
	Q_D(VMessages);
	ConfigGroup history = config().group("history");
	history.setValue("timeStamp",d->historyTimer.property("timeStamp"));
	history.sync();
}

