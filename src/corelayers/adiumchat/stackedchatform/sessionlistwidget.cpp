#include "sessionlistwidget.h"
#include <chatlayer/chatlayerimpl.h>
#include <chatlayer/chatsessionimpl.h>
#include <QModelIndex>
#include <QHelpEvent>
#include <qutim/tooltip.h>
#include <qutim/icon.h>
#include <QDropEvent>
#include <qutim/mimeobjectdata.h>
#include <avatarfilter.h>

namespace Core {
namespace AdiumChat {

class SessionListWidgetPrivate
{
public:
	ChatSessionList sessions;
};

SessionListWidget::SessionListWidget(QWidget *parent) :
	QListWidget(parent),
	d_ptr(new SessionListWidgetPrivate)
{
	connect(this,SIGNAL(itemActivated(QListWidgetItem*)),SLOT(onActivated(QListWidgetItem*)));
}

void SessionListWidget::addSession(ChatSessionImpl *session)
{
	QListWidgetItem *item = new QListWidgetItem(session->unit()->title(),this);
	QIcon icon = ChatLayerImpl::iconForState(ChatStateInActive,session->getUnit());
	if(Buddy *b = qobject_cast<Buddy*>(session->unit()))
		icon = AvatarFilter::icon(b->avatar(),icon);
	item->setIcon(icon);
	d_func()->sessions.append(session);

#ifndef QUTIM_MOBILE_UI
	setIconSize(QSize(32,32));
#endif

	connect(session->getUnit(),SIGNAL(titleChanged(QString,QString)),
			this,SLOT(onTitleChanged(QString)));
	connect(session,SIGNAL(destroyed(QObject*)),SLOT(onRemoveSession(QObject*)));
	connect(session,SIGNAL(unreadChanged(qutim_sdk_0_3::MessageList)),
			this,SLOT(onUnreadChanged(qutim_sdk_0_3::MessageList)));
	connect(session->getUnit(),
			SIGNAL(chatStateChanged(qutim_sdk_0_3::ChatState,qutim_sdk_0_3::ChatState)),
			this,
			SLOT(onChatStateChanged(qutim_sdk_0_3::ChatState,qutim_sdk_0_3::ChatState)));
}

void SessionListWidget::removeSession(ChatSessionImpl *session)
{
	removeItem(d_func()->sessions.indexOf(session));
}

ChatSessionImpl *SessionListWidget::session(int index) const
{
	Q_D(const SessionListWidget);
	if((index == -1) || (index >= d->sessions.count()))
		return 0;
	return d->sessions.at(index);
}

ChatSessionImpl *SessionListWidget::currentSession() const
{
	Q_D(const SessionListWidget);
	int index = currentIndex().row();
	if(index != -1 && index < d->sessions.count())
		return d->sessions.at(currentIndex().row());
	return 0;
}

void SessionListWidget::setCurrentSession(ChatSessionImpl *session)
{
	setCurrentItem(item(d_func()->sessions.indexOf(session)));
}

bool SessionListWidget::contains(ChatSessionImpl *session) const
{
	return d_func()->sessions.contains(session);
}

int SessionListWidget::indexOf(ChatSessionImpl *session) const
{
	return d_func()->sessions.indexOf(session);
}

void SessionListWidget::onRemoveSession(QObject *obj)
{
	Q_D(SessionListWidget);
	ChatSessionImpl *s = reinterpret_cast<ChatSessionImpl*>(obj);
	int index = d->sessions.indexOf(s);
	d->sessions.removeAll(s);
	delete item(index);
}

void SessionListWidget::removeItem(int index)
{
	ChatSessionImpl *s = d_func()->sessions.at(index);
	s->disconnect(this);
	s->removeEventFilter(this);
	onRemoveSession(s);
	emit remove(s);
}

void SessionListWidget::onTitleChanged(const QString &title)
{
	ChatUnit *u = qobject_cast<ChatUnit*>(sender());
	ChatSessionImpl *s = static_cast<ChatSessionImpl*>(ChatLayer::get(u,false));
	item(indexOf(s))->setText(title);
}

bool SessionListWidget::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip) {
		if (QHelpEvent *help = static_cast<QHelpEvent*>(event)) {
			int index = indexAt(help->pos()).row();
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
	} else if (event->type() == QEvent::ContextMenu) {
		QContextMenuEvent *ev = static_cast<QContextMenuEvent*>(event);
		session(row(itemAt(ev->pos())))->unit()->showMenu(ev->globalPos());
	}
	return QListWidget::event(event);
}

void SessionListWidget::chatStateChanged(ChatState state, ChatSessionImpl *session)
{
	if(session->unread().count())
		return;
	QIcon icon = ChatLayerImpl::iconForState(state,session->getUnit());
	if(Buddy *b = qobject_cast<Buddy*>(session->unit()))
		icon = AvatarFilter::icon(b->avatar(),icon);
	item(indexOf(session))->setIcon(icon);
}

void SessionListWidget::onUnreadChanged(const qutim_sdk_0_3::MessageList &unread)
{
	ChatSessionImpl *session = static_cast<ChatSessionImpl*>(sender());
	int index = indexOf(session);
	QIcon icon;
	QString title = session->getUnit()->title();
	if (unread.isEmpty()) {
		ChatState state = static_cast<ChatState>(session->property("currentChatState").toInt());//FIXME remove in future
		icon =  ChatLayerImpl::iconForState(state,session->getUnit());
		if(Buddy *b = qobject_cast<Buddy*>(session->unit()))
			icon = AvatarFilter::icon(b->avatar(),icon);

	} else {
		icon = Icon("mail-unread-new");
		title.insert(0,QChar('*'));
	}
	QListWidgetItem *i = item(index);
	i->setIcon(icon);
	i->setText(title);
}

void SessionListWidget::onChatStateChanged(qutim_sdk_0_3::ChatState now, qutim_sdk_0_3::ChatState)
{
	ChatUnit *unit = qobject_cast<ChatUnit*>(sender());
	Q_ASSERT(unit);
	ChatSessionImpl *s = static_cast<ChatSessionImpl*>(ChatLayerImpl::get(unit,false));
	if(s)
		chatStateChanged(now,s);
}

SessionListWidget::~SessionListWidget()
{

}

void SessionListWidget::onActivated(QListWidgetItem *i)
{
	if(ChatSessionImpl *s = session(row(i)))
		s->setActive(true);
}

void SessionListWidget::closeCurrentSession()
{
	if(currentItem()) {
		removeItem(currentIndex().row());
	}
}


} // namespace AdiumChat
} // namespace Core
