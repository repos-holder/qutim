/****************************************************************************
 *  xstatus.h
 *
 *  Copyright (c) 2010 by Prokhin Alexey <alexey.prokhin@yandex.ru>
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

#ifndef XTRAZ_P_H
#define XTRAZ_P_H

#include "xtraz.h"
#include "messages.h"
#include "roster.h"
#include "oscarstatus.h"
#include <QXmlStreamReader>
#include <qutim/plugin.h>

namespace qutim_sdk_0_3 {

class Account;

namespace oscar {

enum QipStatusEnum
{
	// QIP Extended Status flags
	OscarFFC            = 0x0020,
	OscarEvil           = 0x3000,
	OscarDepress        = 0x4000,
	OscarHome           = 0x5000,
	OscarWork           = 0x6000,
	OscarLunch          = 0x2001
};

struct QipExtendedStatus : public OscarStatusData
{
	QipExtendedStatus(int statusId, quint16 status, const QString &iconName,
					  const LocalizedString &name, quint16 id);
};

struct XStatus
{
	XStatus() { }
	XStatus(const QString &icon);
	XStatus(const LocalizedString &status, const QString &icon,
			qint8 mood = -1, const Capability &capability = Capability());
	QString name;
	LocalizedString value;
	ExtensionIcon icon;
	qint8 mood;
	Capability capability;
};

typedef QList<XStatus> XStatusList;
XStatusList *xstatusList();
int xstatusIndexByName(const QString &name);

class XStatusHandler: public Plugin, public Tlv2711Plugin, public RosterPlugin
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::oscar::Tlv2711Plugin qutim_sdk_0_3::oscar::RosterPlugin)
	Q_CLASSINFO("DebugName", "ICQ/Xstatus")
public:
	virtual void init();
	virtual bool load();
	virtual bool unload();
	XStatusHandler();
	static QHash<Capability, OscarStatusData> qipstatuses;
protected:
	void processTlvs2711(IcqContact *contact, Capability guid, quint16 type, const DataUnit &data, const Cookie &cookie);
	void statusChanged(IcqContact *contact, Status &status, const TLVMap &tlvs);
	XStatus findXStatus(IcqContact *contact, qint8 mood);
	void removeXStatuses(Capabilities &caps);
	void setXstatus(IcqContact *contact, const QString &title, const ExtensionIcon &icon, const QString &desc = QString());
	void setXstatus(Status &status, const QString &title, const ExtensionIcon &icon, const QString &desc = QString());
	void setAcountXstatus(IcqAccount *account, QVariantHash extStatus, const XStatus &xstatus);
	void setAcountXstatus(IcqAccount *account, QVariantHash extStatus);
	bool eventFilter(QObject *obj, QEvent *e);
private slots:
	void onSetCustomStatus(QObject *object);
	void onCustomDialogAccepted();
	void onAccountAdded(qutim_sdk_0_3::Account *account);
private:
	int m_aboutToBeChanged;
	int m_changed;
	int m_change;
};

} } // namespace qutim_sdk_0_3::oscar

#endif // XTRAZ_P_H
