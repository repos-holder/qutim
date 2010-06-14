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

#include "classicchatwidget.h"
#include "../chatsessionimpl.h"
#include <libqutim/account.h>
#include <libqutim/icon.h>
#include <libqutim/menucontroller.h>
#include <libqutim/debug.h>
#include <QWebFrame>
#include <QTime>
#include <libqutim/qtwin.h>
#include <qshortcut.h>
#include <QWidgetAction>
#include "../actions/chatemoticonswidget.h"
#include <libqutim/history.h>
#include <libqutim/conference.h>
#include "../chatsessionitemdelegate.h"
#include "ui_classicchatwidget.h"
#include <libqutim/tooltip.h>

namespace Core
{
	namespace AdiumChat
	{
		ClassicChatWidget::ClassicChatWidget(const QString &widgetKey, bool removeSessionOnClose):
				AbstractChatWidget(widgetKey, removeSessionOnClose),
				ui(new Ui::ClassicChatForm)
		{
			m_current_index = -1;

			ui->setupUi(this);
			
			m_originalDoc = ui->chatEdit->document();

			//init tabbar
			ui->tabBar->setTabsClosable(true);
			ui->tabBar->setMovable(true);
			ui->tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
			ui->tabButton->setIcon(Icon("view-list-text"));
			ui->contactsView->hide();
			//init status and menubar

			connect(ui->tabBar,SIGNAL(currentChanged(int)),SLOT(currentIndexChanged(int)));
			connect(ui->tabBar,SIGNAL(tabMoved(int,int)),SLOT(onTabMoved(int,int)));
			connect(ui->tabBar,SIGNAL(tabCloseRequested(int)),SLOT(onCloseRequested(int)));
			connect(ui->tabBar,SIGNAL(customContextMenuRequested(QPoint)),SLOT(onTabContextMenu(QPoint)));
			connect(ui->sendButton,SIGNAL(clicked(bool)),SLOT(onSendButtonClicked()));
			ui->contactsView->setItemDelegate(new ChatSessionItemDelegate(this));

			ui->contactsView->installEventFilter(this);
			ui->chatEdit->installEventFilter(this);
			ui->chatView->installEventFilter(this);
			ui->tabBar->installEventFilter(this);
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

			ui->actionToolBar->addAction(new MenuActionGenerator(Icon("face-smile"),
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
			onLoad();
		}

		ClassicChatWidget::~ClassicChatWidget()
		{
			delete ui;
		}

		void ClassicChatWidget::addSession(ChatSessionImpl* session)
		{
			if(m_sessions.contains(session))
				return;
			m_sessions.append(session);
			session->installEventFilter(this);
			connect(session, SIGNAL(destroyed(QObject*)), SLOT(onSessionDestroyed(QObject*)));
			connect(session, SIGNAL(unreadChanged(qutim_sdk_0_3::MessageList)),
					SLOT(onUnreadChanged(qutim_sdk_0_3::MessageList)));

			ChatUnit *u = session->getUnit();
			connect(u,SIGNAL(titleChanged(QString)),SLOT(onUnitTitleChanged(QString)));

			QIcon icon;
			if (m_chatFlags & ChatStateIconsOnTabs) {
				ChatState state = static_cast<ChatState>(session->property("currentChatState").toInt());
				icon =  ChatLayerImpl::iconForState(state);
			}

			ui->tabBar->addTab(icon,u->title());

			QAction *act = new QAction(icon,u->title(),this);
			connect(act,SIGNAL(triggered()),this,SLOT(onSessionListActionTriggered()));
			ui->tabButton->addAction(act);
		}

		void ClassicChatWidget::addSession(const ChatSessionList &sessions)
		{
			for (int i = 0; i!=sessions.count(); i++)
				addSession(sessions.at(i));
		}

		void ClassicChatWidget::currentIndexChanged(int index)
		{
			if (index == -1) {
				ui->chatEdit->setDocument(m_originalDoc);
				return;
			}
			int previous_index = m_current_index;
			m_current_index = index;
			ChatSessionImpl *session = m_sessions.at(index);

			if ((previous_index != -1) && (previous_index != index)) {
				m_sessions.at(previous_index)->setActive(false);
				session->activate();
			}
			ui->contactsView->setModel(session->getModel());

			ChatUnit *u = session->getUnit();
			QIcon icon = Icon("view-choose");
			QString title = tr("Chat with %1").arg(u->title());

			if (Conference *c = qobject_cast<Conference *>(u)) {
				icon = Icon("meeting-attending"); //TODO
				title = tr("Conference %1 (%2)").arg(c->title(),c->id());
				ui->contactsView->setVisible(true);
			} else {
				ui->contactsView->setVisible(session->getModel()->rowCount(QModelIndex()) > 0);
				if (Buddy *b = qobject_cast<Buddy*>(u))
					icon = b->avatar().isEmpty() ? Icon("view-choose") : QIcon(b->avatar());
			}

			setWindowTitle(title);
			setWindowIcon(icon);

			if (ui->chatView->page() != session->getPage()) {
				ui->chatView->page()->setView(0);
				ui->chatView->setPage(session->getPage());
				session->getPage()->setView(ui->chatView);
			}

			if ((m_chatFlags & SendTypingNotification) && (m_chatstate & ChatStateComposing)) {
				m_chatstateTimer.stop();
				m_chatstate = ui->chatEdit->document()->isEmpty() ? ChatStateActive : ChatStatePaused;
				m_sessions.at(previous_index)->setChatState(m_chatstate);
			}

			ui->sendButton->setMenu(session->menu());
			ui->chatEdit->setDocument(session->getInputField());
		}

		void ClassicChatWidget::clear()
		{
			int count = m_sessions.count();
			for (int i = 0;i!=count;i++)
				ui->tabBar->removeTab(i);
			if (m_removeSessionOnClose)
				qDeleteAll(m_sessions);
			m_sessions.clear();
		}

		void ClassicChatWidget::removeSession(ChatSessionImpl* session)
		{
			int index = m_sessions.indexOf(session);
			if (index == -1)
				return;
			ui->tabBar->removeTab(index);
			m_sessions.removeAt(index);
			ui->tabButton->removeAction(ui->tabButton->actions().at(index));
			session->disconnect(this);

			currentIndexChanged(ui->tabBar->currentIndex());

			if (session && m_removeSessionOnClose) {
				session->deleteLater();
			}

			if (m_sessions.isEmpty())
				close();
		}

		void ClassicChatWidget::onSessionDestroyed(QObject* object)
		{
			ChatSessionImpl *sess = reinterpret_cast<ChatSessionImpl *>(object);
			removeSession(sess);
		}

		ChatSessionList ClassicChatWidget::getSessionList() const
		{
			return m_sessions;
		}

		void ClassicChatWidget::onCloseRequested(int index)
		{
			removeSession(m_sessions.at(index));
		}

		void ClassicChatWidget::onTabMoved(int from, int to)
		{
			m_sessions.move(from,to);

			//FIXME remove hack

			QList <QAction *> actions = ui->tabButton->actions();
			actions.move(from,to);
			foreach (QAction *a,ui->tabButton->actions())
				ui->tabButton->removeAction(a);
			ui->tabButton->addActions(actions);

			debug() << "moved session" << from << to;
		}

		void ClassicChatWidget::activate(AdiumChat::ChatSessionImpl* session)
		{
			raise();
			activateWindow();

			//TODO customize support
			int index = m_sessions.indexOf(session);
			debug() << "active index" << index;
			if (ui->tabBar->currentIndex() != index)
				ui->tabBar->setCurrentIndex(index);

			if ((m_chatFlags & ShowUnreadMessages) && !session->unread().isEmpty()) {
				session->markRead();
			}

			ui->chatEdit->setFocus(Qt::ActiveWindowFocusReason);
		}

		void ClassicChatWidget::onUnreadChanged(const qutim_sdk_0_3::MessageList &unread)
		{
			ChatSessionImpl *session = static_cast<ChatSessionImpl*>(sender());
			int index = m_sessions.indexOf(session);
			if (index < 0)
				return;
			if (unread.isEmpty()) {
				ChatState state = static_cast<ChatState>(session->property("currentChatState").toInt());
				QIcon icon =  ChatLayerImpl::iconForState(state);
				ui->tabBar->setTabIcon(index, icon);
			} else if (m_chatFlags & ShowUnreadMessages) {
				ui->tabBar->setTabIcon(index, Icon("mail-unread-new"));
			}
		}

		void ClassicChatWidget::chatStateChanged(ChatState state, ChatSessionImpl *session)
		{
			int index = m_sessions.indexOf(session);
			if (index == -1)
				return;

			if (m_chatFlags & ChatStateIconsOnTabs) {
				if (!session->unread().count()) {
					QIcon icon =  ChatLayerImpl::iconForState(state);
					ui->tabBar->setTabIcon(index,icon);
					ui->tabButton->actions().at(index)->setIcon(icon);
				}
			}

			session->setProperty("currentChatState",static_cast<int>(state));
		}

		QPlainTextEdit *ClassicChatWidget::getInputField()
		{
			return ui->chatEdit;
		}
		
		QTabBar *ClassicChatWidget::getTabBar()
		{
			return ui->tabBar;
		}
		
		QListView *ClassicChatWidget::getContactsView()
		{
			return ui->contactsView;
		}

		ChatSessionImpl *ClassicChatWidget::currentSession()
		{
			return m_sessions.at(m_current_index);
		}

		void ClassicChatWidget::onTextChanged()
		{
			m_chatstateTimer.stop();
			m_chatstateTimer.start();
			if ((m_chatstate != ChatStateComposing) && (!ui->chatEdit->toPlainText().isEmpty())) {
				m_chatstate = ChatStateComposing;
				m_sessions.at(ui->tabBar->currentIndex())->setChatState(m_chatstate);
			}
		}

		bool ClassicChatWidget::event(QEvent *event)
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

		void ClassicChatWidget::onTabContextMenu(const QPoint &pos)
		{
			int index = ui->tabBar->tabAt(pos);
			if (index != -1) {
				if (MenuController *session = m_sessions.value(index)->getUnit()) {
					session->showMenu(ui->tabBar->mapToGlobal(pos));
				}
			}
		}

		void ClassicChatWidget::onShowHistory()
		{
			ChatUnit *unit = m_sessions.at(m_current_index)->getUnit();
			History::instance()->showHistory(unit);
		}

		void ClassicChatWidget::onSessionListActionTriggered()
		{
			QAction *act = qobject_cast<QAction *>(sender());
			Q_ASSERT(act);

			ui->tabBar->setCurrentIndex(ui->tabButton->actions().indexOf(act));
		}

		void ClassicChatWidget::onUnitTitleChanged(const QString &title)
		{
			ChatUnit *u = qobject_cast<ChatUnit *>(sender());
			if (!u)
				return;
			ChatSessionImpl *s = static_cast<ChatSessionImpl *>(ChatLayerImpl::get(u,false));
			if (!s)
				return;
			ui->tabBar->setTabText(m_sessions.indexOf(s),title);
		}
	}
}
