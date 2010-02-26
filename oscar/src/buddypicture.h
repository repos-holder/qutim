/****************************************************************************
 *  buddypicture.h
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

#ifndef BUDDYPICTURE_H_
#define BUDDYPICTURE_H_

#include "oscarconnection.h"
#include "icqcontact.h"
#include "snachandler.h"
#include "feedbag.h"
#include "roster.h"

namespace qutim_sdk_0_3 {

namespace oscar {

class IcqAccount;
class BuddyPicture;

class BuddyPicture: public AbstractConnection, public FeedbagItemHandler, public RosterPlugin
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::oscar::FeedbagItemHandler qutim_sdk_0_3::oscar::RosterPlugin)
public:
	BuddyPicture(IcqAccount *account, QObject *parent = 0);
	virtual ~BuddyPicture();
	void sendUpdatePicture(QObject *reqObject, quint16 icon_id, quint8 icon_flags, const QByteArray &icon_hash);
protected:
	void handleSNAC(AbstractConnection *conn, const SNAC &snac);
	void connectToServ(const QString &addr, quint16 port, const QByteArray &cookie);
	void processNewConnection();
	void processCloseConnection();
	virtual bool handleFeedbagItem(Feedbag *feedbag, const FeedbagItem &item, Feedbag::ModifyType type, FeedbagError error);
	virtual void statusChanged(IcqContact *contact, Status &status, const TLVMap &tlvs);
private slots:
	void disconnected();
private:
	QList<SNAC> m_history;
	bool m_is_connected;
	QByteArray m_cookie;
};

} } // namespace qutim_sdk_0_3::oscar

#endif /* BUDDYPICTURE_H_ */
