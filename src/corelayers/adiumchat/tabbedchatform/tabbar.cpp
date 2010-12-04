#include "tabbar.h"
#include <QStyle>
#include <QAction>
#include <chatlayer/chatsessionimpl.h>
#include <chatlayer/chatlayerimpl.h>
#include <qutim/tooltip.h>
#include <qutim/icon.h>
#include <QDropEvent>
#include <qutim/mimeobjectdata.h>
#include <qutim/shortcut.h>

namespace Core
{
namespace AdiumChat
{

struct TabBarPrivate
{
	bool closableActiveTab;
	ChatSessionList sessions;
	QMenu *sessionList;
};

TabBar::TabBar(QWidget *parent) : QTabBar(parent), p(new TabBarPrivate())
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	setAcceptDrops(true);
	p->closableActiveTab = false;
	setMouseTracking(true);
	p->sessionList = new QMenu(this);
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	setMovable(true);
#ifdef Q_WS_MAC
	setClosableActiveTab(true);
#else
	setTabsClosable(true);
#endif

	//init shortcuts
	Shortcut *key = new Shortcut ("chatCloseSession",this);
	connect(key,SIGNAL(activated()),SLOT(closeCurrentTab()));
	key = new Shortcut ("chatNext",this);
	connect(key,SIGNAL(activated()),SLOT(showNextTab()));
	key = new Shortcut ("chatPrevious",this);
	connect(key,SIGNAL(activated()),SLOT(showPreviousTab()));

	connect(this,SIGNAL(currentChanged(int)),SLOT(onCurrentChanged(int)));
	connect(this,SIGNAL(tabCloseRequested(int)),SLOT(onCloseRequested(int)));
	connect(this,SIGNAL(tabMoved(int,int)),SLOT(onTabMoved(int,int)));
	connect(this,SIGNAL(customContextMenuRequested(QPoint)),SLOT(onContextMenu(QPoint)));
	connect(p->sessionList,SIGNAL(triggered(QAction*)),SLOT(onSessionListActionTriggered(QAction*)));
}

TabBar::~TabBar()
{
	blockSignals(true);
	foreach(ChatSessionImpl *s,p->sessions)
		removeSession(s);
}

void TabBar::setClosableActiveTab(bool state)
{
	p->closableActiveTab = state;
	setTabsClosable(tabsClosable() | state);
} 

bool TabBar::closableActiveTab()
{
	return p->closableActiveTab;
}

void TabBar::setTabsClosable(bool closable)
{
	p->closableActiveTab = closable & p->closableActiveTab;
	QTabBar::setTabsClosable(closable);
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
	int hoveredTab = -1;
	if (p->closableActiveTab)
		for (int tab = 0; tab < count(); tab++) {
			QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this);
			if (QWidget *button = tabButton(tab, closeSide))
				button->setVisible(false);
			if (tabRect(tab).contains(event->pos()))
				hoveredTab = tab;
		}
	if (hoveredTab != -1) {
		QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this);
		if (QWidget *button = tabButton(hoveredTab, closeSide))
			button->setVisible(true);
	}
	return QTabBar::mouseMoveEvent(event);
}

void TabBar::leaveEvent(QEvent *event)
{
	if (p->closableActiveTab)
		for (int tab = 0; tab < count(); tab++) {
			QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this);
			if (QWidget *button = tabButton(tab, closeSide))
				button->setVisible(false);
		}
	return QTabBar::leaveEvent(event);
}

void TabBar::addSession(ChatSessionImpl *session)
{
	p->sessions.append(session);
	ChatUnit *u = session->getUnit();
	QIcon icon = ChatLayerImpl::iconForState(u->chatState(),u);
	p->sessionList->addAction(icon,session->getUnit()->title());
	addTab(icon,session->getUnit()->title());

#if defined(Q_WS_MAC)
	if (closableActiveTab()) {
		QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this);
		if (QWidget *button = tabButton(count()-1, closeSide))
			button->setVisible(false);
	}
#endif

	connect(session->getUnit(),SIGNAL(titleChanged(QString,QString)),
			this,SLOT(onTitleChanged(QString)));
	connect(session->getUnit(),
			SIGNAL(chatStateChanged(qutim_sdk_0_3::ChatState,qutim_sdk_0_3::ChatState)),
			this,
			SLOT(onChatStateChanged(qutim_sdk_0_3::ChatState,qutim_sdk_0_3::ChatState)));
	connect(session,SIGNAL(destroyed(QObject*)),SLOT(onRemoveSession(QObject*)));
	connect(session,SIGNAL(unreadChanged(qutim_sdk_0_3::MessageList)),
			this,SLOT(onUnreadChanged(qutim_sdk_0_3::MessageList)));
}

void TabBar::removeSession(ChatSessionImpl *session)
{
	removeTab(p->sessions.indexOf(session));
}

ChatSessionImpl *TabBar::session(int index) const
{
	if((index == -1) || (index >= p->sessions.count()))
		return 0;
	return p->sessions.at(index);
}

ChatSessionImpl *TabBar::currentSession() const
{
	int index = currentIndex();
	if(index != -1 && index < p->sessions.count())
		return p->sessions.at(currentIndex());
	return 0;
}

void TabBar::setCurrentSession(ChatSessionImpl *session)
{
	setCurrentIndex(p->sessions.indexOf(session));
}

bool TabBar::contains(ChatSessionImpl *session) const
{
	return p->sessions.contains(session);
}

int TabBar::indexOf(ChatSessionImpl *session) const
{
	return p->sessions.indexOf(session);
}

void TabBar::onCurrentChanged(int index)
{
	if(ChatSessionImpl *s = session(index))
		s->setActive(true);
}

void TabBar::onCloseRequested(int index)
{
	removeTab(index);
}

void TabBar::removeTab(int index)
{
	ChatSessionImpl *s = p->sessions.at(index);
	s->disconnect(this);
	s->removeEventFilter(this);
	s->unit()->disconnect(this);
	onRemoveSession(s);
	emit remove(s);
}

void TabBar::onRemoveSession(QObject *obj)
{
	ChatSessionImpl *s = reinterpret_cast<ChatSessionImpl*>(obj);
	int index = p->sessions.indexOf(s);
	p->sessions.removeAll(s);
	p->sessionList->removeAction(p->sessionList->actions().at(index));
	QTabBar::removeTab(index);
}

void TabBar::onTabMoved(int from, int to)
{
	p->sessions.move(from,to);
	//small hack
	QList <QAction *> actions = p->sessionList->actions();
	actions.move(from,to);
	foreach (QAction *a,p->sessionList->actions())
		p->sessionList->removeAction(a);
	p->sessionList->addActions(actions);
}

void TabBar::onTitleChanged(const QString &title)
{
	ChatUnit *u = qobject_cast<ChatUnit*>(sender());
	ChatSessionImpl *s = static_cast<ChatSessionImpl*>(ChatLayer::get(u,false));
	setTabText(indexOf(s),title);
}

void TabBar::onChatStateChanged(qutim_sdk_0_3::ChatState now, qutim_sdk_0_3::ChatState)
{
	ChatUnit *unit = qobject_cast<ChatUnit*>(sender());
	Q_ASSERT(unit);
	ChatSessionImpl *s = static_cast<ChatSessionImpl*>(ChatLayerImpl::get(unit,false));
	if(s)
		chatStateChanged(now,s);
}

bool TabBar::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip) {
		if (QHelpEvent *help = static_cast<QHelpEvent*>(event)) {
			int index = tabAt(help->pos());
			if (index != -1) {
				ChatUnit *unit = session(index)->getUnit();
				ToolTip::instance()->showText(help->globalPos(), unit, this);
				return true;
			}
		}
	} else if (event->type() == QEvent::DragEnter) {
		QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
		if (const MimeObjectData *data = qobject_cast<const MimeObjectData*>(dragEvent->mimeData())) {
			ChatUnit *u = qobject_cast<ChatUnit*>(data->object());
			if (u)
				dragEvent->acceptProposedAction();
		}
		return true;
	} else if (event->type() == QEvent::Drop) {
		QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
		if (const MimeObjectData *mimeData
				= qobject_cast<const MimeObjectData*>(dropEvent->mimeData())) {
			if (ChatUnit *u = qobject_cast<ChatUnit*>(mimeData->object())) {
				ChatLayerImpl::get(u,true)->activate();
				dropEvent->setDropAction(Qt::CopyAction);
				dropEvent->accept();
				return true;
			}
		}
	}
	return QTabBar::event(event);
}

void TabBar::chatStateChanged(ChatState state, ChatSessionImpl *session)
{
	if(session->unread().count())
		return;
	QIcon icon = ChatLayerImpl::iconForState(state,session->getUnit());
	setTabIcon(indexOf(session),icon);
	p->sessionList->actions().at(indexOf(session))->setIcon(icon);
}

void TabBar::onUnreadChanged(const qutim_sdk_0_3::MessageList &unread)
{
	ChatSessionImpl *session = static_cast<ChatSessionImpl*>(sender());
	int index = indexOf(session);
	QIcon icon;
	QString title = session->getUnit()->title();
	if (unread.isEmpty()) {
		ChatState state = static_cast<ChatState>(session->property("currentChatState").toInt());//FIXME remove in future
		icon =  ChatLayerImpl::iconForState(state,session->getUnit());
	} else {
		icon = Icon("mail-unread-new");
		title.insert(0,QChar('*'));
	}
	p->sessionList->actions().at(index)->setIcon(icon);

	setTabIcon(index, icon);
	setTabText(index, title);
}

void TabBar::onContextMenu(const QPoint &pos)
{
	if(currentSession())
		currentSession()->getUnit()->showMenu(mapToGlobal(pos));
}

QMenu *TabBar::menu() const
{
	return p->sessionList;
}

void TabBar::onSessionListActionTriggered(QAction *act)
{
	int index = p->sessionList->actions().indexOf(act);
	setCurrentIndex(index);
}

void TabBar::closeCurrentTab()
{
	onCloseRequested(currentIndex());
}

void TabBar::showNextTab()
{
	int index = currentIndex()+1;
	if (index >= count())
		index = 0;
	setCurrentIndex(index);
}

void TabBar::showPreviousTab()
{
	int index = currentIndex()-1;
	if (index >= count())
		index = count() - 1;
	setCurrentIndex(index);
}

}
}
