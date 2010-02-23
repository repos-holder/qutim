/****************************************************************************
 *  icqprotocol.h
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

#ifndef ICQPROTOCOL_H
#define ICQPROTOCOL_H

#include <qutim/protocol.h>
#include <qutim/status.h>
#include <icq_global.h>

namespace qutim_sdk_0_3 {

namespace oscar {

struct IcqProtocolPrivate;
class IcqProtocol;

class LIBOSCAR_EXPORT IcqProtocol: public Protocol
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(IcqProtocol)
	Q_CLASSINFO("Protocol", "icq")
public:
	IcqProtocol();
	virtual ~IcqProtocol();
	static inline IcqProtocol *instance() { if (!self) qWarning("IcqProtocol isn't created"); return self; }
	virtual QList<Account *> accounts() const;
	virtual Account *account(const QString &id) const;
public slots:
	void updateSettings();
protected:
	void loadAccounts();
private slots:
	void onStatusActionPressed();
private:
	friend class IcqAccountCreationWizard;
	QScopedPointer<IcqProtocolPrivate> d_ptr;
	static IcqProtocol *self;
};

} } // namespace qutim_sdk_0_3::oscar

#endif // ICQPROTOCOL_H
