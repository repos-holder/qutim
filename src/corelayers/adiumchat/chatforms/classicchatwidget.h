/****************************************************************************
 *  classicchatwidget.h
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

#ifndef CLASSICCHATWIDGET_H
#define CLASSICCHATWIDGET_H

#include <QWidget>
#include "../chatlayerimpl.h"
#include "abstractchatwidget.h"
#include <QMainWindow>
#include <QModelIndex>
#include <QTimer>

namespace Ui
{
	class ClassicChatForm;
}
class QPlainTextEdit;
namespace Core
{
	namespace AdiumChat
	{
		class ChatSessionImpl;
		class ConfTabCompletion;
		class ClassicChatWidget : public AbstractChatWidget
		{
			Q_OBJECT
		public:
			ClassicChatWidget(const QString &key, bool removeSessionOnClose);
			void clear();//remove all sessions
			ChatSessionList getSessionList() const;
			virtual ~ClassicChatWidget();
			QPlainTextEdit *getInputField();
			QTabBar *getTabBar();
			QListView *getContactsView();
			ChatSessionImpl *currentSession();
		public slots:
			void addSession(ChatSessionImpl *session);
			void addSession(const ChatSessionList &sessions);
			void removeSession(ChatSessionImpl *session);
			void activate(AdiumChat::ChatSessionImpl* session);
			void onUnreadChanged(const qutim_sdk_0_3::MessageList &unread);
		protected:
			virtual bool event(QEvent *event);
		private slots:
			void currentIndexChanged (int index);
			void onBuddiesChanged();
			void onCloseRequested(int index);
			void onTabMoved(int from,int to);
			void onSessionDestroyed(QObject* object);
			void onTextChanged();
			void onTabContextMenu(const QPoint &pos);
			void closeCurrentTab();
			void onSessionListActionTriggered();
			void onShowHistory(); //TODO Move to startup module
			void showNextSession();
			void showPreviousSession();
			void onDoubleClicked(const QModelIndex &index);
			void onUnitTitleChanged(const QString &title);
			void onChatStateTimeout();
		private:
			void chatStateChanged(ChatState state, ChatSessionImpl* session);
			int m_current_index;
			Ui::ClassicChatForm *ui;
			QPointer<QTextDocument> m_originalDoc;
		};
	}
}
#endif // CLASSICCHATWIDGET_H
