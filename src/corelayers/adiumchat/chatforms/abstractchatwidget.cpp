/****************************************************************************
 *
 *  This file is part of qutIM
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *
 ***************************************************************************
 *                                                                         *
 *   This file is part of free software; you can redistribute it and/or    *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************
 ****************************************************************************/

#include "abstractchatwidget.h"
#include "../chatsessionimpl.h"
#include "libqutim/configbase.h"
#include <QSplitter>
#include <QPlainTextEdit>
#include <QTabBar>
#include <QDateTime>
#include <QListView>
#include <QWebView>
#include "libqutim/tooltip.h"
#include <libqutim/shortcut.h>
#include <libqutim/conference.h>

#ifdef Q_WS_X11
# include <QX11Info>
# include <X11/Xutil.h>
# include <X11/Xlib.h>
# include <X11/Xatom.h>
# ifdef KeyPress
#  undef KeyPress
# endif
# define MESSAGE_SOURCE_OLD            0
# define MESSAGE_SOURCE_APPLICATION    1
# define MESSAGE_SOURCE_PAGER          2
#endif //Q_WS_X11

namespace Core
{
	namespace AdiumChat
	{
		AbstractChatWidget::AbstractChatWidget(const QString &key, bool removeSessionOnClose)
			: m_key(key), m_removeSessionOnClose(removeSessionOnClose),
			m_htmlMessage(false), m_chatstate(ChatStateActive), m_entersNumber(0),m_current_index(-1)
		{
			setAttribute(Qt::WA_DeleteOnClose);
		}
		
		AbstractChatWidget::~AbstractChatWidget()
		{
			ConfigGroup group = Config("appearance").group("adiumChat/behavior/widget/keys").group(m_key);
			group.setValue("geometry", saveGeometry());
			foreach (QSplitter *splitter, findChildren<QSplitter*>()) {
				group.setValue(splitter->objectName(), splitter->saveState());
			}
			group.sync();
			if (m_removeSessionOnClose) {
				foreach (ChatSession *session, m_sessions) {
					session->disconnect(this);
					session->deleteLater();
				}
			}
		}
		
		bool AbstractChatWidget::contains(ChatSessionImpl *session)
		{
			return m_sessions.contains(session);
		}
		
		void AbstractChatWidget::onSendButtonClicked()
		{
			QPlainTextEdit *edit = getInputField();
			QTabBar *tabBar = getTabBar();
			if (edit->toPlainText().trimmed().isEmpty() || tabBar->currentIndex() < 0)
				return;
			ChatSessionImpl *session = currentSession();
			ChatUnit *unit = session->getUnit();
			foreach (QAction *a, session->menu()->actions()) {
				if (a->isChecked()) {
					unit = a->data().value<ChatUnit *>();
					break;
				}
			}

			Message message(edit->toPlainText());
			if (m_htmlMessage)
				message.setProperty("html", Qt::escape(message.text()));
			message.setIncoming(false);
			message.setChatUnit(unit);
			message.setTime(QDateTime::currentDateTime());
			session->appendMessage(message);
			unit->sendMessage(message);
			edit->clear();

			m_chatstateTimer.stop();
			m_chatstate = ChatStateActive;
		}
		
		void AbstractChatWidget::raise()
		{
#ifdef Q_WS_X11
			if (m_chatFlags & SwitchDesktopOnRaise) {
				static Atom NET_ACTIVE_WINDOW = 0;
				XClientMessageEvent xev;

				if(NET_ACTIVE_WINDOW == 0)
				{
					Display *dpy      = QX11Info::display();
					NET_ACTIVE_WINDOW = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
				}

				xev.type         = ClientMessage;
				xev.window       = winId();
				xev.message_type = NET_ACTIVE_WINDOW;
				xev.format       = 32;
				xev.data.l[0]    = MESSAGE_SOURCE_PAGER;
				xev.data.l[1]    = QX11Info::appUserTime();
				xev.data.l[2]    = xev.data.l[3] = xev.data.l[4] = 0;

				XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask | SubstructureRedirectMask, (XEvent*)&xev);
			}
#endif//Q_WS_X11
			QWidget::raise();
		}
		
		void AbstractChatWidget::onLoad()
		{
			ConfigGroup group = Config("appearance").group("chat/behavior/widget");
			ConfigGroup keysGroup = group.group("keys");
			if (keysGroup.hasChildGroup(m_key)) {
				ConfigGroup keyGroup = keysGroup.group(m_key);
				QByteArray geom = keyGroup.value("geometry", QByteArray());
				restoreGeometry(geom);
				foreach (QSplitter *splitter, findChildren<QSplitter*>()) {
					geom = keyGroup.value(splitter->objectName(), QByteArray());
					splitter->restoreState(geom);
				}
			} else {
				centerizeWidget(this);
			}
			m_htmlMessage = group.value("htmlMessage", false);
			// We need api for loading QFlags ;)
			m_chatFlags = static_cast<ChatFlag>(group.value<int>("widgetFlags", SendTypingNotification
																  | ChatStateIconsOnTabs
																  | ShowUnreadMessages
																  | SwitchDesktopOnRaise
																  | AeroThemeIntegration));
			m_sendKey = group.value("sendKey", SendCtrlEnter);

			if (m_chatFlags & SendTypingNotification) {
				connect(getInputField(),SIGNAL(textChanged()),SLOT(onTextChanged()));
				m_chatstate = ChatStateActive;
				m_chatstateTimer.setInterval(5000);
				m_chatstateTimer.setSingleShot(true);
				connect(&m_chatstateTimer,SIGNAL(timeout()),SLOT(onChatStateTimeout()));
			}

			//init shortcuts
			Shortcut *key = new Shortcut ("chatCloseSession",getTabBar());
			connect(key,SIGNAL(activated()),SLOT(closeCurrentTab()));
			key = new Shortcut ("chatNext",getTabBar());
			connect(key,SIGNAL(activated()),SLOT(showNextSession()));
			key = new Shortcut ("chatPrevious",getTabBar());
			connect(key,SIGNAL(activated()),SLOT(showPreviousSession()));

			connect(getContactsView(), SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClicked(QModelIndex)));

		}
		
		bool AbstractChatWidget::eventFilter(QObject *obj, QEvent *event)
		{
			const QMetaObject *meta = obj->metaObject();
			if (meta == &QPlainTextEdit::staticMetaObject) {
				if (event->type() == QEvent::KeyPress) {
					QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
					if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
						QPlainTextEdit *edit = getInputField();
						if (keyEvent->modifiers() == Qt::ControlModifier)  {
							if (m_sendKey == SendCtrlEnter) {
								onSendButtonClicked();
								return true;
							} else if (m_sendKey == SendEnter || m_sendKey == SendDoubleEnter) {
								edit->insertPlainText(QLatin1String("\n"));
							}
						} else if (keyEvent->modifiers() == Qt::NoModifier
								   || keyEvent->modifiers() == Qt::KeypadModifier) {
							if (m_sendKey == SendEnter) {
								onSendButtonClicked();
								return true;
							} else if (m_sendKey == SendDoubleEnter) {
								m_entersNumber++;
								if (m_entersNumber > 1) {
									m_enterPosition.deleteChar();
									m_entersNumber = 0;
									onSendButtonClicked();
									return true;
								} else {
									m_enterPosition = edit->textCursor();
								}
							}
						}
					} else {
						m_entersNumber = 0;
					}
				}
			} else if (meta == &QListView::staticMetaObject) {
				if (event->type() == QEvent::ContextMenu) {
					QContextMenuEvent *menuEvent = static_cast<QContextMenuEvent*>(event);
					QModelIndex index = getContactsView()->indexAt(menuEvent->pos());
					Buddy *buddy = index.data(Qt::UserRole).value<Buddy*>();
					if (buddy)
						buddy->showMenu(menuEvent->globalPos());
				}
			} else if (meta == &ChatSessionImpl::staticMetaObject) {
				if (event->type() == ChatStateEvent::eventType()) {
					ChatStateEvent *chatEvent = static_cast<ChatStateEvent *>(event);
					chatStateChanged(chatEvent->chatState(), qobject_cast<ChatSessionImpl *>(obj));
				}
			} else if (meta == &QTabBar::staticMetaObject) {
				if (event->type() == QEvent::ToolTip) {
					if (QHelpEvent *help = static_cast<QHelpEvent*>(event)) {
						QTabBar *bar = getTabBar();
						int index = bar->tabAt(help->pos());
						if (index != -1) {
							ChatUnit *unit = m_sessions.at(index)->getUnit();
							ToolTip::instance()->showText(help->globalPos(), unit, bar);
							return true;
						}
					}
				}
			} else if (meta == &QWebView::staticMetaObject) {
				if (event->type() == QEvent::KeyPress) {
					QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
					if (keyEvent->matches(QKeySequence::Copy)) {
						if (QWebView *view = static_cast<QWebView*>(obj))
							view->triggerPageAction(QWebPage::Copy);
						return true;
					}
				}
			}
			return QObject::eventFilter(obj, event);
		}

		void AbstractChatWidget::onChatStateTimeout()
		{
			m_chatstate = getInputField()->document()->isEmpty() ? ChatStateActive : ChatStatePaused;
			m_sessions.at(getTabBar()->currentIndex())->setChatState(m_chatstate);
		}

		void AbstractChatWidget::onTextChanged()
		{
			m_chatstateTimer.stop();
			m_chatstateTimer.start();
			if ((m_chatstate != ChatStateComposing) && (!getInputField()->toPlainText().isEmpty())) {
				m_chatstate = ChatStateComposing;
				m_sessions.at(getTabBar()->currentIndex())->setChatState(m_chatstate);
			}
		}


		void AbstractChatWidget::showNextSession()
		{
			m_current_index++;
			if (m_current_index >= m_sessions.count())
				m_current_index = 0;
			activate(m_sessions.at(m_current_index));
		}

		void AbstractChatWidget::showPreviousSession()
		{
			m_current_index--;
			if (m_current_index < 0 )
				m_current_index = m_sessions.count() - 1;
			activate(m_sessions.at(m_current_index));
		}


		void AbstractChatWidget::closeCurrentTab()
		{
			if (getTabBar()->count() > 1)
				removeSession(m_sessions.at(getTabBar()->currentIndex()));
			else
				close();
		}

		void AbstractChatWidget::onBuddiesChanged()
		{
			ChatSessionImpl *s = qobject_cast<ChatSessionImpl *>(sender());

			if (!s || (m_sessions.indexOf(s) != m_current_index))
				return;

			if (qobject_cast<Conference*>(s->getUnit()))
				getContactsView()->setVisible(true);
			else
				getContactsView()->setVisible(s->getModel()->rowCount(QModelIndex()) > 0);

		}

		void AbstractChatWidget::onDoubleClicked(const QModelIndex &index)
		{
			Buddy *buddy = index.data(Qt::UserRole).value<Buddy*>();
			if (buddy)
				ChatLayer::get(buddy, true)->activate();
		}

	}
}
