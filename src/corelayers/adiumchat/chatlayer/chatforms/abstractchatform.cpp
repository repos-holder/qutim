/****************************************************************************
 *
 *  This file is part of qutIM
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *
 ***************************************************************************
 *                                                                         *
 *   This file is part of free software; you can redistribute it and/or    *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************
 ****************************************************************************/

#include "abstractchatform.h"
#include "abstractchatwidget.h"
#include <qutim/conference.h>
#include <qutim/configbase.h>
#include <qutim/messagesession.h>
#include <qutim/debug.h>
#include "../chatsessionimpl.h"
#include <QPlainTextEdit>

namespace Core
{
namespace AdiumChat
{
AbstractChatForm::AbstractChatForm()
{
	connect(ChatLayer::instance(), SIGNAL(sessionCreated(qutim_sdk_0_3::ChatSession*)),
			SLOT(onSessionCreated(qutim_sdk_0_3::ChatSession*)));
}

void AbstractChatForm::onChatWidgetDestroyed(QObject *object)
{
	AbstractChatWidget *widget = reinterpret_cast<AbstractChatWidget*>(object);
	QString key = m_chatwidgets.key(widget);
	m_chatwidgets.remove(key);
}


QString AbstractChatForm::getWidgetId(ChatSessionImpl *sess) const
{
	ConfigGroup group = Config("behavior/chat").group("widget");
	int windows = group.value<int>("windows", 0);

	//TODO add configuration

	if (!windows) {
		return QLatin1String("session");
	} else  {
		if (qobject_cast<const Conference*>(sess->getUnit())) {
			return QLatin1String("conference");
		} else {
			return QLatin1String("chat");
		}
	}
}

void AbstractChatForm::onSessionActivated(bool active)
{
	//init or update chat widget(s)
	ChatSessionImpl *session = qobject_cast<ChatSessionImpl*>(sender());
	if (!session)
		return;
	QString key = getWidgetId(session);
	AbstractChatWidget *widget = m_chatwidgets.value(key,0);
	if (!widget)
	{
		if (!active)
			return;
		widget = createWidget(key);
		m_chatwidgets.insert(key,widget);
		connect(widget,SIGNAL(destroyed(QObject*)),SLOT(onChatWidgetDestroyed(QObject*)));
		widget->show();
	}
	if (active)
	{
		if (!widget->contains(session))
			widget->addSession(session);
		widget->activate(session);
	}
}

void AbstractChatForm::onAppearanceSettingsChanged()
{
//	qDebug("%s", Q_FUNC_INFO);
//	foreach (AbstractChatWidget *widget, m_chatwidgets) {
//		if (widget)
//			widget->loadAppearanceSettings();
//	}
}

void AbstractChatForm::onBehaviorSettingsChanged()
{
//	qDebug("%s", Q_FUNC_INFO);
//	foreach (AbstractChatWidget *widget, m_chatwidgets) {
//		if (widget)
//			widget->loadBehaviorSettings();
//	}
}

AbstractChatWidget *AbstractChatForm::findWidget(ChatSession *sess) const
{
	QHash<QString, AbstractChatWidget*>::const_iterator it;
	ChatSessionImpl *session = qobject_cast<ChatSessionImpl*>(sess);
	for (it=m_chatwidgets.constBegin();it!=m_chatwidgets.constEnd();it++) {
		if ((*it) && it.value()->contains(session))
			return it.value();
	}
	return 0;
}

QObject *AbstractChatForm::textEdit(ChatSession *session)
{
	AbstractChatWidget *widget = findWidget(session);
	if (widget && widget->currentSession() == session)
		return widget->getInputField();
	return 0;
}

QWidgetList AbstractChatForm::chatWidgets()
{
	QWidgetList list;
	foreach (QWidget *widget, m_chatwidgets)
		list << widget;
	return list;
}

QWidget* AbstractChatForm::chatWidget(ChatSession* session) const
{
	return findWidget(session);
}


void AbstractChatForm::onSessionCreated(ChatSession *session)
{
	connect(session, SIGNAL(activated(bool)), SLOT(onSessionActivated(bool)));
}

AbstractChatForm::~AbstractChatForm()
{
	foreach (AbstractChatWidget *widget,m_chatwidgets) {
		widget->disconnect(this);
		delete widget;
	}
}

}
}
