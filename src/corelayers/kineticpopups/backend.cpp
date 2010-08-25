/****************************************************************************
 *  backend.cpp
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

#include "backend.h"
#include "popup.h"
#include "manager.h"
#include <QVariant>
#include <QTime>
#include <QDebug>
#include "popupwidget.h"
#include <qutim/configbase.h>
#include "settings/popupappearance.h"
#include <qutim/settingslayer.h>
#include <qutim/icon.h>
#include <qutim/message.h>
#include <qutim/chatunit.h>
#include <qutim/contact.h>
#include <qutim/protocol.h>
#include <QTimer>

namespace KineticPopups
{
	Backend::Backend () : m_id_counter(0)
	{
		GeneralSettingsItem<Core::PopupAppearance> *appearance = new GeneralSettingsItem<Core::PopupAppearance>(Settings::Appearance, Icon("dialog-information"), QT_TRANSLATE_NOOP("Settings","Popups"));
		Settings::registerItem(appearance);
	}

	void Backend::show(Notifications::Type type, QObject* sender, const QString& body,
					   const QVariant& data)
	{
		Manager *manager =  Manager::self();
 		if (!(manager->showFlags & type) || (manager->count() >= manager->maxCount)) {
			return;
		}

		QString text = Qt::escape(body);
		QString sender_id;
		QString sender_name;
		if (sender) {
			sender_id = sender->metaObject()->className();
			sender_name = sender->property("title").toString();
			if (sender_name.isEmpty()) {
				sender_name = sender->property("name").toString();
				if (sender_name.isEmpty())
					sender_name = sender_id;
			}
		}

		QString title = Notifications::toString(type).arg(sender_name);
		

		if (data.canConvert<Message>() && (type & Notifications::MessageSend & Notifications::MessageGet)) {
			const Message &msg = data.value<Message>();
			title = Notifications::toString(type).arg(msg.chatUnit()->title());
		} else if (data.canConvert<QString>()) {
			title = data.toString();
		}
		
		QString popup_id = title;

		bool updateMode = manager->updateMode;
		bool appendMode = manager->appendMode;

		Popup *popup = manager->getById(popup_id);
		if (popup) {
			if (appendMode) {
				updateMode ? popup->updateMessage(text) : popup->appendMessage(text);
				return;
			} else if (sender) {
				popup_id.append("." + QString::number(m_id_counter++));
			}
		}
		popup  = new Popup (popup_id);
		popup->setMessage(title,text,sender);
		popup->setData(data);
		if (sender)
			connect(sender,SIGNAL(destroyed(QObject*)),popup,SLOT(deleteLater()));
		popup->send();
	}
	
	void Backend::updateSettings()
	{
		Manager *manager = Manager::self();
		if (manager)
			manager->loadSettings();
	}
}
