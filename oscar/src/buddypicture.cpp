/****************************************************************************
 *  buddypicture.cpp
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

#include "buddypicture.h"
#include "qutim/systeminfo.h"
#include "qutim/protocol.h"
#include <qutim/debug.h>
#include "icqaccount.h"
#include <QSet>
#include <QDir>
#include <QFile>

#include <QImage>

namespace Icq
{

BuddyPictureConnection::BuddyPictureConnection(QObject *parent) :
	AbstractConnection(parent)
{
}

void BuddyPictureConnection::connectToServ(const QString &addr, quint16 port, const QByteArray &cookie)
{
	m_cookie = cookie;
	socket()->connectToHost(addr, port);
}

void BuddyPictureConnection::processNewConnection()
{
	FLAP flap(0x01);
	flap.appendSimple<quint32>(0x01);
	flap.appendTLV<QByteArray>(0x0006, m_cookie);
	send(flap);
}

void BuddyPictureConnection::processCloseConnection()
{
	AbstractConnection::processCloseConnection();
}

BuddyPicture::BuddyPicture(IcqAccount *account, QObject *parent) :
	ProtocolNegotiation(parent), m_account(account), m_is_connected(false)
{
	m_infos << SNACInfo(ServiceFamily, ServerRedirectService)
			<< SNACInfo(AvatarFamily, AvatarGetReply);
	m_conn = new BuddyPictureConnection(this);
	m_conn->registerHandler(this);
	connect(m_conn->socket(), SIGNAL(disconnected()), SLOT(disconnected()));
}

BuddyPicture::~BuddyPicture()
{

}

void BuddyPicture::handleSNAC(AbstractConnection *conn, const SNAC &snac)
{
	if (conn == m_conn) {
		ProtocolNegotiation::handleSNAC(conn, snac);
		snac.resetState();
		if (snac.family() == ServiceFamily && snac.subtype() == ServiceServerRateInfo) {
			SNAC snac(ServiceFamily, ServiceClientReady);
			snac.appendData(QByteArray::fromHex(
					"0001 0004 0110 164f" // ServiceFamily
					"000f 0001 0110 164f"));// AvatarFamily
			conn->send(snac);
			m_is_connected = true;
			while (!m_history.isEmpty()) {
				SNAC tmp = m_history.takeFirst();
				conn->send(tmp);
			}
		}
	} else {
		if (snac.family() == ServiceFamily && snac.subtype() == ServerRedirectService) {
			TLVMap tlvs = snac.readTLVChain();
			quint16 id = tlvs.value(0x0D).value<quint16>();
			if (id == AvatarFamily) {
				QList<QByteArray> list = tlvs.value(0x05).value().split(':');
				m_conn->connectToServ(list.at(0), list.size() > 1 ? atoi(list.at(1).constData()) : 5190, tlvs.value(0x06).value());
			}
		}
	}
	switch ((snac.family() << 16) | snac.subtype()) {
	case AvatarFamily << 16 | AvatarGetReply: {
		QString uin = snac.readString<quint8>();
		QObject *obj;
		if (uin == m_account->id())
			obj = m_account;
		else
			obj = m_account->getUnit(uin, false);
		if (!obj)
			break;
		snac.skipData(3); // skip icon_id and icon_flag
		QByteArray hash = snac.readData<quint8>();
		snac.skipData(21);
		QByteArray image = snac.readData<quint16>();
		if (!image.isEmpty()) {
			QString image_path = QString("%1/%2.%3/avatars/") .arg(SystemInfo::getPath(SystemInfo::ConfigDir)) .arg(m_account->protocol()->id()) .arg(m_account->id());
			QDir dir(image_path);
			if (!dir.exists())
				dir.mkpath(image_path);
			image_path += hash.toHex();
			QFile icon_file(image_path);
			if (!icon_file.exists()) {
				if (icon_file.open(QIODevice::WriteOnly))
					icon_file.write(image);
				obj->setProperty("icon_hash", hash);
			}
			obj->setProperty("avatar", image_path);
		}
		break;
	}
	}
}

void BuddyPicture::sendUpdatePicture(QObject *reqObject, quint16 icon_id, quint8 icon_flags, const QByteArray &icon_hash)
{
	if (m_conn->socket()->state() == QTcpSocket::UnconnectedState)
		return;
	QByteArray old_hash = reqObject->property("icon_hash").toByteArray();
	if (old_hash != icon_hash) {
		SNAC snac(AvatarFamily, AvatarGetRequest);
		snac.appendData<quint8>(reqObject->property("id").toString());
		snac.appendSimple<quint8>(1); // unknown
		snac.appendSimple<quint16>(icon_id);
		snac.appendSimple<quint8>(icon_flags);
		snac.appendData<quint8>(icon_hash);
		if (m_is_connected)
			m_conn->send(snac);
		else
			m_history.push_back(snac);
	}
}

void BuddyPicture::disconnected()
{
	m_is_connected = false;
	m_history.clear();
}

} // namespace Icq
