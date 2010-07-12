#include "idle-global.h"
#include "idlestatuschanger.h"
#include "idlestatuswidget.h"
#include "libqutim/protocol.h"
#include "libqutim/config.h"
#include "libqutim/status.h"
#include "libqutim/settingslayer.h"
#include "libqutim/icon.h"
#include "src/modulemanagerimpl.h"
#include <QDebug>

Core::IdleStatusChanger* pIdleStatusChanger = 0;

namespace Core
{
	static CoreSingleModuleHelper<IdleStatusChanger> history_static(
		QT_TRANSLATE_NOOP("Plugin", "AutoAway"),
		QT_TRANSLATE_NOOP("Plugin", "Automatically changes status of accounts on inactivity")
	);

	IdleStatusChanger::IdleStatusChanger() :
			m_awayStatus(Status::Away), m_naStatus(Status::NA)
	{
		QObject *idle = getService("Idle");
		reloadSettings();
		m_state = Active;
		connect(idle, SIGNAL(secondsIdle(int)), this, SLOT(onIdle(int)));
		SettingsItem* settings = new GeneralSettingsItem<IdleStatusWidget>(
				Settings::General,
				Icon("user-away-extended"),
				QT_TRANSLATE_NOOP("AutoAway", "Auto-away"));
		Settings::registerItem(settings);
		pIdleStatusChanger = this;
	}

	void IdleStatusChanger::refillAccounts()
	{
		foreach (Protocol *proto, allProtocols()) {
			foreach (Account *acc, proto->accounts()) {
				if (m_accounts.contains(acc)
					|| acc->status() == Status::Offline
					|| acc->status() == Status::Invisible
					|| acc->status() == Status::DND
					|| acc->status() == Status::NA)
					continue;
				m_accounts.append(acc);
				m_statuses.append(acc->status());
			}
		}
	}

	void IdleStatusChanger::onIdle(int secs)
	{
		if ((m_awayEnabled?m_state == Away:1)
			 && secs > m_naSecs
			 && m_naEnabled) {
			refillAccounts();
			foreach (Account *acc, m_accounts) {
				if (acc)
					acc->setStatus(m_naStatus);
			}
			m_state = Inactive;
		} else if (m_state == Active
					  && secs > m_awaySecs
					  && m_awayEnabled) {
			refillAccounts();
			foreach (Account *acc, m_accounts) {
				if (acc)
					acc->setStatus(m_awayStatus);
			}
			m_state = Away;
		} else if (m_state != Active && secs < m_awaySecs) {
			for (int i = 0; i < m_accounts.size(); i++) {
				if (m_accounts.at(i))
					m_accounts.at(i)->setStatus(m_statuses.at(i));
			}
			m_accounts.clear();
			m_statuses.clear();
			m_state = Active;
		}
	}

	void IdleStatusChanger::reloadSettings()
	{
		Config conf(AA_CONFIG_GROUP);
		m_awayEnabled = conf.value("away-enabled", true);
		m_naEnabled   = conf.value("na-enabled",   true);
		m_awaySecs = conf.value("away-secs", AWAY_DEF_SECS);
		m_naSecs   = conf.value("na-secs",   NA_DEF_SECS);
		m_awayStatus.setText(conf.value("away-text", ""));
		m_naStatus.  setText(conf.value("na-text",   ""));
	}
}
