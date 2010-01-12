/****************************************************************************
 *  messages.h
 *
 *  Copyright (c) 2009 by Prokhin Alexey <alexey.prokhin@yandex.ru>
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

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include "snac.h"
#include "snachandler.h"
#include "messageplugin.h"
#include <QDateTime>

namespace Icq
{

class IcqAccount;
class IcqContact;

enum Channel1Codec
{
	CodecUsAscii = 0x0000,
	CodecUtf16Be = 0x0002,
	CodecAnsi    = 0x0003
};

enum MessageType
{
	MsgPlain    = 0x01,
	MsgChatReq  = 0x02,
	MsgFileReq  = 0x03,
	MsgUrl      = 0x04,
	MsgAuthReq  = 0x06,
	MsgAuthDeny = 0x07,
	MsgAuthOk   = 0x08,
	MsgServer   = 0x09,
	MsgAdded    = 0x0C,
	MsgWwp      = 0x0D,
	MsgEexpress = 0x0E,
	MsgPlugin   = 0x1A,
	MsgAutoAway = 0xE8,
	MsgAutoBusy = 0xE9,
	MsgAutoNA   = 0xEA,
	MsgAutoDND  = 0xEB,
	MsgAutoFFC  = 0xEC,
	MsgUnknown  = 0x00,
};

// Typing notification statuses
enum MTN
{
	MtnUnknown  = 0xFFFF,
	MtnFinished = 0x0000,
	MtnTyped    = 0x0001,
	MtnBegun    = 0x0002,
	MtnGone     = 0x000F
};

class Channel1MessageData: public DataUnit
{
public:
	Channel1MessageData(const QString &message, Channel1Codec charset = CodecUtf16Be);
};

class Tlv2711: public DataUnit
{
public:
	Tlv2711(quint8 msgType, quint8 msgFlags, quint16 X1, quint16 X2, const Cookie &cookie = Cookie(true));
	void appendEmptyPacket();
	void appendColors();
	const Cookie &cookie() const { return m_cookie; }
private:
	Cookie m_cookie;
};

class Channel2BasicMessageData: public DataUnit
{
public:
	Channel2BasicMessageData(quint16 command, const Capability &guid, const Cookie &cookie = Cookie(true));
	const Cookie &cookie() const { return m_cookie; }
private:
	Cookie m_cookie;
};

class Channel2MessageData: public Channel2BasicMessageData
{
public:
	Channel2MessageData(quint16 ackType, const Tlv2711 &data);
};

class ServerMessage: public SNAC
{
public:
	ServerMessage();
	ServerMessage(IcqContact *contact, const Channel1MessageData &data, const Cookie &cookie, bool storeMessage = true);
	ServerMessage(IcqContact *contact, const Channel2BasicMessageData &data);
protected:
	void init(IcqContact *contact, qint16 channel, const Cookie &cookie = Cookie(true));
};

class ServerResponseMessage: public SNAC
{
public:
	ServerResponseMessage(IcqContact *contact, quint16 format, quint16 reason, const Cookie &cookie = Cookie(true));
};

class MessagesHandler: public SNACHandler
{
Q_OBJECT
public:
	MessagesHandler(IcqAccount *account, QObject *parent = 0);
	virtual void handleSNAC(AbstractConnection *conn, const SNAC &snac);
private:
	void handleMessage(const SNAC &snac);
	void handleResponse(const SNAC &snac);
	void handleChannel1Message(const SNAC &snac, IcqContact *contact, const QString &uin, const TLVMap &tlvs);
	void handleChannel2Message(const SNAC &snac, IcqContact *contact, const QString &uin, const TLVMap &tlvs, quint64 msgCookie);
	void handleChannel4Message(const SNAC &snac, IcqContact *contact, const QString &uin, const TLVMap &tlvs);
	void handleTlv2711(const DataUnit &data, IcqContact *contact, quint16 ack, const Cookie &msgCookie);
	void appendMessage(IcqContact *contact, const QString &message, QDateTime time = QDateTime());
	void sendChannel2Response(IcqContact *contact, quint8 type, quint8 flags, const Cookie &cookie);
	IcqAccount *m_account;
	QMultiHash<Capability, MessagePlugin *> m_msg_plugins;
};

} // namespace Icq

#endif /* MESSAGES_H_ */
