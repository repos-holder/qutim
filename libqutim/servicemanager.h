/****************************************************************************
 *  servicemanager.h
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *  Copyright (c) 2011 by Sidorov Aleksey <sauron@citadelspb.com>
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

#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include "libqutim_global.h"
#include "extensioninfo.h"

namespace qutim_sdk_0_3
{

class ServiceManagerPrivate;

class LIBQUTIM_EXPORT ServiceManager : public QObject
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(ServiceManager)
public:
	static bool isInited();
	static QObject *getByName(const QByteArray &name);
	template<typename T>
	static inline T getByName(const QByteArray &name)
	{ return qobject_cast<T>(ServiceManager::getByName(name)); }
	static QList<QByteArray> names();
	static ServiceManager *instance();
	/*! List info about all implementations of service with \param name
	  */
	static ExtensionInfoList listImplementations(const QByteArray &name);
	/*! Set an implementation with \param info for service with \param name.
	  * When \param info is empty, service is turned off.
	  * Returns true, if the service has been successfully switched. Otherwise,
	  * return false.
	  */
	static bool setImplementation(const QByteArray &name, const ExtensionInfo &info);

signals:
	/*! Notify a change of service.
	  */
	void serviceChanged(const QByteArray &name, QObject *newObject, QObject *oldObject);
	void serviceChanged(QObject *newObject, QObject *oldObject);

private:
	ServiceManager();
	~ServiceManager();

	QScopedPointer<ServiceManagerPrivate> d_ptr;
};

} // namespace qutim_sdk_0_3

#endif // SERVICEMANAGER_H
