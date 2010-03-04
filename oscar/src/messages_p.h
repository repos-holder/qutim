/****************************************************************************
 *  messages_p.h
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *                        Prokhin Alexey <alexey.prokhin@yandex.ru>
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

#ifndef MESSAGES_P_H_
#define MESSAGES_P_H_

#include "messages.h"
#include <qutim/messagesession.h>

namespace qutim_sdk_0_3 {

namespace oscar {

class MessagesHandler : public QObject, public SNACHandler
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::oscar::SNACHandler)
public:
	MessagesHandler(IcqAccount *account, QObject *parent = 0);
	virtual ~MessagesHandler();
	void registerMessagePlugin(MessagePlugin *plugin);
	void registerTlv2711Plugin(Tlv2711Plugin *plugin);
	virtual void handleSNAC(AbstractConnection *conn, const SNAC &snac);
private slots:
	void loginFinished();
private:
	void handleMessage(const SNAC &snac);
	void handleResponse(const SNAC &snac);
	QString handleChannel1Message(const SNAC &snac, IcqContact *contact, const TLVMap &tlvs);
	QString handleChannel2Message(const SNAC &snac, IcqContact *contact, const TLVMap &tlvs, quint64 msgCookie);
	QString handleChannel4Message(const SNAC &snac, IcqContact *contact, const TLVMap &tlvs);
	QString handleTlv2711(const DataUnit &data, IcqContact *contact, quint16 ack, const Cookie &msgCookie);
	void sendChannel2Response(IcqContact *contact, quint8 type, quint8 flags, const Cookie &cookie);
	void sendMetaInfoRequest(quint16 type);
	IcqAccount *m_account;
	QMultiHash<Capability, MessagePlugin *> m_msg_plugins;
	QMultiHash<Tlv2711Type, Tlv2711Plugin *> m_tlvs2711Plugins;
};

} } // namespace qutim_sdk_0_3::oscar

#endif /* MESSAGES_P_H_ */
