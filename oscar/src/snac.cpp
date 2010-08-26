/****************************************************************************
 *  snac.cpp
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

#include "snac.h"
#include "util.h"

namespace qutim_sdk_0_3 {

namespace oscar {

SNAC::SNAC(quint16 family, quint16 subtype) :
	m_receiver(0), m_member(0)
{
	reset(family, subtype);
}

SNAC::~SNAC()
{
	if (m_member)
		delete [] m_member;
}

void SNAC::reset(quint16 family, quint16 subtype)
{
	m_family = family;
	m_subtype = subtype;
	m_flags = 0;
	m_data.clear();
}

SNAC SNAC::fromByteArray(const QByteArray &d)
{
	DataUnit data(d);
	quint16 family = data.read<quint16>();
	quint16 subtype = data.read<quint16>();
	SNAC snac(family, subtype);
	snac.m_flags = data.read<quint16>();
	snac.m_id = data.read<quint32>();
	snac.m_data = data.readAll();
	return snac;
}

QByteArray SNAC::toByteArray() const
{
	QByteArray data = header();
	data += m_data;
	return data;
}

QByteArray SNAC::header() const
{
	DataUnit data;
	data.append<quint16>(m_family);
	data.append<quint16>(m_subtype);
	data.append<quint16>(m_flags);
	data.append<quint32>(m_id);
	return data.data();
}

void SNAC::setCookie(const Cookie &cookie, QObject *receiver, const char *member, int msec)
{
	m_cookie = cookie;
	m_receiver = receiver;
	if (m_member) {
		m_member = new char [strlen(member) + 1];
		strcpy(m_member, member);
	}
	m_msec = msec;
}

void SNAC::lock()
{
	if (!m_cookie.isEmpty()) {
		m_cookie.lock(m_receiver, m_member, m_msec);
		delete [] m_member; m_member = 0;
	}
}

} } // namespace qutim_sdk_0_3::oscar
