/****************************************************************************
 *  oscarproxy.h
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

#ifndef OSCARPROXY_H
#define OSCARPROXY_H

#include "icq_global.h"
#include <qutim/networkproxy.h>

namespace qutim_sdk_0_3 {
namespace oscar {

class OscarProxyManager : public QObject, public NetworkProxyManager
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::NetworkProxyManager)
public:
	OscarProxyManager();
	QList<NetworkProxyInfo*> proxies();
	void setProxy(Account *account, NetworkProxyInfo *proxy, const DataItem &settings);
};

} } // namespace qutim_sdk_0_3::oscar

#endif // OSCARPROXY_H
