/****************************************************************************
 *  infometarequest.h
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

#ifndef INFOMETAREQUEST_H
#define INFOMETAREQUEST_H

#include "abstractmetarequest.h"

namespace qutim_sdk_0_3 {

namespace oscar {

class IcqAccount;
class IcqContact;
class ShortInfoMetaRequestPrivate;
class FullInfoMetaRequestPrivate;

class LIBOSCAR_EXPORT ShortInfoMetaRequest : public AbstractMetaRequest
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(ShortInfoMetaRequest)
public:
	ShortInfoMetaRequest(IcqAccount *account);
	ShortInfoMetaRequest(IcqContact *contact);
	MetaInfoValuesHash values() const;
	QVariant value(MetaField value, const QVariant &defaultValue = QVariant()) const;
	template <typename T>
	T value(MetaField value, const T &defaultValue = T());
	virtual void send() const;
protected:
	ShortInfoMetaRequest();
	virtual bool handleData(quint16 type, const DataUnit &data);
};

class LIBOSCAR_EXPORT FullInfoMetaRequest : public ShortInfoMetaRequest
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(FullInfoMetaRequest)
public:
	enum State {
		StateBasicInfo = 0x00c8,
		StateMoreInfo = 0x00dc,
		StateEmails = 0x00eb,
		StateHomepage = 0x010e,
		StateWork = 0x00d2,
		StateNotes = 0x00e6,
		StateInterests = 0x00f0,
		StateAffilations = 0x00fa
	};
	FullInfoMetaRequest(IcqAccount *account);
	FullInfoMetaRequest(IcqContact *contact);
	virtual void send() const;
signals:
	void infoUpdated(State state);
protected:
	FullInfoMetaRequest();
	virtual bool handleData(quint16 type, const DataUnit &data);
};

template <typename T>
T ShortInfoMetaRequest::value(MetaField val, const T &defaultValue) {
	QVariant res = value(val);
	if (!res.isValid())
		return defaultValue;
	return res.value<T>();
}

} } // namespace qutim_sdk_0_3::oscar

#endif // INFOMETAREQUEST_H
