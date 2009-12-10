/****************************************************************************
 *  connection.cpp
 *
 *  Copyright (c) 2009 by Nigmatullin Ruslan <euroelessar@gmail.com>
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

#include "connection.h"
#include <QTimer>
#include <QHostInfo>
#include <QBuffer>
#include <QDebug>

namespace Icq {

quint16 generate_flap_sequence()
{
	quint32 n = qrand(), s = 0;
	for (quint32 i = n; i >>= 3; s += i);
	return ((((0 - s) ^ (quint8)n) & 7) ^ n) + 2;
}

ProtocolError::ProtocolError(const SNAC &snac)
{
	code = snac.readSimple<qint16>();
	subcode = 0;
	TLVMap tlvs = snac.readTLVChain();
	if(tlvs.contains(0x08))
	{
		DataUnit data(tlvs.value(0x08));
		subcode = data.readSimple<qint16>();
	}
	str = getErrorStr();
}

QString ProtocolError::getErrorStr()
{
	switch(code)
	{
		case(0x01):
			return "Invalid SNAC header";
		case(0x02):
			return "Server rate limit exceeded";
		case(0x03):
			return "Client rate limit exceeded";
		case(0x04):
			return "Recipient is not logged in";
		case(0x05):
			return "Requested service unavailable";
		case(0x06):
			return "Requested service not defined";
		case(0x07):
			return "You sent obsolete SNAC";
		case(0x08):
			return "Not supported by server";
		case(0x09):
			return "Not supported by client";
		case(0x0A):
			return "Refused by client";
		case(0x0B):
			return "Reply too big";
		case(0x0C):
			return "Responses lost";
		case(0x0D):
			return "Request denied";
		case(0x0E):
			return "Incorrect SNAC format";
		case(0x0F):
			return "Insufficient rights";
		case(0x10):
			return "In local permit/deny (recipient blocked)";
		case(0x11):
			return "Sender too evil";
		case(0x12):
			return "Receiver too evil";
		case(0x13):
			return "User temporarily unavailable";
		case(0x14):
			return "No match";
		case(0x15):
			return "List overflow";
		case(0x16):
			return "Request ambiguous";
		case(0x17):
			return "Server queue full";
		case(0x18):
			return "Not while on AOL";
		default:
			return "Unknown error";
	}
}

ProtocolNegotiation::ProtocolNegotiation(QObject *parent):
	SNACHandler(parent)
{
	m_infos << SNACInfo(ServiceFamily, ServiceServerReady)
			<< SNACInfo(ServiceFamily, ServiceServerNameInfo)
			<< SNACInfo(ServiceFamily, ServiceServerFamilies2)
			<< SNACInfo(ServiceFamily, ServiceServerRateInfo);
	m_login_reqinfo = qrand();
}

void ProtocolNegotiation::handleSNAC(AbstractConnection *conn, const SNAC &sn)
{
	switch((sn.family() << 16) | sn.subtype())
	{
	// Server sends supported services list
	case 0x00010003: {
		QList<quint16> services;
		while(sn.dataSize() != 0)
			services << sn.readSimple<quint16>();
		conn->setServicesList(services);
		SNAC snac(ServiceFamily, ServiceClientFamilies);
		// Sending the same as ICQ 6
		snac.appendSimple<quint32>(0x00220001);
		snac.appendSimple<quint32>(0x00010004);
		snac.appendSimple<quint32>(0x00130004);
		snac.appendSimple<quint32>(0x00020001);
		snac.appendSimple<quint32>(0x00030001);
		snac.appendSimple<quint32>(0x00150001);
		snac.appendSimple<quint32>(0x00040001);
		snac.appendSimple<quint32>(0x00060001);
		snac.appendSimple<quint32>(0x00090001);
		snac.appendSimple<quint32>(0x000a0001);
		snac.appendSimple<quint32>(0x000b0001);
		conn->send(snac);
		break; }
	// This is the reply to CLI_REQINFO
	case 0x0001000f: {

		// Skip uin
		Q_UNUSED(sn.readData<quint8>());
		sn.skipData(4);
		// Login
		//qDebug() << (m_login_reqinfo == sn.id());
		if(m_login_reqinfo == sn.id())
		{
			// TLV(x01) User type?
			// TLV(x0C) Empty CLI2CLI Direct connection info
			// TLV(x0A) External IP
			// TLV(x0F) Number of seconds that user has been online
			// TLV(x03) The online since time.
			// TLV(x0A) External IP again
			// TLV(x22) Unknown
			// TLV(x1E) Unknown: empty.
			// TLV(x05) Member of ICQ since.
			// TLV(x14) Unknown
			TLVMap tlvs = sn.readTLVChain();
			quint32 ip = tlvs.value(0x0a).value<quint32>();
			conn->setExternalIP(QHostAddress(ip));
			//qDebug() << conn->externalIP();
		}
		// Else
		else
		{
		}
		break; }
	// Server sends its services version numbers
	case 0x00010018: {
		SNAC snac(ServiceFamily, ServiceClientReqRateInfo);
		conn->send(snac);
		break; }
	// Server sends rate limits information
	case 0x00010007: {
		// Don't know what does it mean
		// TODO: Understand it
		quint16 group_count = sn.readSimple<quint16>();
		SNAC snac(ServiceFamily, ServiceClientRateAck);
		for(int i = 1; i <= group_count; i++)
			snac.appendSimple<quint16>(i);
		conn->send(snac);

		// This command requests from the server certain information about the client that is stored on the server
		// In other words: CLI_REQINFO
		snac.reset(ServiceFamily, ServiceClientReqinfo);
		m_login_reqinfo = conn->send(snac);
		break; }
	}
}


AbstractConnection::AbstractConnection(QObject *parent):
	QObject(parent)
{
	m_socket = new QTcpSocket(this);
	connect(m_socket, SIGNAL(readyRead()), SLOT(readData()));
	connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(stateChanged(QAbstractSocket::SocketState)));
	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
	m_id = (quint32)qrand();
}

void AbstractConnection::registerHandler(SNACHandler *handler)
{
	QList<SNACInfo> infos = handler->infos();
	foreach(const SNACInfo &info, infos)
		m_handlers.insertMulti((info.first << 16) | info.second, handler);
}

void AbstractConnection::send(FLAP &flap)
{
	flap.setSeqNum(seqNum());
	//qDebug("FLAP: %s", flap.toByteArray().toHex().constData());
	m_socket->write(flap.header());
	m_socket->write(flap.data());
	m_socket->flush();
}

void AbstractConnection::disconnectFromHost(bool force)
{
	Q_UNUSED(force);
	m_socket->disconnectFromHost();
}

quint32 AbstractConnection::send(SNAC &snac)
{
	qDebug("Sending SNAC: 0x%x 0x%x %s", (int)snac.family(), (int)snac.subtype(), metaObject()->className());
	FLAP flap(0x02);
	quint32 id = nextId();
	snac.setId(id);
	flap.appendData(snac);
	send(flap);
	return id;
}

void AbstractConnection::setSeqNum(quint16 seqnum)
{
	// Have a look at seqNum method to understand reasons
	m_seqnum = (seqnum > 0) ? (seqnum - 1) : 0x7fff;
}

void AbstractConnection::processNewConnection()
{
	qDebug("processNewConnection: 0x0%d %d %s", (int)flap().channel(), (int)flap().seqNum(), flap().data().toHex().constData());
}

void AbstractConnection::processCloseConnection()
{
	qDebug("processCloseConnection: 0x0%d %d %s", (int)flap().channel(), (int)flap().seqNum(), flap().data().toHex().constData());
	FLAP flap(0x04);
	flap.appendSimple<quint32>(0x00000001);
	send(flap);
	socket()->disconnectFromHost();
}

void AbstractConnection::processSnac()
{
	SNAC snac = SNAC::fromByteArray(m_flap.data());
	qDebug("Receiving SNAC: 0x%x 0x%x %s", (int)snac.family(), (int)snac.subtype(), metaObject()->className());
	bool found = false;
	foreach(SNACHandler *handler, m_handlers.values((snac.family() << 16)| snac.subtype()))
	{
		found = true;
		snac.resetState();
		handler->handleSNAC(this, snac);
	}
	if(!found)
		qWarning("No handlers for SNAC %02X %02X %s", int(snac.family()), int(snac.subtype()), metaObject()->className());
}

void AbstractConnection::readData()
{
	if(m_socket->bytesAvailable() <= 0) // Hack for windows (Qt4.6 tp1).
	{
		qDebug() << "readyRead emmited but the socket is empty";
		return;
	}
	if(m_flap.readData(m_socket))
	{
		if(m_flap.isFinished())
		{
			switch(m_flap.channel())
			{
			case 0x01:
				processNewConnection();
				break;
			case 0x02:
				processSnac();
				break;
			case 0x04:
				processCloseConnection();
				break;
			default:
				qDebug("Unknown shac channel: 0x%04X", (int)m_flap.channel());
			case 0x03:
			case 0x05:
				break;
			}
			m_flap.clear();
		}
		// Just give a chance to other parts of qutIM to do something if needed
		if(m_socket->bytesAvailable())
			QTimer::singleShot(0, this, SLOT(readData()));
	}
	else
	{
		qCritical("Strange situation at %s: %d", Q_FUNC_INFO, __LINE__);
		m_socket->close();
	}
}


void AbstractConnection::stateChanged(QAbstractSocket::SocketState state)
{
	qDebug() << "New connection state" << state;
}

void AbstractConnection::error(QAbstractSocket::SocketError error)
{
	qDebug() << IMPLEMENT_ME << error;
}

} // namespace Icq
