/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Ruslan Nigmatullin <euroelessar@yandex.ru>
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

#ifndef WEBKITMESSAGEVIEWSTYLE_H
#define WEBKITMESSAGEVIEWSTYLE_H

#include "adiumwebview_global.h"
#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QVariantMap>
#include <QCoreApplication>

#define SHARE_PATH QString::fromLatin1("/home/elessar/gitgames/labs/adiumwebview-build-desktop/share/")

namespace qutim_sdk_0_3 {
class ChatSession;
class Message;
}

struct IUnit
{
	QString id;
	QString title;
	QString avatar;
};

struct IProtocol
{
	QString shortDescription;
	QString iconPath;
};

struct IAccount : public IUnit
{
	IProtocol protocol;
};

struct IChat
{
	bool isGroupChat;
	bool supportsTopic;
	QString title;
	IAccount account;
	IUnit unit;
	QDateTime dateOpened;
};

struct IContent
{
	enum Type {
		Event,
		Message,
		Topic,
		FileTranfser,
		Status,
		Typing
	};

	IContent() : chat(0), source(0), type(Message),
	    isOutgoing(false), isAutoreply(false), isHistory(false) {}
	IChat *chat;
	IUnit *source;
	QString message;
	QString status;
	QDateTime date;
	QStringList displayClasses;
	Type type;
	bool isOutgoing;
	bool isAutoreply;
	bool isHistory;
	QString senderPrefix;
	QVariantMap userInfo;
	QString statusType;
};

class WebKitMessageViewStylePrivate;

class ADIUMWEBVIEW_EXPORT WebKitMessageViewStyle
{
	Q_DECLARE_PRIVATE(WebKitMessageViewStyle)
public:
	enum NameFormat {
		AIDefaultName = 0,
		AIDisplayName = 1,
		AIDisplayName_ScreenName = 2,
		AIScreenName_DisplayName = 3,
		AIScreenName = 4
	};
	
	enum WebkitBackgroundType {
		BackgroundNormal = 0,
		BackgroundCenter,
		BackgroundTile,
		BackgroundTileCenter,
		BackgroundScale
	};
	
    WebKitMessageViewStyle();
    ~WebKitMessageViewStyle();
	
	void setStylePath(const QString &path);
	QString baseTemplateForChat(qutim_sdk_0_3::ChatSession *session);
	QString templateForContent(const qutim_sdk_0_3::Message &message, bool contentIsSimilar);
	QString scriptForChangingVariant();
	QString scriptForAppendingContent(const qutim_sdk_0_3::Message &message, bool contentIsSimilar, bool willAddMoreContentObjects, bool replaceLastContent);
	QStringList variants();
	QString defaultVariant() const;
	QString activeVariant() const;
	void setActiveVariant(const QString &variant);
	
private:
	struct UnitData
	{
		QString id;
		QString title;
		QString avatar;
	};
	void reloadStyle();
	QString noVariantName() const;
	QString activeVariantPath() const;
	QStringList validSenderColors();
	void loadTemplates();
	void releaseResources();
	UnitData getSourceData(const qutim_sdk_0_3::Message &message);
	QString &fillKeywords(QString &inString, const qutim_sdk_0_3::Message &message, bool contentIsSimilar);
	QString &fillKeywordsForBaseTemplate(QString &inString, qutim_sdk_0_3::ChatSession *session);
	QString pathForResource(const QString &name, const QString &directory = QString());
	QString loadResourceFile(const QString &name, const QString &directory = QString());
	QString stringWithFormat(const QString &str, const QStringList &args);
	
	QScopedPointer<WebKitMessageViewStylePrivate> d_ptr;
};

#endif // WEBKITMESSAGEVIEWSTYLE_H
