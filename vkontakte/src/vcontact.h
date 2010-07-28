/****************************************************************************
 *  vcontact.h
 *
 *  Copyright (c) 2010 by Aleksey Sidorov <sauron@citadelspb.com>
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

#ifndef VCONTACT_H
#define VCONTACT_H
#include "vkontakte_global.h"
#include <qutim/contact.h>

class VAccount;
struct VContactPrivate;
class LIBVKONTAKTE_EXPORT VContact : public Contact
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(VContact)
public:
	VContact(const QString& id, VAccount* account);
	virtual QString id() const;
	virtual bool isInList() const;
	virtual bool sendMessage(const Message& message);
	virtual void setTags(const QStringList& tags);
	virtual void setInList(bool inList);
	void setContactTags(const QStringList& tags);
	void setContactInList(bool inList);
	void setStatus(bool online);
	void setActivity(const QString &activity);
	virtual Status status() const;
	virtual ~VContact();
	virtual QStringList tags() const;
	virtual QString name() const;
	void setContactName(const QString& name);
	virtual void setName(const QString& name);
	void setAvatar(const QString &avatar);
	virtual QString avatar() const;
private:
	virtual bool event(QEvent *ev);
	QScopedPointer<VContactPrivate> d_ptr;
};

Q_DECLARE_METATYPE(VContact*)

#endif // VCONTACT_H
