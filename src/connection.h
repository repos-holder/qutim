/****************************************************************************
 *  connection.h
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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QTcpSocket>
#include <QMap>
#include <QHostAddress>
#include <QDateTime>
#include <qutim/libqutim_global.h>
#include "snachandler.h"
#include "flap.h"

namespace Icq {

extern quint16 generate_flap_sequence();

struct ProtocolError
{
public:
	ProtocolError(const SNAC &snac);
	qint16 code;
	qint16 subcode;
	QString str;
private:
	QString getErrorStr();
};

struct OscarRate
{
public:
	OscarRate(const SNAC &sn);
	const QList<quint32> &snacTypes() { return m_snacTypes; }
	void addSnacType(quint32 snacType) { m_snacTypes << snacType; }
	quint16 groupId() { return m_groupId; }
private:
	quint16 m_groupId;
	quint32 m_windowSize;
	quint32 m_clearLevel;
	quint32 m_alertLevel;
	quint32 m_limitLevel;
	quint32 m_disconnectLevel;
	quint32 m_currentLevel;
	quint32 m_maxLevel;
	quint32 m_lastTime;
	quint8 m_currentState;
	QDateTime m_time;
	QList<quint32> m_snacTypes;
};

class ProtocolNegotiation: public SNACHandler
{
	Q_OBJECT
public:
	ProtocolNegotiation(QObject *parent = 0);
	virtual void handleSNAC(AbstractConnection *conn, const SNAC &snac);
private:
	void setMsgChannelParams(AbstractConnection *conn, quint16 chans, quint32 flags);
	quint32 m_login_reqinfo;
};


class AbstractConnection: public QObject
{
	Q_OBJECT
public:
	AbstractConnection(QObject *parent);
	~AbstractConnection();
	void registerHandler(SNACHandler *handler);
	quint32 send(SNAC &snac);
	void disconnectFromHost(bool force);
	void setExternalIP(const QHostAddress &ip) { m_ext_ip = ip; }
	const QHostAddress &externalIP() const { return m_ext_ip; }
	void setServicesList(const QList<quint16> &services) { m_services = services; };
	const QList<quint16> &servicesList() { return m_services; };
	QTcpSocket *socket() { return m_socket; };
protected:
	const FLAP &flap() { return m_flap; }
	void send(FLAP &flap);
	void setSeqNum(quint16 seqnum);
	virtual void processNewConnection();
	virtual void processCloseConnection();
private slots:
	void processSnac();
	void readData();
	void stateChanged(QAbstractSocket::SocketState);
	void error(QAbstractSocket::SocketError);
private:
	// max value is 0x7fff, min is 0
	inline quint16 seqNum() { m_seqnum++; return (m_seqnum &= 0x7fff); }
	inline quint32 nextId() { return m_id++; }
private:
	friend class ProtocolNegotiation;
	QTcpSocket *m_socket;
	FLAP m_flap;
	QMultiMap<quint32, SNACHandler*> m_handlers;
	quint16 m_seqnum;
	quint32 m_id;
	QHostAddress m_ext_ip;
	QList<quint16> m_services;
	QHash<quint16, OscarRate*> m_rates;
	QHash<quint32, OscarRate*> m_ratesHash;
};

} // namespace Icq

#endif // CONNECTION_H
