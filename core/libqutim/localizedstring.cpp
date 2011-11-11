/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright (C) 2011 Ruslan Nigmatullin <euroelessar@yandex.ru>
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

#include "localizedstring.h"
#include <QtCore/QCoreApplication>
#include <QSharedData>

namespace qutim_sdk_0_3
{
	// It may be usefull for localizable XMPP string
	// i.e. Message with body in several languages
	class LocalizationEngine
	{
	public:
		virtual QString text() const = 0;
		virtual ~LocalizationEngine() {}
	};
	// Args are usefull sometimes, i.e. at strings like
	// LocalizedString("", "Hello, %1").arg("Tom"), where
	// there may be several unlocalizable arguments
	// which should be inserted after translation
	class LocalizedStringData : public QSharedData
	{
	public:
		LocalizationEngine *engine;
		QByteArray ctx;
		QByteArray str;
		QList<QString> args;
	};
	
	QString LocalizedString::toString() const
	{
		if (m_ctx.isEmpty())
			return QString::fromUtf8(m_str);
		return QCoreApplication::translate(m_ctx.constData(), m_str.constData(), 0, QCoreApplication::UnicodeUTF8);
	}

	QDataStream &operator<<(QDataStream &out, const LocalizedString &str)
	{
		out << 1;
		out << str.m_ctx;
		out << str.m_str;
		return out;
	}

	QDataStream &operator>>(QDataStream &in, LocalizedString &str)
	{
		int ver = 1;
		in >> ver;
		Q_ASSERT(ver == 1);
		in >> str.m_ctx;
		in >> str.m_str;
		return in;
	}

	struct StaticConstructor
	{
		StaticConstructor()
		{
			qRegisterMetaTypeStreamOperators<LocalizedString>("qutim_sdk_0_3::LocalizedString");
		}
	} staticConstructor;
}

