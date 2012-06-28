/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Aleksey Sidorov <gorthauer87@yandex.ru>
**
*****************************************************************************
**
** $QUTIM_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $QUTIM_END_LICENSE$
**
****************************************************************************/

#ifndef VCONTACT_H
#define VCONTACT_H
#include <qutim/contact.h>
#include <QPointer>

namespace vk {
class Buddy;
}

class VAccount;
class VContactPrivate;
class VContact : public qutim_sdk_0_3::Contact
{
	Q_OBJECT
public:
	VContact(vk::Buddy *contact, VAccount* account);

	virtual QString id() const;
	virtual bool isInList() const;
	virtual bool sendMessage(const qutim_sdk_0_3::Message& message);
	virtual void setTags(const QStringList& tags);
	virtual void setInList(bool inList);
	virtual qutim_sdk_0_3::Status status() const;
	virtual ~VContact();
	virtual QStringList tags() const;
	virtual QString name() const;
	virtual void setName(const QString& name);
	virtual QString avatar() const;
	QString activity() const;
public slots:
	void setTyping(bool set = false);
private:
	virtual bool event(QEvent *ev);
	vk::Buddy *m_buddy;
	QPointer<QTimer> m_typingTimer;
};

Q_DECLARE_METATYPE(VContact*)

#endif // VCONTACT_H

