/****************************************************************************
 *  connection.h
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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QTcpSocket>
#include <QMap>
#include <QHostAddress>
#include <qutim/libqutim_global.h>
#include "snachandler.h"
#include "flap.h"

namespace qutim_sdk_0_3 {

namespace oscar {

class OscarRate;
class AbstractConnectionPrivate;

struct ProtocolError
{
public:
	ProtocolError(const SNAC &snac);
	qint16 code() { return m_code; }
	qint16 subcode() { return m_subcode; }
	QString errorString();
protected:
	qint16 m_code;
	qint16 m_subcode;
};

class AbstractConnection : public QObject, public SNACHandler
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::oscar::SNACHandler)
	Q_DECLARE_PRIVATE(AbstractConnection)
public:
	enum ConnectionError
	{
		NoError = 0x00,
		InvalidNickOrPassword = 0x01,
		ServiceUnaivalable = 0x02,
		IncorrectNickOrPassword = 0x04,
		MismatchNickOrPassword = 0x05,
		InternalClientError = 0x06,
		InvalidAccount = 0x07,
		DeletedAccount = 0x08,
		ExpiredAccount = 0x09,
		NoAccessToDatabase = 0x0a,
		NoAccessToResolver = 0x0b,
		InvalidDatabaseFields = 0x0c,
		BadDatabaseStatus = 0x0D,
		BadResolverStatus = 0x0E,
		InternalError = 0x0F,
		ServiceOffline = 0x10,
		SuspendedAccount = 0x11,
		DBSendError = 0x12,
		DBLinkError = 0x13,
		ReservationMapError = 0x14,
		ReservationLinkError = 0x15,
		ConnectionLimitExceeded = 0x16,
		ConnectionLimitExceededReservation = 0x17,
		RateLimitExceededReservation = 0x18,
		UserHeavilyWarned = 0x19,
		ReservationTimeout = 0x1a,
		ClientUpgradeRequired = 0x1b,
		ClientUpgradeRecommended = 0x1c,
		RateLimitExceeded = 0x1d,
		IcqNetworkError = 0x1e,
		InvalidSecirID = 0x20,
		AgeLimit = 0x22,
		AnotherClientLogined = 0x80
	};
public:
	explicit AbstractConnection(QObject *parent = 0);
	virtual ~AbstractConnection();
	void registerHandler(SNACHandler *handler);
	void send(SNAC &snac, bool priority = true);
	void disconnectFromHost(bool force = false);
	const QHostAddress &externalIP() const;
	const QList<quint16> &servicesList();
	QTcpSocket *socket();
	bool isConnected();
	ConnectionError error();
	QString errorString();
signals:
	void error(ConnectionError error);
protected:
	AbstractConnection(AbstractConnectionPrivate *d);
	const FLAP &flap();
	void send(FLAP &flap);
	quint32 sendSnac(SNAC &snac);
	void setSeqNum(quint16 seqnum);
	virtual void processNewConnection();
	virtual void processCloseConnection();
	void setError(ConnectionError error);
	virtual void handleSNAC(AbstractConnection *conn, const SNAC &snac);
	static quint16 generateFlapSequence();
private slots:
	void processSnac();
	void readData();
	void stateChanged(QAbstractSocket::SocketState);
	void error(QAbstractSocket::SocketError);
private:
	friend class OscarRate;
	QScopedPointer<AbstractConnectionPrivate> d_ptr;
};

} } // namespace qutim_sdk_0_3::oscar

#endif // CONNECTION_H
