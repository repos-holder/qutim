/****************************************************************************
 *  chatsessionimpl.cpp
 *
 *  Copyright (c) 2010 by Sidorov Aleksey <sauron@citadelspb.com>
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

#include "chatsessionimpl.h"
#include "chatstyleoutput.h"
#include <QWebPage>
#include <QWebFrame>
#include "libqutim/message.h"
#include "libqutim/account.h"
#include <QTextDocument>
#include <QStringBuilder>
#include <QDateTime>
#include <QDebug>
#include "libqutim/history.h"
#include "libqutim/notificationslayer.h"
#include "chatlayerimpl.h"

namespace AdiumChat

{

	ChatSessionImpl::ChatSessionImpl ( ChatUnit* unit, ChatLayer* chat)
	: ChatSession ( chat ),m_chat_style_output(new ChatStyleOutput),m_web_page(new QWebPage)
	{
		setChatUnit(unit);
		qDebug() << "create session" << m_chat_unit->title();
		connect(unit,SIGNAL(destroyed(QObject*)),SLOT(deleteLater()));
		m_store_service_messages = Config("appearance/chat").group("general/history").value<bool>("storeServiceMessages", false);
		m_chat_style_output->preparePage(m_web_page,this);
		m_skipOneMerge = true;
		m_active = false;
		loadHistory();
	}

	void ChatSessionImpl::loadTheme(const QString& path, const QString& variant)
	{
		m_chat_style_output->loadTheme(path,variant);
		m_chat_style_output->preparePage(m_web_page,this);
	}

	void ChatSessionImpl::setVariant(const QString& variant)
	{
		m_chat_style_output->setVariant(variant);
		m_chat_style_output->reloadStyle(m_web_page);
	}

	QString ChatSessionImpl::getVariant() const
	{
		return m_chat_style_output->getVariant();
	}

	void ChatSessionImpl::loadHistory()
	{
		ConfigGroup adium_chat = Config("appearance/chat").group("general/history");
		int max_num = adium_chat.value<int>("maxDisplayMessages",5);
		MessageList messages = History::instance()->read(getUnit(), max_num);
		foreach (Message mess, messages) {
			mess.setProperty("history",true);
			if (!mess.chatUnit()) //TODO FIXME
				mess.setChatUnit(getUnit()); 
			appendMessage(mess);
		}
		m_previous_sender = 0;
		m_skipOneMerge = true;
	}

	ChatSessionImpl::~ChatSessionImpl()
	{
		if (m_web_page)
		{
			m_web_page->deleteLater();
		}
		qDebug() << "Session removed:" << m_chat_unit->title();
	}

	void ChatSessionImpl::addContact ( ChatUnit* c )
	{
//		connect(c,SIGNAL(statusChanged(qutim_sdk_0_3::Status)),SLOT(statusChanged(qutim_sdk_0_3::Status)));
	}

	void ChatSessionImpl::appendMessage ( const Message& message )
	{
		Message tmp_message = message;
		if (!tmp_message.chatUnit()) {
			qWarning() << tr("Message %1 must have a ChatUnit").arg(tmp_message.text());
			tmp_message.setChatUnit(getUnit());
		}
		if (!isActive()) {
			m_unread.append(message);
			unreadChanged(m_unread);
		}
		
		if (message.isIncoming())
			emit messageReceived(message);
		else
			emit messageSended(message);
		
		bool same_from = false;
		bool service = tmp_message.property("service").isValid();
		QString item;
		if(tmp_message.text().startsWith("/me ")) {
			tmp_message.setText(tmp_message.text().mid(3));
			tmp_message.setProperty("title",tmp_message.isIncoming() ? tmp_message.chatUnit()->title() : tmp_message.chatUnit()->account()->name());
			item = m_chat_style_output->makeAction(this,tmp_message,true);
			m_previous_sender = 0;
			m_skipOneMerge = true;
		}
		else if (service) {
			item = m_chat_style_output->makeStatus(this,tmp_message);
			m_previous_sender = 0;
			m_skipOneMerge = true;
		}
		else {
			const ChatUnit *currentSender = tmp_message.isIncoming() ? tmp_message.chatUnit() : 0;
			same_from = (!m_skipOneMerge) && (m_previous_sender == currentSender);
			item = m_chat_style_output->makeMessage(this, tmp_message, true,
															same_from );
			m_previous_sender = currentSender;
			m_skipOneMerge = false;
		}

		QString result = m_web_page->mainFrame()->evaluateJavaScript(QString("getEditedHtml(\"%1\", \"%2\");")
																	 .arg(validateCpp(item))
																	 .arg(message.id())).toString();
		QString jsTask = QString("append%2Message(\"%1\");").arg(
				result.isEmpty() ? item :
				validateCpp(result), same_from?"Next":"");
		bool isHistory = tmp_message.property("history", false);
		bool silent = tmp_message.property("silent", false);
		if (!isHistory && !silent) {
			Notifications::sendNotification(tmp_message);
		}
		if (tmp_message.property("store", true) && (!service || (service && m_store_service_messages)))
			History::instance()->store(message);
		m_web_page->mainFrame()->evaluateJavaScript(jsTask);
	}

	void ChatSessionImpl::removeContact ( ChatUnit* c )
	{

	}


	QWebPage* ChatSessionImpl::getPage() const
	{
		return m_web_page;
	}

	Account* ChatSessionImpl::getAccount() const
	{
		return m_chat_unit->account();
	}

	QString ChatSessionImpl::getId() const
	{
		return m_chat_unit->id();
	}


	ChatUnit* ChatSessionImpl::getUnit() const
	{
		return m_chat_unit;
	}

	QVariant ChatSessionImpl::evaluateJavaScript(const QString &scriptSource)
	{
		if(m_web_page.isNull())
			return QVariant();
		return m_web_page->mainFrame()->evaluateJavaScript(scriptSource);
	}

	void ChatSessionImpl::setActive(bool active)
	{
		if (m_active == active)
			return;
		m_active = active;
		emit activated(active);
	}

	bool ChatSessionImpl::isActive()
	{
		return m_active;
	}

	bool ChatSessionImpl::event(QEvent *ev)
	{
		if (ev->type() == MessageReceiptEvent::eventType()) {
			MessageReceiptEvent *msgEvent = static_cast<MessageReceiptEvent *>(ev);
			m_web_page->mainFrame()->evaluateJavaScript(QLatin1Literal("messageDlvrd(\"")
														% QString::number(msgEvent->id())
														% QLatin1Literal("\");"));
			return true;
		} else if (ev->type() == ChatStateEvent::eventType()) {
			ChatStateEvent *chatEvent = static_cast<ChatStateEvent *>(ev);
			if (chatEvent->chatState() & ChatStateComposing)
				Notifications::sendNotification(Notifications::Typing, m_chat_unit);
			return ChatSession::event(ev);
		} else {
			return ChatSession::event(ev);
		}
	}

	QAbstractItemModel* ChatSessionImpl::getItemsModel() const
	{
		return 0; //TODO
	}
	
	void ChatSessionImpl::onStatusChanged(qutim_sdk_0_3::Status status)
	{
		Contact *contact = qobject_cast<Contact *>(sender());
		if (!contact && contact->property("silent").toBool())
			return;
		QString text = contact->status().text();
		Message msg;
		msg.setChatUnit(contact);
		msg.setIncoming(true);
		msg.setProperty("service",Notifications::StatusChange);
		msg.setProperty("title",contact->title() + " " + status.name());
		msg.setTime(QDateTime::currentDateTime());
		msg.setText(text);
		appendMessage(msg);
	}
	
	QTextDocument* ChatSessionImpl::getInputField()
	{
		ChatLayerImpl *chat_layer = qobject_cast<ChatLayerImpl *>(ChatLayerImpl::instance());
		return chat_layer->getInputField(this);
	}

	void ChatSessionImpl::markRead(quint64 id)
	{
		if (id == Q_UINT64_C(0xffffffffffffffff)) {
			m_unread.clear();
			unreadChanged(m_unread);
			return;
		}
		MessageList::iterator it = m_unread.begin();
		for (; it != m_unread.end(); it++) {
			if (it->id() == id) {
				m_unread.erase(it);
				unreadChanged(m_unread);
				return;
			}
		}
	}

	MessageList ChatSessionImpl::unread() const
	{
		return m_unread;
	}

	void ChatSessionImpl::setChatUnit(ChatUnit* unit)
	{
		m_chat_unit = unit;
		Contact *c = qobject_cast<Contact *>(unit);
		if (c) {
			connect(c,SIGNAL(statusChanged(qutim_sdk_0_3::Status)),SLOT(onStatusChanged(qutim_sdk_0_3::Status)));
		}
	}
}
