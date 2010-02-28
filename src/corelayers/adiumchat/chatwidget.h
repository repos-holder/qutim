/****************************************************************************
 *  chatwidget.h
 *
 *  Copyright (c) 2010 by Sidorov Aleksey <sauron@citadelspb.com>
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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include "chatlayerimpl.h"
#include <QMainWindow>

namespace qutim_sdk_0_3 {
	class ActionToolBar;
}

namespace Ui
{
	class AdiumChatForm;
}
namespace AdiumChat
{
	enum ChatFlag
	{
		AeroThemeIntegration	=	0x01,
		ChatStateIconsOnTabs	=	0x02,
		AvatarsOnTabs			=	0x04,
		SendTypingNotification	=	0x08,
		ShowUnreadMessages		=	0x10
	};
	Q_DECLARE_FLAGS(ChatFlags, ChatFlag);
	class ChatSessionImpl;
	class ChatWidget : public QMainWindow
	{
		Q_OBJECT
	public:
		ChatWidget(bool removeSessionOnClose);
		void clear();//remove all sessions
		ChatSessionList getSessionList() const;
		virtual ~ChatWidget();
		bool contains (ChatSessionImpl *session);
		QTextDocument *getInputField();
	public slots:
		void addSession(ChatSessionImpl *session);
		void addSession(const ChatSessionList &sessions);
		void removeSession(ChatSessionImpl *session);
		void activate(ChatSessionImpl *sessionz);
	protected:
		bool eventFilter(QObject *obj, QEvent *event);
		virtual void timerEvent(QTimerEvent* );
	private:
		QIcon iconForState(ChatState state);
		ChatSessionList m_sessions;
		int m_current_index;
		Ui::AdiumChatForm *ui;
		ChatFlags m_chat_flags;
		bool m_html_message;
		bool m_remove_session_on_close;
		ChatState m_chatstate;
		int m_timerid;
		int m_timeout;
	private slots:
		void currentIndexChanged (int index);
		void onCloseRequested(int index);
		void onTabMoved(int from,int to);
		void onSessionDestroyed(QObject* object);
		void onSendButtonClicked();
		void chatStateChanged(ChatState state);
		void onTextChanged(); //TODO separate from chatlayer
		void onMessageReceived(const Message &message);
		void onMessageSended(const Message &message);
		void onTabContextMenu(const QPoint &pos);
	};

}
#endif // CHATWIDGET_H
