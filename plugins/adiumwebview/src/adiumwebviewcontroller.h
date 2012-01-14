/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Sidorov Aleksey <sauron@citadelspb.com>
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

#ifndef QUICKCHATVIEWCONTROLLER_H
#define QUICKCHATVIEWCONTROLLER_H

#include <QWebPage>
#include <qutim/adiumchat/chatviewfactory.h>
#include <qutim/adiumchat/chatsessionimpl.h>
#include "../lib/webkitmessageviewstyle.h"
#include <QVariant>
#include <QPointer>

class QDeclarativeEngine;
class QDeclarativeItem;
class QDeclarativeContext;

namespace Adium {

class WebViewController : public QWebPage, public Core::AdiumChat::ChatViewController
{
    Q_OBJECT
	Q_INTERFACES(Core::AdiumChat::ChatViewController)
public:
	WebViewController(bool isPreview = false);
	virtual ~WebViewController();
	
	virtual void setChatSession(Core::AdiumChat::ChatSessionImpl *session);
	virtual Core::AdiumChat::ChatSessionImpl *getSession() const;
	virtual void appendMessage(const qutim_sdk_0_3::Message &msg);
	virtual void clearChat();
	virtual QString quote();
	WebKitMessageViewStyle *style();
	
public slots:
	void evaluateJavaScript(const QString &script);
	
protected:
	void loadSettings();
	void loadHistory();
	
private slots:
	void onLoadFinished();
	void onTopicChanged(const QString &topic);
	void updateTopic();
	void onContentsChanged();
	
private:
	QWeakPointer<Core::AdiumChat::ChatSessionImpl> m_session;
	WebKitMessageViewStyle m_style;
	bool m_isLoading;
	bool m_isPreview;
	QStringList m_pendingScripts;
	qutim_sdk_0_3::Message m_last;
	qutim_sdk_0_3::Message m_topic;
};

} // namespace Adium

#endif // QUICKCHATVIEWCONTROLLER_H

