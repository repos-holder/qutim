/****************************************************************************
 *  contact.h
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
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

#ifndef CONTACT_H
#define CONTACT_H

#include "chatunit.h"
#include "status.h"
#include <QSet>
#include <QIcon>
#include <QMetaType>

namespace qutim_sdk_0_3
{
	class Account;
	class MetaContact;
	class Message;
	typedef QPair<LocalizedString, QVariant> InfoField;
	typedef QList<InfoField> InfoFieldList;

	/**
	* @brief Contact is base class for all contactlist members
	*/
	class LIBQUTIM_EXPORT Contact : public ChatUnit
	{
		Q_OBJECT
		Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
		Q_PROPERTY(QString avatar READ avatar NOTIFY avatarChanged)
		Q_PROPERTY(QSet<QString> tags READ tags WRITE setTags NOTIFY tagsChanged)
		Q_PROPERTY(Status status READ status NOTIFY statusChanged)
		Q_PROPERTY(bool inList READ isInList WRITE setInList NOTIFY inListChanged)
	public:
		/**
		* @brief default Contact's contructor
		*
		* @param account Pointer to chatunit's account
		*/
		Contact(Account *account);
		/**
		* @brief Contact's destructor
		*/
		virtual ~Contact();
		/*!
		  Path to buddy image
		*/
		virtual QString avatar() const;
		/**
		* @brief Returns contact's representable name
		*
		* @return Contact's name
		*/
		virtual QString title() const;
		/**
		* @brief Returns set of tags that mark the contact
		*
		* @return QSet<QString> tags
		*/
		virtual QSet<QString> tags() const;
		virtual QString name() const;
		virtual Status status() const;
		/**
		* @brief send message to contact
		*
		* @param message Message, which to be sent to the recipient
		*/
		virtual void sendMessage(const Message &message) = 0;
		virtual void setName(const QString &name) = 0;
		/**
		* @brief set a set of tags
		*
		* @param tags
		*/
		virtual void setTags(const QSet<QString> &tags) = 0;
		/**
		* @brief Returns membership contact to contactlist
		*
		* @return isInList
		*/
		virtual bool isInList() const = 0;
		/**
		* @brief set a membership contact to contactlist
		*
		* @param inList
		*/
		virtual void setInList(bool inList) = 0;
		/**
		* @brief add contact to contactlist, see also setInList(bool inList)
		*/
		void addToList() { setInList(true); }
		/**
		* @brief remove contact from contactlist, see also setInList(bool inList)
		*/
		void removeFromList() { setInList(false); }
		/*!
		  Returnes list of protocol-specific info pairs for tooltip, i.e.:
		  \code
InfoFieldList list;
list.append(Field(QT_TRANSLATE_NOOP("Jabber", "Mood"),     tr("Angry")));
list.append(Field(QT_TRANSLATE_NOOP("Jabber", "Activity"), tr("Playing")));
list.append(Field(QT_TRANSLATE_NOOP("Protocol", "Idle"),   m_idle - QDateTime::currentDateTime().toTime_t())));
return list;
		  \endcode
		 */
		virtual InfoFieldList info(bool extra = false);
	signals:
		void avatarChanged(const QString &path);
		void statusChanged(const qutim_sdk_0_3::Status &status);
		void nameChanged(const QString &name);
		void tagsChanged(const QSet<QString> &tags);
		void inListChanged(bool inList);
	};
}

Q_DECLARE_METATYPE(qutim_sdk_0_3::Contact*)

#endif // CONTACT_H
