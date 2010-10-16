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
#include <QStringBuilder>
#include "chatlayerimpl.h"
#include "chatsessionmodel.h"
#include <QApplication>
#include <QPlainTextDocumentLayout>
#include "chatsessionimpl_p.h"
#include "chatforms/abstractchatform.h"
#include <qutim/message.h>
#include <qutim/account.h>
#include <qutim/notificationslayer.h>
#include <qutim/conference.h>
#include <qutim/debug.h>
#include <qutim/servicemanager.h>
//TODO temporary
#include "../webkitchat/chatstyleoutput.h"
#include "chatviewfactory.h"

namespace Core
{
	namespace AdiumChat
	{
		ChatSessionImplPrivate::ChatSessionImplPrivate() :
			controller(new ChatStyleOutput(this)),
			myself_chat_state(ChatStateInActive),
			lastStatusType(Status::Offline)
		{
		}

		ChatSessionImplPrivate::~ChatSessionImplPrivate()
		{
		}

		ChatSessionImpl::ChatSessionImpl(ChatUnit* unit, ChatLayer* chat)
			: ChatSession(chat),
			d_ptr(new ChatSessionImplPrivate)
		{
			Q_D(ChatSessionImpl);
			d->input = new QTextDocument(this);
			d->model = new ChatSessionModel(this);
			d->q_ptr = this;
			d->chat_unit = unit;
			d->input->setDocumentLayout(new QPlainTextDocumentLayout(d->input));
			Config cfg = Config("appearance").group("chat");
			d->sendToLastActiveResource = cfg.value<bool>("sendToLastActiveResource", false);
			d->active = false;

			d->controller->setChatSession(this);

			d->inactive_timer.setInterval(120000);
			d->inactive_timer.setSingleShot(true);

			connect(&d->inactive_timer,SIGNAL(timeout()),d,SLOT(onActiveTimeout()));
			d->chat_unit = 0;
			setChatUnit(unit);
		}
		
		void ChatSessionImpl::clearChat()
		{
			Q_D(ChatSessionImpl);
			d->controller->clear();
		}

		ChatSessionImpl::~ChatSessionImpl()
		{
			Q_D(ChatSessionImpl);
			if (d->menu)
				d->menu->deleteLater();
		}

		void ChatSessionImpl::addContact(Buddy* c)
		{
			//		connect(c,SIGNAL(statusChanged(qutim_sdk_0_3::Status,qutim_sdk_0_3::Status)),SLOT(statusChanged(qutim_sdk_0_3::Status)));
			d_func()->model->addContact(c);
			emit buddiesChanged();
		}

		qint64 ChatSessionImpl::appendMessage(Message &message)
		{
			Q_D(ChatSessionImpl);
			if (!message.chatUnit()) {
				qWarning() << QString("Message %1 must have a chatUnit").arg(message.text());
				message.setChatUnit(getUnit());
			}

			if (message.isIncoming())
				messageReceived(&message);
			else
				messageSent(&message);

			if (message.property("spam", false) || message.property("hide", false))
				return message.id();
			
			if ((!isActive() && !message.property("service", false))/* || !message.property("history",false)*/) {
				d->unread.append(message);
				unreadChanged(d->unread);
			}

			if (!message.isIncoming())
				setChatState(ChatStateActive);

			bool service = message.property("service").isValid();
			const Conference *conf = qobject_cast<const Conference *>(message.chatUnit());
			if (!service && !conf
				&& message.chatUnit() != d->current_unit
				&& message.isIncoming()
				&& !message.property("history", false))
			{
				d->last_active_unit = const_cast<ChatUnit*>(message.chatUnit());
			}

			bool silent = message.property("silent", false);

			if (conf) {
				silent = true;
				QString sender = conf->me() ? conf->me()->name() : QString();
				if (message.text().contains(sender)) {
					AbstractChatForm *form = ServiceManager::getByName<AbstractChatForm*>("ChatForm");
					if (form) {
						QWidget *widget = form->chatWidget(this);
						if (widget) {
							widget->raise();
						}
					}
				}
			}

			if (!silent && !d->active)
				Notifications::send(message);

			d->controller->appendMessage(message);
			return message.id();
		}

		void ChatSessionImpl::removeContact(Buddy *c)
		{
			d_func()->model->removeContact(c);
			emit buddiesChanged();
		}


		QWebPage* ChatSessionImpl::getPage() const
		{
			return d_func()->controller;
		}

		Account* ChatSessionImpl::getAccount() const
		{
			return d_func()->chat_unit->account();
		}

		QString ChatSessionImpl::getId() const
		{
			return d_func()->chat_unit->id();
		}


		ChatUnit* ChatSessionImpl::getUnit() const
		{
			return d_func()->chat_unit;
		}

		ChatUnit* ChatSessionImpl::getCurrentUnit() const
		{
			Q_D(const ChatSessionImpl);
			if (d->sendToLastActiveResource)
				return d->last_active_unit ? d->last_active_unit : d->chat_unit;
			else
				return d->current_unit ? d->current_unit : d->chat_unit;
		}

		QVariant ChatSessionImpl::evaluateJavaScript(const QString &scriptSource)
		{
			Q_D(ChatSessionImpl);
			QVariant retVal;
			QMetaObject::invokeMethod(d->controller,
									  "evaluateJavaScript",
									  Q_RETURN_ARG(QVariant,retVal),
									  Q_ARG(QString,scriptSource));
			return retVal;
		}

		void ChatSessionImpl::setActive(bool active)
		{
			Q_D(ChatSessionImpl);
			if (d->active == active)
				return;
			d->active = active;
			emit activated(active);
		}

		bool ChatSessionImpl::isActive()
		{
			return d_func()->active;
		}

		bool ChatSessionImpl::event(QEvent *ev)
		{
			return ChatSession::event(ev);
		}

		QAbstractItemModel* ChatSessionImpl::getModel() const
		{
			return d_func()->model;
		}

		void ChatSessionImplPrivate::onStatusChanged(qutim_sdk_0_3::Status status)
		{
			Contact *contact = qobject_cast<Contact *>(sender());
			if (!contact)
				return;
			statusChanged(status,contact,contact->property("silent").toBool());
		}

		ChatState ChatSessionImplPrivate::statusToState(Status::Type type)
		{
			//TODO may be need to move to protocols?
			switch(type) {
				case Status::Offline: {
					return ChatStateGone;
					break;
				}
				case Status::NA: {
					return ChatStateInActive;
					break;
				}
				case Status::Away: {
					return ChatStateInActive;
					break;
				}
				case Status::DND: {
					return ChatStateInActive;
					break;
				}
				case Status::Online: {
					return ChatStateActive;
					break;
				}
				default: {
					break;
				}
			}
			//It is a good day to die!
			return ChatStateActive;
		}

		void ChatSessionImplPrivate::statusChanged(const Status &status,Contact* contact, bool silent)
		{
			Q_Q(ChatSessionImpl);
			Notifications::Type type = Notifications::StatusChange;
			QString title = status.name().toString();


			switch(status.type()) {
				case Status::Offline: {
					type = Notifications::Offline;
					break;
				}
				case Status::Online: {
					type = Notifications::Online;
					break;
				}
				default: {
					break;
				}
			}

//			debug() << "chat state event sended";
			ChatStateEvent ev(statusToState(status.type()));
			qApp->sendEvent(q, &ev);
			
			if (lastStatusType == status.type() && lastStatusText == status.text())
				return;
			lastStatusType = status.type();
			lastStatusText = status.text();

			//title = title.isEmpty() ? contact->status().name().toString() : title;
			
			Message msg;
			msg.setChatUnit(contact);
			msg.setIncoming(true);
			msg.setProperty("service",type);
			msg.setProperty("title",title);
			msg.setTime(QDateTime::currentDateTime());
			msg.setText(status.text());
			msg.setProperty("silent",silent);
			msg.setProperty("store",!silent);
			q->appendMessage(msg);
		}

		QTextDocument* ChatSessionImpl::getInputField()
		{
			return d_func()->input;
		}

		void ChatSessionImpl::markRead(quint64 id)
		{
			Q_D(ChatSessionImpl);
			if (id == Q_UINT64_C(0xffffffffffffffff)) {
				d->unread.clear();
				unreadChanged(d->unread);
				return;
			}
			MessageList::iterator it = d->unread.begin();
			for (; it != d->unread.end(); it++) {
				if (it->id() == id) {
					d->unread.erase(it);
					unreadChanged(d->unread);
					return;
				}
			}
		}

		MessageList ChatSessionImpl::unread() const
		{
			return d_func()->unread;
		}

		void ChatSessionImpl::setChatUnit(ChatUnit* unit)
		{
			Q_D(ChatSessionImpl);
			if (d->chat_unit)
				disconnect(d->chat_unit, 0, this, 0);
			ChatUnit *oldUnit = d->chat_unit;
			static_cast<ChatLayerImpl*>(ChatLayer::instance())->onUnitChanged(oldUnit, unit);
			d->chat_unit = unit;
			connect(unit,SIGNAL(destroyed(QObject*)),SLOT(deleteLater()));
			setParent(unit);
			
			if (Contact *c = qobject_cast<Contact *>(unit)) {
				connect(c, SIGNAL(statusChanged(qutim_sdk_0_3::Status,qutim_sdk_0_3::Status)),
						d, SLOT(onStatusChanged(qutim_sdk_0_3::Status)));
				d->statusChanged(c->status(),c,true);
				setProperty("currentChatState",d->statusToState(c->status().type()));
			} else {
				//if you create a session, it is likely that the chat state is active
				setProperty("currentChatState",static_cast<int>(ChatStateActive));
				setChatState(ChatStateActive);
				
				Conference *conf;
				if (!!(conf = qobject_cast<Conference *>(oldUnit))) {
					foreach (ChatUnit *u, conf->lowerUnits()) {
						if (Buddy *buddy = qobject_cast<Buddy*>(u))
							removeContact(buddy);
					}
				}
				if (!!(conf = qobject_cast<Conference *>(unit))) {
					foreach (ChatUnit *u, conf->lowerUnits()) {
						if (Buddy *buddy = qobject_cast<Buddy*>(u))
							addContact(buddy);
					}
				}
			}		

			if (d->menu)
				d->refillMenu();

			emit chatUnitChanged(unit);
		}

		void ChatSessionImplPrivate::onActiveTimeout()
		{
//			debug() << "set inactive state";
			q_func()->setChatState(ChatStateInActive);
		}

		void ChatSessionImpl::setChatState(ChatState state)
		{
			Q_D(ChatSessionImpl);
			ChatStateEvent event(state);
			qApp->sendEvent(d->chat_unit,&event);
			d->myself_chat_state = state;
			if ((state != ChatStateInActive) && (state != ChatStateGone) && (state != ChatStateComposing)) {
				d->inactive_timer.start();
//				debug() << "timer activated";
			}
		}

		void ChatSessionImplPrivate::onResourceChosen(bool active)
		{
			if (!active)
				return;
			Q_ASSERT(qobject_cast<QAction*>(sender()));
			QAction *action = reinterpret_cast<QAction*>(sender());
			current_unit = qVariantValue<ChatUnit*>(action->data());
		}

		void ChatSessionImplPrivate::onSendToLastActiveResourceActivated(bool active)
		{
			sendToLastActiveResource = active;
		}

		void ChatSessionImplPrivate::onLowerUnitAdded()
		{
			if (!menu)
				return;
			if (menu->isVisible())
				connect(menu.data(), SIGNAL(aboutToHide()), SLOT(refillMenu()));
			else
				refillMenu();
		}

		void ChatSessionImplPrivate::refillMenu()
		{
			Q_Q(ChatSessionImpl);
			if (menu) {
				qDeleteAll(group->actions());
				ChatUnit *unit = chat_unit;
				fillMenu(menu, unit, unit->lowerUnits());
			} else {
				Q_UNUSED(q->menu());
			}
		}

		void ChatSessionImplPrivate::fillMenu(QMenu *menu, ChatUnit *unit, const ChatUnitList &lowerUnits, bool root)
		{
			Q_Q(ChatSessionImpl);
			QAction *act = new QAction(menu);
			act->setText(QT_TRANSLATE_NOOP("ChatSession", "Auto"));
			act->setData(qVariantFromValue(unit));
			act->setCheckable(true);
			act->setChecked(!sendToLastActiveResource && unit == q->getCurrentUnit());
			group->addAction(act);
			connect(act, SIGNAL(toggled(bool)), SLOT(onResourceChosen(bool)));
			menu->addAction(act);

			if (root) {
				act = new QAction(menu);
				act->setText(QT_TRANSLATE_NOOP("ChatSession", "Last active"));
				act->setCheckable(true);
				act->setChecked(sendToLastActiveResource);
				group->addAction(act);
				connect(act, SIGNAL(toggled(bool)), SLOT(onSendToLastActiveResourceActivated(bool)));
				menu->addAction(act);
			}

			menu->addSeparator();

			foreach (ChatUnit *lower, lowerUnits) {
				connect(lower, SIGNAL(lowerUnitAdded(ChatUnit*)), SLOT(onLowerUnitAdded()));
				ChatUnitList lowerLowerUnits = lower->lowerUnits();
				if (lowerLowerUnits.isEmpty()) {
					// That unit does not have any lower units
					// so just create an action for it.
					act = new QAction(menu);
					act->setText(lower->title());
					act->setData(qVariantFromValue(lower));
					act->setCheckable(true);
					act->setChecked(!sendToLastActiveResource && lower == q->getCurrentUnit());
					group->addAction(act);
					menu->addAction(act);
					connect(lower, SIGNAL(destroyed()), act, SLOT(deleteLater()));
					connect(act, SIGNAL(toggled(bool)), SLOT(onResourceChosen(bool)));
				} else {
					// That unit has lower units and we need to create a submenu for it.
					QMenu *submenu = new QMenu(lower->title(), menu);
					fillMenu(submenu, lower, lowerLowerUnits, false);
					menu->addMenu(submenu);
					connect(lower, SIGNAL(destroyed()), submenu, SLOT(deleteLater()));
				}
			}
		}

		QMenu *ChatSessionImpl::menu()
		{
			Q_D(ChatSessionImpl);
			//for JMessageSession
			//FIXME maybe need to move to the protocols
			//ChatUnit *unit = const_cast<ChatUnit*>(d->chat_unit->getHistoryUnit());
			ChatUnit *unit = d->chat_unit;
			if (!d->menu && qobject_cast<Conference*>(unit) == 0) {
				d->menu = new QMenu();
				if (!d->group) {
					d->group = new QActionGroup(d->menu);
					d->group->setExclusive(true);
				}
				d->fillMenu(d->menu, unit, unit->lowerUnits());
				connect(unit, SIGNAL(lowerUnitAdded(ChatUnit*)), d, SLOT(onLowerUnitAdded()));
			}
			return d->menu;
		}
		
		ChatState ChatSessionImpl::getChatState() const
		{
			return d_func()->myself_chat_state;
		}
	}
}
