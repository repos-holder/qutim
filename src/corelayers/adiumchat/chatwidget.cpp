/****************************************************************************
 *  chatwidget.cpp
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

#include "chatwidget.h"
#include "chatsessionimpl.h"
#include "ui_chatwidget.h"
#include <libqutim/account.h>
#include <libqutim/icon.h>
#include <libqutim/menucontroller.h>
#include <libqutim/debug.h>
#include <QWebFrame>
#include <QTime>
#include <libqutim/qtwin.h>
#include <qshortcut.h>
#include <QWidgetAction>
#include "actions/chatemoticonswidget.h"
#include <libqutim/history.h>

namespace AdiumChat
{
	ChatWidget::ChatWidget(bool removeSessionOnClose): 
	ui(new Ui::AdiumChatForm),m_remove_session_on_close(removeSessionOnClose)
	{
		m_current_index = -1;
		
		ui->setupUi(this);
		centerizeWidget(this);
		//init tabbar
		ui->tabBar->setVisible(false);
		ui->tabBar->setTabsClosable(true);
		ui->tabBar->setMovable(true);
		ui->tabBar->setDocumentMode(true);
		ui->tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
		ui->contactsView->hide();
		//ui->tabBar->setDrawBase(false);
		//init status and menubar
		setAttribute(Qt::WA_DeleteOnClose);
		setAttribute(Qt::WA_MacBrushedMetal);
		
		connect(ui->tabBar,SIGNAL(currentChanged(int)),SLOT(currentIndexChanged(int)));
		connect(ui->tabBar,SIGNAL(tabMoved(int,int)),SLOT(onTabMoved(int,int)));
		connect(ui->tabBar,SIGNAL(tabCloseRequested(int)),SLOT(onCloseRequested(int)));
		connect(ui->tabBar,SIGNAL(customContextMenuRequested(QPoint)),SLOT(onTabContextMenu(QPoint)));
		connect(ui->pushButton,SIGNAL(clicked(bool)),SLOT(onSendButtonClicked()));
		
		ui->chatEdit->installEventFilter(this);
		ui->chatView->installEventFilter(this);
		ui->chatEdit->setFocusPolicy(Qt::StrongFocus);

		//init toolbar
		ui->actionToolBar->setStyleSheet("QToolBar{background:none;border:none}");
		ui->actionToolBar->setIconSize(QSize(16,16));
		
			//for testing
		QMenu *menu = new QMenu(this);
		
		ui->actionToolBar->addAction(new ActionGenerator(Icon("view-history"),
														 QT_TRANSLATE_NOOP("Chat", "View History"),
														 this,
														 SLOT(onShowHistory())));
		
		ui->actionToolBar->addAction(new MenuActionGenerator(Icon("emoticon"),
															 QT_TRANSLATE_NOOP("Chat", "Emoticons"),
															 menu));
		QWidgetAction *emoticons_widget_act = new QWidgetAction(this);
		ChatEmoticonsWidget *emoticons_widget = new ChatEmoticonsWidget(this);
		emoticons_widget->loadTheme();
		emoticons_widget_act->setDefaultWidget(emoticons_widget);
		menu->addAction(emoticons_widget_act);
		connect(emoticons_widget,SIGNAL(insertSmile(QString)),ui->chatEdit,SLOT(appendPlainText(QString)));

		//
		//load settings
		m_html_message = Config("appearance/adiumChat").group("behavior/widget").value<bool>("htmlMessage",false);
		ConfigGroup adium_chat = Config("appearance/adiumChat").group("behavior/widget");
		m_chat_flags = static_cast<ChatFlag> (adium_chat.value<int>("widgetFlags",ChatStateIconsOnTabs | SendTypingNotification | ShowUnreadMessages));
		
		if (m_chat_flags & SendTypingNotification) {
			connect(ui->chatEdit,SIGNAL(textChanged()),SLOT(onTextChanged()));
			m_chatstate = ChatStateActive;
			m_timeout = 5000;
		}
		//init aero integration for win		
		if (m_chat_flags & AeroThemeIntegration) {
			if (QtWin::isCompositionEnabled()) {
				QtWin::extendFrameIntoClientArea(this);
				setContentsMargins(0, 0, 0, 0);
			}
		}
		ui->pushButton->setShortcut(QKeySequence(tr("Ctrl+Return","Send message")));

		new QShortcut(QKeySequence(QKeySequence::Close),
					  this,
					  SLOT(closeCurrentTab())
					  );
		
	}

	ChatWidget::~ChatWidget()
	{
		clear();
	}	

	void ChatWidget::addSession(ChatSessionImpl* session)
	{
		if(m_sessions.contains(session))
			return;
		m_sessions.append(session);
		session->installEventFilter(this);
		connect(session, SIGNAL(destroyed(QObject*)), SLOT(onSessionDestroyed(QObject*)));
		connect(session, SIGNAL(unreadChanged(qutim_sdk_0_3::MessageList)),
				SLOT(onUnreadChanged(qutim_sdk_0_3::MessageList)));

		QIcon icon;
		if (m_chat_flags & ChatStateIconsOnTabs) {
			ChatState state = static_cast<ChatState>(session->property("currentChatState").toInt());
			icon = iconForState(state);
		}
		ui->tabBar->addTab(icon,session->getUnit()->title());
		if (ui->tabBar->count() >1)
			ui->tabBar->setVisible(true);
	}

	void ChatWidget::addSession(const ChatSessionList &sessions)
	{
		for (int i = 0; i!=sessions.count(); i++)
			addSession(sessions.at(i));
	}

	void ChatWidget::currentIndexChanged(int index)
	{
		if (index == -1)
			return;
		int previous_index = m_current_index;
		ChatSessionImpl *session = m_sessions.at(index);
		if ((previous_index != -1) && (previous_index != index)) {
			m_sessions.at(previous_index)->setActive(false);
			session->activate();
		}
		m_current_index = index;
		ui->contactsView->setModel(session->getModel());
		ui->contactsView->setVisible(session->getModel()->rowCount(QModelIndex()) > 0);
		if (ui->chatView->page() != session->getPage()) {
			ui->chatView->page()->setView(0);
			ui->chatView->setPage(session->getPage());
			session->getPage()->setView(ui->chatView);
		}
		setWindowTitle(tr("Chat with %1").arg(session->getUnit()->title()));
		
 		if ((m_chat_flags & SendTypingNotification) && (m_chatstate & ChatStateComposing)) {
			killTimer(m_self_chatstate_timer);
 			m_chatstate = ui->chatEdit->document()->isEmpty() ? ChatStateActive : ChatStatePaused;
			m_sessions.at(previous_index)->setChatState(m_chatstate);
 		}

	}

	void ChatWidget::clear()
	{
		int count = m_sessions.count();
		for (int i = 0;i!=count;i++)
			ui->tabBar->removeTab(i);
		if (m_remove_session_on_close)
			qDeleteAll(m_sessions);
		m_sessions.clear();		
	}

	void ChatWidget::removeSession(ChatSessionImpl* session)
	{
		int index = m_sessions.indexOf(session);
		if (index == -1)
			return;
		ui->tabBar->removeTab(index);
		m_sessions.removeAt(index);
		session->disconnect(this);

		currentIndexChanged(ui->tabBar->currentIndex());

		if (ui->tabBar->count() == 1)
			ui->tabBar->setVisible(false);
		if (session && m_remove_session_on_close) {			
			session->deleteLater();
			debug () << "session removed" << index;
		}
	}

	void ChatWidget::onSessionDestroyed(QObject* object)
	{
		ChatSessionImpl *sess = static_cast<ChatSessionImpl *>(object);
		if (!sess)
			return;
		removeSession(sess);
	}

	ChatSessionList ChatWidget::getSessionList() const
	{
		return m_sessions;
	}

	void ChatWidget::onCloseRequested(int index)
	{
		removeSession(m_sessions.at(index));
	}

	void ChatWidget::onTabMoved(int from, int to)
	{
		m_sessions.move(from,to);
		debug() << "moved session" << from << to;
	}

	void ChatWidget::activate(AdiumChat::ChatSessionImpl* session)
	{
		activateWindow();
		raise();
		//TODO customize support
		int index = m_sessions.indexOf(session);
		debug() << "active index" << index;
		if (ui->tabBar->currentIndex() != index)
			ui->tabBar->setCurrentIndex(index);

		if ((m_chat_flags & ShowUnreadMessages) && !session->unread().isEmpty()) {
			session->markRead();
		}		
	}

	void ChatWidget::onUnreadChanged(const qutim_sdk_0_3::MessageList &unread)
	{
		ChatSessionImpl *session = static_cast<ChatSessionImpl*>(sender());
		int index = m_sessions.indexOf(session);
		if (index < 0)
			return;
		if (unread.isEmpty()) {
 			ChatState state = static_cast<ChatState>(session->property("currentChatState").toInt());
 			QIcon icon = iconForState(state);
 			ui->tabBar->setTabIcon(index, icon);
		} else if (m_chat_flags & ShowUnreadMessages) {
			ui->tabBar->setTabIcon(index, Icon("mail-unread-new"));
		}
	}

	bool ChatWidget::eventFilter(QObject *obj, QEvent *event)
	{
		if (obj->metaObject() == &ChatSessionImpl::staticMetaObject) {
			if (event->type() == ChatStateEvent::eventType()) {
				ChatStateEvent *chatEvent = static_cast<ChatStateEvent *>(event);
				chatStateChanged(chatEvent->chatState(),qobject_cast<ChatSessionImpl *>(obj));				
			}
		} else {
			if (event->type() == QEvent::KeyPress) {
				QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
				QString key = QString::number(keyEvent->key(), 16);
				QString modifiers = QString::number(keyEvent->modifiers(), 16);
// 				if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
// 					if ((keyEvent->modifiers() & Qt::ControlModifier)) {
// 						onSendButtonClicked();
// 						return true;
// 					}
// 				}
				if (keyEvent->matches(QKeySequence::Copy) )
				{
					if (QWebView *view = qobject_cast<QWebView*>(obj))
						view->triggerPageAction(QWebPage::Copy);
					return true;
				}
			}
		}
		return QObject::eventFilter(obj, event);
	}

	bool ChatWidget::contains(ChatSessionImpl* session)
	{
		return m_sessions.contains(session);
	}

	void ChatWidget::onSendButtonClicked()
	{
		if (ui->chatEdit->toPlainText().trimmed().isEmpty() || ui->tabBar->currentIndex() < 0)
			return;
		ChatSessionImpl *session = m_sessions.at(ui->tabBar->currentIndex());
		ChatUnit *unit = session->getUnit();
		Message message(ui->chatEdit->toPlainText());
		if (m_html_message)
			message.setProperty("html",Qt::escape(message.text()));
		message.setIncoming(false);
		message.setChatUnit(unit);
		message.setTime(QDateTime::currentDateTime());
		session->appendMessage(message);
		session->getUnit()->sendMessage(message);
		ui->chatEdit->clear();

		killTimer(m_self_chatstate_timer);
		m_chatstate = ChatStateActive;
	}

	void ChatWidget::chatStateChanged(ChatState state, ChatSessionImpl *session)
	{
		int index = m_sessions.indexOf(session);
		if (index == -1)
			return;
		
		if (m_chat_flags & ChatStateIconsOnTabs) {
			if (!session->unread().count())
				ui->tabBar->setTabIcon(index,iconForState(state));
		}
		
		session->setProperty("currentChatState",static_cast<int>(state));
	}

	QTextDocument *ChatWidget::getInputField()
	{
		return ui->chatEdit->document();
	}
	
	void ChatWidget::onTextChanged()
	{
		killTimer(m_self_chatstate_timer);
		m_self_chatstate_timer = startTimer(m_timeout);
		if ((m_chatstate != ChatStateComposing) && (!ui->chatEdit->toPlainText().isEmpty())) {
			m_chatstate = ChatStateComposing;
			 m_sessions.at(ui->tabBar->currentIndex())->setChatState(m_chatstate);
		}
	}

	void ChatWidget::timerEvent(QTimerEvent* e)
	{
		m_chatstate = ui->chatEdit->document()->isEmpty() ? ChatStateActive : ChatStatePaused;
		m_sessions.at(ui->tabBar->currentIndex())->setChatState(m_chatstate);
		killTimer(m_self_chatstate_timer);
		QObject::timerEvent(e);
	}

	bool ChatWidget::event(QEvent *event)
	{
		if (event->type() == QEvent::WindowActivate
			|| event->type() == QEvent::WindowDeactivate) {
			bool active = event->type() == QEvent::WindowActivate;
			if (ui->tabBar->currentIndex() == -1)
				return false;
			m_sessions.at(ui->tabBar->currentIndex())->setActive(active);
		}
		return QMainWindow::event(event);
	}

	void ChatWidget::onTabContextMenu(const QPoint &pos)
	{
		int index = ui->tabBar->tabAt(pos);
		if (index != -1) {
			if (MenuController *session = m_sessions.value(index)->getUnit()) {
				session->showMenu(ui->tabBar->mapToGlobal(pos));
			}
		}
	}
	
	QIcon ChatWidget::iconForState(ChatState state)
	{
		QString icon_name;
		switch (state) {
			//FIXME icon names
			case ChatStateActive:
				icon_name = "im-user";
				break;
			case ChatStateInActive:
				icon_name = "im-user-away";
				break;
			case ChatStateGone:
				icon_name =  "im-user-offline";
				break;
			case ChatStateComposing:
				icon_name = "im-status-message-edit";
				break;
			case ChatStatePaused:
				icon_name = "mail-unread";
				break;
			default:
				break;
		}
		return Icon(icon_name);
	}

	void ChatWidget::closeCurrentTab()
	{
		if (ui->tabBar->count() > 1)
			ui->tabBar->removeTab(ui->tabBar->currentIndex());
		else
			close();
	}
	
		
	void ChatWidget::onShowHistory()
	{
		ChatUnit *unit = m_sessions.at(m_current_index)->getUnit();
		History::instance()->showHistory(unit);
	}	

}

