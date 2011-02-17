/****************************************************************************
 *  ircsettingswindget.h
 *
 *  Copyright (c) 2011 by Prokhin Alexey <alexey.prokhin@yandex.ru>
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

#ifndef IRCSETTINGSWIDGET_H
#define IRCSETTINGSWIDGET_H

#include <qutim/settingswidget.h>
#include <QVBoxLayout>
#include <QSpacerItem>
#include "ircaccount.h"

namespace qutim_sdk_0_3 {

namespace irc {

template <class T>
class IrcSettingsWidget: public SettingsWidget
{
public:
	IrcSettingsWidget() :
		m_widget(0), m_layout(new QVBoxLayout(this))
	{
	}

	virtual ~IrcSettingsWidget()
	{
	}

	virtual void loadImpl()
	{
		Q_ASSERT(m_account);
		m_widget = new T(this);
		m_widget->reloadSettings(m_account);
		m_layout->addWidget(m_widget);
		m_layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
		m_widget->initSettingsWidget(this);
		foreach (QWidget *widget, m_widget->editableWidgets())
			lookForWidgetState(widget);
	}

	virtual void cancelImpl()
	{
		if (m_widget) {
			m_widget->reloadSettings(m_account);
		}
	}

	virtual void saveImpl()
	{
		if (m_widget) {
			Q_ASSERT(m_account);
			Config cfg = m_account->config();
			m_widget->saveToConfig(cfg);
			cfg.sync();
			emit saved();
			m_account->updateSettings();
		}
	}

	virtual void setController(QObject *controller)
	{
		m_account = qobject_cast<IrcAccount*>(controller);
	}

private:
	T *m_widget;
	QVBoxLayout *m_layout;
	IrcAccount *m_account;
};

} } // namespace namespace qutim_sdk_0_3::irc

#endif // IRCSETTINGSWIDGET_H
